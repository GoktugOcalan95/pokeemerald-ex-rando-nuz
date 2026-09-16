#include "global.h"
#include "event_data.h"
#include "script.h"
#include "teaching_randomizer.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/script_commands.h"

extern const u8 SlateportCity_PokemonFanClub_EventScript_SwaggerTutor[];
extern ScrCmdFunc gScriptCmdTable[];
extern ScrCmdFunc gScriptCmdTableEnd[];
void ChooseBoxMon(struct ScriptContext *ctx);

static u32 sTeachCount;
static bool32 sTeachSuccess;
static u16 sOfferedMove;

static bool8 SkipTutorPresentation(struct ScriptContext *ctx)
{
    if (ctx->scriptPtr[-1] == SCR_OP_MESSAGE)
        ctx->scriptPtr += 4;
    else if (ctx->scriptPtr[-1] == SCR_OP_YESNOBOX)
    {
        ctx->scriptPtr += 2;
        gSpecialVar_Result = TRUE;
    }
    return TRUE;
}

static bool8 RunTutorNative(struct ScriptContext *ctx)
{
    u32 pointer = ScriptReadWord(ctx) & ~0x02000000;
    if (pointer == (u32)ChooseBoxMon)
    {
        ctx->scriptPtr++;
        sTeachCount++;
        sOfferedMove = gSpecialVar_0x8005;
        gSpecialVar_Result = sTeachSuccess;
        return TRUE;
    }
    ctx->scriptPtr -= 4;
    return gScriptCmdTable[SCR_OP_CALLNATIVE](ctx);
}

static void VisitTutor(void)
{
    ScrCmdFunc commands[256];
    struct ScriptContext ctx;
    u32 count = gScriptCmdTableEnd - gScriptCmdTable;
    static const u8 presentation[] = {SCR_OP_MESSAGE, SCR_OP_WAITMESSAGE, SCR_OP_WAITBUTTONPRESS,
        SCR_OP_LOCK, SCR_OP_RELEASE, SCR_OP_FACEPLAYER, SCR_OP_CLOSEMESSAGE, SCR_OP_WAITSTATE, SCR_OP_YESNOBOX};
    memcpy(commands, gScriptCmdTable, count * sizeof(*commands));
    for (u32 i = 0; i < ARRAY_COUNT(presentation); i++)
        commands[presentation[i]] = SkipTutorPresentation;
    commands[SCR_OP_CALLNATIVE] = RunTutorNative;
    InitScriptContext(&ctx, commands, commands + count);
    SetupBytecodeScript(&ctx, SlateportCity_PokemonFanClub_EventScript_SwaggerTutor);
    u32 steps = 0;
    while (RunScriptCommand(&ctx))
        EXPECT(++steps < 500);
}

TEST("Reusable tutors preserve history but allow repeat teaching and never consume cancelled offers")
{
    bool32 reusable = FALSE, randomized = FALSE;
    PARAMETRIZE { reusable = FALSE; randomized = FALSE; }
    PARAMETRIZE { reusable = TRUE; randomized = FALSE; }
    PARAMETRIZE { reusable = FALSE; randomized = TRUE; }
    PARAMETRIZE { reusable = TRUE; randomized = TRUE; }
    FlagClear(FLAG_MOVE_TUTOR_TAUGHT_SWAGGER);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
    FlagClear(FLAG_RUN_RULE_TMS_TUTORS);
    if (reusable) FlagSet(FLAG_RUN_RULE_REUSABLE_TMS);
    if (randomized) FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    sTeachCount = 0;
    sTeachSuccess = FALSE;
    VisitTutor();
    EXPECT_EQ(sTeachCount, 1);
    EXPECT(!FlagGet(FLAG_MOVE_TUTOR_TAUGHT_SWAGGER));
    sTeachSuccess = TRUE;
    VisitTutor();
    EXPECT_EQ(sTeachCount, 2);
    EXPECT(FlagGet(FLAG_MOVE_TUTOR_TAUGHT_SWAGGER));
    EXPECT_EQ(sOfferedMove, GetRandomizedTutorMove(MOVE_SWAGGER));
    u16 move = sOfferedMove;
    VisitTutor();
    EXPECT_EQ(sTeachCount, reusable ? 3 : 2);
    EXPECT_EQ(sOfferedMove, move);
}

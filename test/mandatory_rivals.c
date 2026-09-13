#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "constants/event_objects.h"
#include "mandatory_rivals.h"
#include "overworld.h"
#include "save.h"
#include "script.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/maps.h"
#include "constants/map_event_ids.h"
#include "constants/vars.h"
#include "constants/script_commands.h"
#include "constants/trainers.h"

extern const u8 RustboroCity_EventScript_RearmRival[];
extern const u8 Route104_EventScript_RearmRival[];

TEST("Mandatory rivals gate onward exits with every early travel and story combination")
{
    const u16 maps[] = {MAP_OLDALE_TOWN, MAP_ROUTE104, MAP_ROUTE110, MAP_ROUTE119, MAP_ROUTE118, MAP_LILYCOVE_CITY};
    const u16 flags[] = {FLAG_RUN_RULE_EARLY_SURF, FLAG_RUN_RULE_EARLY_FLY, FLAG_RUN_RULE_REMOVE_STORY};
    for (u32 options = 0; options < 8; options++)
    {
        InitEventData();
        for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
            if (options & (1 << i))
                FlagSet(flags[i]);
        for (u32 i = 0; i < ARRAY_COUNT(maps); i++)
        {
            const struct MapLayout *layout = Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(maps[i]), MAP_NUM(maps[i]))->mapLayout;
            u32 rival = i < 4 ? i + 1 : i;
            s16 x = i == 5 ? 30 : i == 3 || i == 4 ? layout->width - 1 : 0;
            s16 y = i == 1 ? layout->height - 1 : 0;
            s16 nextX = x + (i >= 3 ? 1 : i == 0 ? -1 : 0);
            s16 nextY = y + (i == 1 ? 1 : i == 2 ? -1 : 0);
            EXPECT_EQ(GetMandatoryRivalBoundary(maps[i], x, y, nextX, nextY), rival);
            EXPECT_EQ(GetMandatoryRivalBoundary(maps[i], nextX, nextY, x, y), 0);
            EXPECT_EQ(GetMandatoryRivalBoundary(maps[i], x, y, x, y), 0);
        }
        FlagSet(FLAG_DEFEATED_RIVAL_ROUTE103);
        FlagSet(FLAG_DEFEATED_RIVAL_ROUTE_104);
        VarSet(VAR_ROUTE110_STATE, 1);
        VarSet(VAR_ROUTE119_STATE, 1);
        FlagSet(FLAG_MET_RIVAL_LILYCOVE);
        for (u32 i = 1; i <= 5; i++)
            EXPECT(HasWonMandatoryRival(i));
        // The whole city boundary includes the lower roads and the surfable coast.
        for (u32 y = 0; y < 40; y++)
        {
            EXPECT_EQ(GetMandatoryRivalBoundary(MAP_LILYCOVE_CITY, 30, y, 31, y), 0);
            FlagClear(FLAG_MET_RIVAL_LILYCOVE);
            EXPECT_EQ(GetMandatoryRivalBoundary(MAP_LILYCOVE_CITY, 30, y, 31, y), 5);
            FlagSet(FLAG_MET_RIVAL_LILYCOVE);
        }
    }
}

TEST("Mandatory rivals rearm either second encounter after a loss without repeating registration")
{
    InitEventData();
    gSaveBlock1Ptr->objectEventTemplates[0].localId = LOCALID_RUSTBORO_RIVAL;
    gSaveBlock1Ptr->objectEventTemplates[1].localId = LOCALID_ROUTE104_RIVAL;
    VarSet(VAR_RUSTBORO_CITY_STATE, 8);
    VarSet(VAR_ROUTE104_STATE, 2);
    FlagSet(FLAG_REGISTER_RIVAL_POKENAV);
    RunScriptImmediately(RustboroCity_EventScript_RearmRival);
    RunScriptImmediately(Route104_EventScript_RearmRival);
    EXPECT_EQ(VarGet(VAR_RUSTBORO_CITY_STATE), 7);
    EXPECT_EQ(VarGet(VAR_ROUTE104_STATE), 1);
    EXPECT(FlagGet(FLAG_REGISTER_RIVAL_POKENAV));
    EXPECT(!HasWonMandatoryRival(2));
    for (u32 i = 0; i < 2; i++)
    {
        u32 flag = i ? FLAG_DEFEATED_RIVAL_ROUTE_104 : FLAG_DEFEATED_RIVAL_RUSTBORO;
        FlagSet(flag);
        VarSet(VAR_RUSTBORO_CITY_STATE, 8);
        VarSet(VAR_ROUTE104_STATE, 2);
        RunScriptImmediately(RustboroCity_EventScript_RearmRival);
        RunScriptImmediately(Route104_EventScript_RearmRival);
        EXPECT_EQ(VarGet(VAR_RUSTBORO_CITY_STATE), 8);
        EXPECT_EQ(VarGet(VAR_ROUTE104_STATE), 2);
        EXPECT(HasWonMandatoryRival(2));
        FlagClear(flag);
    }
}

TEST("Mandatory rival victories and pending retries survive saving and loading")
{
    InitEventData();
    FlagSet(FLAG_DEFEATED_RIVAL_ROUTE103);
    FlagSet(FLAG_DEFEATED_RIVAL_RUSTBORO);
    VarSet(VAR_ROUTE110_STATE, 1);
    VarSet(VAR_ROUTE119_STATE, 0);
    FlagClear(FLAG_MET_RIVAL_LILYCOVE);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    InitEventData();
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    for (u32 rival = 1; rival <= 5; rival++)
        EXPECT_EQ(HasWonMandatoryRival(rival), rival <= 3);
}


extern ScrCmdFunc gScriptCmdTable[];
extern ScrCmdFunc gScriptCmdTableEnd[];
extern const u8 RustboroCity_EventScript_MayEncounter[];
extern const u8 RustboroCity_EventScript_BrendanEncounter[];
extern const u8 Route104_EventScript_MayEncounter[];
extern const u8 Route104_EventScript_BrendanEncounter[];
extern const u8 LilycoveCity_EventScript_May[];
extern const u8 LilycoveCity_EventScript_Brendan[];
static EWRAM_DATA u16 sRivalTrainer = 0;

static bool8 SkipRivalPresentation(struct ScriptContext *ctx)
{
    switch (ctx->scriptPtr[-1])
    {
    case SCR_OP_CALL_STD:
        EXPECT_EQ(ScriptReadByte(ctx), 4); // MSGBOX_DEFAULT
        break;
    case SCR_OP_APPLYMOVEMENT: ctx->scriptPtr += 6; break;
    case SCR_OP_PLAYBGM: ctx->scriptPtr += 3; break;
    case SCR_OP_COPYOBJECTXYTOPERM:
    case SCR_OP_WAITMOVEMENT:
    case SCR_OP_DELAY:
    case SCR_OP_PLAYFANFARE: ctx->scriptPtr += 2; break;
    default: break;
    }
    return FALSE;
}

static bool8 CaptureRivalBattle(struct ScriptContext *ctx)
{
    ctx->scriptPtr += 2;
    sRivalTrainer = ScriptReadHalfword(ctx);
    StopScript(ctx);
    return TRUE;
}

TEST("Mandatory rivals start the correct battle without a decline for both genders all starters and retries")
{
    const u8 *scripts[] = {RustboroCity_EventScript_MayEncounter, RustboroCity_EventScript_BrendanEncounter,
        Route104_EventScript_MayEncounter, Route104_EventScript_BrendanEncounter,
        LilycoveCity_EventScript_May, LilycoveCity_EventScript_Brendan};
    const u16 trainers[][3] = {
        {TRAINER_MAY_RUSTBORO_TREECKO, TRAINER_MAY_RUSTBORO_TORCHIC, TRAINER_MAY_RUSTBORO_MUDKIP},
        {TRAINER_BRENDAN_RUSTBORO_TREECKO, TRAINER_BRENDAN_RUSTBORO_TORCHIC, TRAINER_BRENDAN_RUSTBORO_MUDKIP},
        {TRAINER_MAY_LILYCOVE_TREECKO, TRAINER_MAY_LILYCOVE_TORCHIC, TRAINER_MAY_LILYCOVE_MUDKIP},
        {TRAINER_BRENDAN_LILYCOVE_TREECKO, TRAINER_BRENDAN_LILYCOVE_TORCHIC, TRAINER_BRENDAN_LILYCOVE_MUDKIP},
    };
    const u8 presentation[] = {SCR_OP_CALL_STD, SCR_OP_APPLYMOVEMENT, SCR_OP_WAITMOVEMENT,
        SCR_OP_PLAYBGM, SCR_OP_COPYOBJECTXYTOPERM, SCR_OP_DELAY, SCR_OP_PLAYFANFARE,
        SCR_OP_WAITFANFARE, SCR_OP_CLOSEMESSAGE};
    ScrCmdFunc commands[256];
    u32 count = gScriptCmdTableEnd - gScriptCmdTable;
    memcpy(commands, gScriptCmdTable, count * sizeof(*commands));
    for (u32 i = 0; i < ARRAY_COUNT(presentation); i++)
        commands[presentation[i]] = SkipRivalPresentation;
    commands[SCR_OP_TRAINERBATTLE] = CaptureRivalBattle;
    for (u32 entry = 0; entry < ARRAY_COUNT(scripts); entry++)
        for (u32 starter = 0; starter < 3; starter++)
            for (u32 retry = 0; retry < 2; retry++)
            {
                struct ScriptContext ctx;
                InitEventData();
                VarSet(VAR_STARTER_MON, starter);
                if (retry)
                {
                    FlagSet(FLAG_MET_RIVAL_RUSTBORO);
                    FlagSet(FLAG_REGISTER_RIVAL_POKENAV);
                }
                sRivalTrainer = TRAINER_NONE;
                InitScriptContext(&ctx, commands, commands + count);
                SetupBytecodeScript(&ctx, scripts[entry]);
                u32 steps = 0;
                while (RunScriptCommand(&ctx))
                    EXPECT(++steps < 100);
                EXPECT_EQ(sRivalTrainer, trainers[entry < 4 ? entry % 2 : entry - 2][starter]);
                EXPECT(!HasWonMandatoryRival(entry < 4 ? 2 : 5));
                if (entry < 2)
                {
                    EXPECT_EQ(VarGet(VAR_RUSTBORO_CITY_STATE), 8);
                    EXPECT_EQ(VarGet(VAR_ROUTE104_STATE), 2);
                }
            }
}

TEST("Mandatory rival collision blocks walking bikes and Surf before Lilycove interception")
{
    const u8 modes[] = {PLAYER_AVATAR_FLAG_ON_FOOT, PLAYER_AVATAR_FLAG_MACH_BIKE,
        PLAYER_AVATAR_FLAG_ACRO_BIKE, PLAYER_AVATAR_FLAG_SURFING};
    struct ObjectEvent player = {0};
    InitEventData();
    UnlockPlayerFieldControls();
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_LILYCOVE_CITY);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_LILYCOVE_CITY);
    player.isPlayer = TRUE;
    player.currentCoords.x = 30 + MAP_OFFSET;
    player.currentCoords.y = 16 + MAP_OFFSET;
    for (u32 i = 0; i < ARRAY_COUNT(modes); i++)
    {
        gPlayerAvatar.flags = modes[i];
        EXPECT_EQ(GetCollisionAtCoords(&player, 31 + MAP_OFFSET, 16 + MAP_OFFSET, DIR_EAST), COLLISION_IMPASSABLE);
    }
}

extern const u8 Route104_MrBrineysHouse_EventScript_SailToDewford[];
static EWRAM_DATA bool8 sStartedDeparture = FALSE;

static bool8 CaptureDeparture(struct ScriptContext *ctx)
{
    sStartedDeparture = TRUE;
    StopScript(ctx);
    return TRUE;
}

TEST("Mandatory rival victory at either location unlocks Briney departure in both story modes")
{
    ScrCmdFunc commands[256];
    u32 count = gScriptCmdTableEnd - gScriptCmdTable;
    memcpy(commands, gScriptCmdTable, count * sizeof(*commands));
    commands[SCR_OP_CALL_STD] = SkipRivalPresentation;
    commands[SCR_OP_RELEASEALL] = SkipRivalPresentation;
    commands[SCR_OP_CALL] = CaptureDeparture;
    for (u32 story = 0; story < 2; story++)
        for (u32 victory = 0; victory < 3; victory++)
        {
            struct ScriptContext ctx;
            InitEventData();
            if (story)
                FlagSet(FLAG_RUN_RULE_REMOVE_STORY);
            if (victory)
                FlagSet(victory == 1 ? FLAG_DEFEATED_RIVAL_RUSTBORO : FLAG_DEFEATED_RIVAL_ROUTE_104);
            sStartedDeparture = FALSE;
            InitScriptContext(&ctx, commands, commands + count);
            SetupBytecodeScript(&ctx, Route104_MrBrineysHouse_EventScript_SailToDewford);
            u32 steps = 0;
            while (RunScriptCommand(&ctx))
                EXPECT(++steps < 100);
            EXPECT_EQ(sStartedDeparture, victory != 0);
            EXPECT_EQ(VarGet(VAR_BOARD_BRINEY_BOAT_STATE), 0);
        }
}

#include "global.h"
#include "event_data.h"
#include "item.h"
#include "move.h"
#include "move_randomizer.h"
#include "player_teachable_moves.h"
#include "run_randomizer.h"
#include "script.h"
#include "string_util.h"
#include "teaching_randomizer.h"
#include "text.h"
#include "constants/characters.h"
#include "constants/flags.h"

#define TEACHING_DOMAIN 0x300
#include "data/tutor_moves.h"

#define TUTOR_MOVE_COUNT (ARRAY_COUNT(gTutorMoves) - 1)

static EWRAM_DATA u16 sTeachingMoves[NUM_TECHNICAL_MACHINES + TUTOR_MOVE_COUNT] = {0};
static EWRAM_DATA u16 sOriginalMoves[MOVES_COUNT] = {0};
static EWRAM_DATA u16 sAssignedMoves[MOVES_COUNT] = {0};
static EWRAM_DATA u32 sTeachingSeed = 0;
static EWRAM_DATA bool8 sTeachingReady = FALSE;
static EWRAM_DATA u8 sMoveDescription[256] = {0};

static void InitTeachingMoves(void)
{
    u32 seed = RunRandomizerHash(TEACHING_DOMAIN, 0, 0);
    if (sTeachingReady && seed == sTeachingSeed)
        return;
    bool8 used[MOVES_COUNT] = {0};
    for (u32 move = 0; move < MOVES_COUNT; move++)
    {
        sOriginalMoves[move] = move;
        sAssignedMoves[move] = move;
    }
    for (u32 index = NUM_TECHNICAL_MACHINES + 1; index <= NUM_ALL_MACHINES; index++)
        used[gTMHMItemMoveIds[index].moveId] = TRUE;
    for (u32 index = 0; index < ARRAY_COUNT(sTeachingMoves); index++)
    {
        u32 move = ChooseRandomizerMove(TEACHING_DOMAIN, index, 0, used);
        u32 original = index < NUM_TECHNICAL_MACHINES ? gTMHMItemMoveIds[index + 1].moveId : gTutorMoves[index - NUM_TECHNICAL_MACHINES];
        sOriginalMoves[move] = original;
        sAssignedMoves[original] = move;
        sTeachingMoves[index] = move;
        used[move] = TRUE;
    }
    sTeachingSeed = seed;
    sTeachingReady = TRUE;
}

u16 GetRandomizedMachineMove(u32 index)
{
    if (index > NUM_ALL_MACHINES)
        return MOVE_NONE;
    if (!FlagGet(FLAG_RUN_RULE_TMS_TUTORS) || index == 0 || index > NUM_TECHNICAL_MACHINES)
        return gTMHMItemMoveIds[index].moveId;
    InitTeachingMoves();
    return sTeachingMoves[index - 1];
}

u16 GetOriginalTeachingMove(u16 assigned)
{
    if (!FlagGet(FLAG_RUN_RULE_TMS_TUTORS) || assigned >= MOVES_COUNT)
        return assigned;
    InitTeachingMoves();
    return sOriginalMoves[assigned];
}

u16 GetRandomizedTeachingMove(u16 original)
{
    if (!FlagGet(FLAG_RUN_RULE_TMS_TUTORS) || original >= MOVES_COUNT)
        return original;
    InitTeachingMoves();
    return sAssignedMoves[original];
}

u16 GetTutorMove(u32 index)
{
    if (index >= TUTOR_MOVE_COUNT)
        return MOVE_UNAVAILABLE;
    if (!FlagGet(FLAG_RUN_RULE_TMS_TUTORS))
        return gTutorMoves[index];
    InitTeachingMoves();
    return sTeachingMoves[NUM_TECHNICAL_MACHINES + index];
}

u16 GetRandomizedTutorMove(u16 original)
{
    for (u32 i = 0; i < TUTOR_MOVE_COUNT; i++)
        if (gTutorMoves[i] == original)
            return GetTutorMove(i);
    return original;
}

u16 GetFrontierTutorMove(u32 tutor, u32 index)
{
    static const u16 moves[2][10] =
    {
        {MOVE_SOFT_BOILED, MOVE_SEISMIC_TOSS, MOVE_DREAM_EATER, MOVE_MEGA_PUNCH, MOVE_MEGA_KICK,
         MOVE_BODY_SLAM, MOVE_ROCK_SLIDE, MOVE_COUNTER, MOVE_THUNDER_WAVE, MOVE_SWORDS_DANCE},
        {MOVE_DEFENSE_CURL, MOVE_SNORE, MOVE_MUD_SLAP, MOVE_SWIFT, MOVE_ICY_WIND,
         MOVE_ENDURE, MOVE_PSYCH_UP, MOVE_ICE_PUNCH, MOVE_THUNDER_PUNCH, MOVE_FIRE_PUNCH},
    };
    if (tutor >= ARRAY_COUNT(moves) || index >= ARRAY_COUNT(moves[0]))
        return MOVE_NONE;
    return GetRandomizedTutorMove(moves[tutor][index]);
}

bool8 ScriptRandomizeTutorMove(struct ScriptContext *ctx)
{
    gSpecialVar_0x8005 = GetRandomizedTutorMove(gSpecialVar_0x8005);
    StringCopy(gStringVar1, GetMoveName(gSpecialVar_0x8005));
    return FALSE;
}

const u8 *GetRandomizedMoveDescription(u16 move, u32 width)
{
    const u8 *source = GetMoveDescription(move);
    u8 *out = sMoveDescription;
    u32 lineWidth = 0;
    u32 spaceWidth = GetStringWidth(FONT_SMALL, COMPOUND_STRING(" "), 0);
    *out++ = EXT_CTRL_CODE_BEGIN;
    *out++ = EXT_CTRL_CODE_FONT;
    *out++ = FONT_SMALL;
    while (*source != EOS)
    {
        u8 word[128];
        u32 length = 0;
        while (*source == CHAR_SPACE || *source == CHAR_NEWLINE)
            source++;
        while (*source != EOS && *source != CHAR_SPACE && *source != CHAR_NEWLINE && length < sizeof(word) - 1)
            word[length++] = *source++;
        if (!length)
            break;
        word[length] = EOS;
        u32 wordWidth = GetStringWidth(FONT_SMALL, word, 0);
        if (lineWidth)
        {
            if (lineWidth + spaceWidth + wordWidth > width)
            {
                *out++ = CHAR_NEWLINE;
                lineWidth = 0;
            }
            else
            {
                *out++ = CHAR_SPACE;
                lineWidth += spaceWidth;
            }
        }
        for (u32 i = 0; i < length; i++)
        {
            u8 glyph[] = {word[i], EOS};
            u32 glyphWidth = GetStringWidth(FONT_SMALL, glyph, 0);
            if (lineWidth + glyphWidth > width)
            {
                *out++ = CHAR_NEWLINE;
                lineWidth = 0;
            }
            *out++ = word[i];
            lineWidth += glyphWidth;
        }
    }
    *out++ = EXT_CTRL_CODE_BEGIN;
    *out++ = EXT_CTRL_CODE_FONT;
    *out++ = FONT_NORMAL;
    *out = EOS;
    return sMoveDescription;
}

bool8 ScriptPrepareRandomizedTMText(struct ScriptContext *ctx)
{
    u32 move = GetItemTMHMMoveId(ScriptReadHalfword(ctx));
    StringCopy(gStringVar2, GetMoveName(move));
    StringCopy(gStringVar3, GetMoveDescription(move));
    return FALSE;
}

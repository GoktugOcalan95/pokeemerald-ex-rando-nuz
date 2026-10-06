#include "global.h"
#include "event_data.h"
#include "item.h"
#include "move.h"
#include "move_randomizer.h"
#include "player_teachable_moves.h"
#include "run_companion.h"
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
// The companion reads one tutorsSeen bit per tutor.
STATIC_ASSERT(TUTOR_MOVE_COUNT <= RUN_DISCOVERY_TUTOR_COUNT, TutorDiscoveryBitsTooFew);

static EWRAM_DATA u16 sTeachingMoves[NUM_TECHNICAL_MACHINES + TUTOR_MOVE_COUNT] = {0};
static EWRAM_DATA u32 sTeachingSeed = 0;
static EWRAM_DATA u8 sTeachingChance = 0;
static EWRAM_DATA u8 sTeachingTMCount = 0;
static EWRAM_DATA bool8 sTeachingReady = FALSE;
static EWRAM_DATA u8 sMoveDescription[256] = {0};

bool32 IsExpandedTMListEnabled(void)
{
    return FlagGet(FLAG_RUN_RULE_EXPANDED_TMS);
}

u32 GetActiveTMCount(void)
{
    return IsExpandedTMListEnabled() ? NUM_TECHNICAL_MACHINES : NUM_ORIGINAL_TECHNICAL_MACHINES;
}

static u32 CountBits(u32 bits)
{
    bits -= (bits >> 1) & 0x55555555;
    bits = (bits & 0x33333333) + ((bits >> 2) & 0x33333333);
    return (((bits + (bits >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static void InitTeachingMoves(void)
{
    u32 seed = RunRandomizerHash(TEACHING_DOMAIN, 0, 0);
    u32 tmCount = GetActiveTMCount();
    if (sTeachingReady && seed == sTeachingSeed && sTeachingChance == GetRandomizerGoodMoveChance() && sTeachingTMCount == tmCount)
        return;
    // Good attacks are a subset of allowed moves. Each pick takes the choice-th set bit in ascending move order.
    u32 available[MOVE_BITSET_WORDS] = {0}, good[MOVE_BITSET_WORDS] = {0};
    u32 count = 0, goodCount = 0;
    u32 chance = GetRandomizerGoodMoveChance();
    for (u32 move = 1; move < MOVES_COUNT; move++)
    {
        if (!IsRandomizerMoveAllowed(move))
            continue;
        available[move / 32] |= 1u << (move % 32);
        count++;
        if (IsRandomizerGoodAttack(move))
        {
            good[move / 32] |= 1u << (move % 32);
            goodCount++;
        }
    }
    // HMs stay fixed, so their moves never fill a TM or tutor slot.
    for (u32 index = NUM_TECHNICAL_MACHINES + 1; index <= NUM_ALL_MACHINES; index++)
    {
        u32 move = gTMHMItemMoveIds[index].moveId, mask = 1u << (move % 32);
        if (!(available[move / 32] & mask))
            continue;
        available[move / 32] &= ~mask;
        count--;
        if (good[move / 32] & mask)
        {
            good[move / 32] &= ~mask;
            goodCount--;
        }
    }
    for (u32 index = 0; index < tmCount + TUTOR_MOVE_COUNT; index++)
    {
        sTeachingMoves[index] = MOVE_NONE;
        if (count == 0)
            continue;
        bool32 preferGood = goodCount != 0 && RunRandomizerHash(TEACHING_DOMAIN ^ 0x80000000, index, 0) % 100 < chance;
        u32 choice = RunRandomizerHash(TEACHING_DOMAIN, index, 0) % (preferGood ? goodCount : count);
        for (u32 word = 0; word < MOVE_BITSET_WORDS; word++)
        {
            u32 bits = preferGood ? good[word] : available[word];
            u32 bitCount = CountBits(bits);
            if (choice >= bitCount)
            {
                choice -= bitCount;
                continue;
            }
            u32 bit = 0;
            while (!((bits >> bit) & 1) || choice-- != 0)
                bit++;
            u32 mask = 1u << bit;
            sTeachingMoves[index] = word * 32 + bit;
            available[word] &= ~mask;
            count--;
            if (good[word] & mask)
            {
                good[word] &= ~mask;
                goodCount--;
            }
            break;
        }
    }
    sTeachingTMCount = tmCount;
    sTeachingSeed = seed;
    sTeachingChance = GetRandomizerGoodMoveChance();
    sTeachingReady = TRUE;
}

u16 GetRandomizedMachineMove(u32 index)
{
    if (index > NUM_ALL_MACHINES || (index > GetActiveTMCount() && index <= NUM_TECHNICAL_MACHINES))
        return MOVE_NONE;
    if (!FlagGet(FLAG_RUN_RULE_TMS_TUTORS) || index == 0 || index > NUM_TECHNICAL_MACHINES)
        return gTMHMItemMoveIds[index].moveId;
    InitTeachingMoves();
    return sTeachingMoves[index - 1];
}

u16 GetTutorMove(u32 index)
{
    if (index >= TUTOR_MOVE_COUNT)
        return MOVE_UNAVAILABLE;
    if (!FlagGet(FLAG_RUN_RULE_TMS_TUTORS))
        return gTutorMoves[index];
    InitTeachingMoves();
    return sTeachingMoves[GetActiveTMCount() + index];
}

u16 GetRandomizedTutorMove(u16 original)
{
    for (u32 i = 0; i < TUTOR_MOVE_COUNT; i++)
        if (gTutorMoves[i] == original)
            return GetTutorMove(i);
    return original;
}

u16 GetOriginalFrontierTutorMove(u32 tutor, u32 index)
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
    return moves[tutor][index];
}

u16 GetFrontierTutorMove(u32 tutor, u32 index)
{
    return GetRandomizedTutorMove(GetOriginalFrontierTutorMove(tutor, index));
}

bool8 ScriptRandomizeTutorMove(struct ScriptContext *ctx)
{
    gSpecialVar_0x8005 = GetRandomizedTutorMove(gSpecialVar_0x8005);
    StringCopy(gStringVar1, GetMoveName(gSpecialVar_0x8005));
    return FALSE;
}

// Both description windows are 48 px tall, so only three 12 px FONT_SMALL lines fit.
#define MOVE_DESCRIPTION_MAX_LINES 3

// Line breaks switch back to FONT_SMALL, because the narrow small fonts advance only 8 px per line.
static u8 *BreakMoveDescriptionLine(u8 *out, u32 fontId)
{
    if (fontId != FONT_SMALL)
    {
        *out++ = EXT_CTRL_CODE_BEGIN;
        *out++ = EXT_CTRL_CODE_FONT;
        *out++ = FONT_SMALL;
    }
    *out++ = CHAR_NEWLINE;
    if (fontId != FONT_SMALL)
    {
        *out++ = EXT_CTRL_CODE_BEGIN;
        *out++ = EXT_CTRL_CODE_FONT;
        *out++ = fontId;
    }
    return out;
}

static u32 WrapMoveDescription(const u8 *source, u32 width, u32 fontId)
{
    u8 *out = sMoveDescription;
    // Leave room for the closing font switch and EOS.
    u8 *end = sMoveDescription + sizeof(sMoveDescription) - 4;
    u32 lineWidth = 0, lines = 1;
    u32 spaceWidth = GetStringWidth(fontId, COMPOUND_STRING(" "), 0);
    *out++ = EXT_CTRL_CODE_BEGIN;
    *out++ = EXT_CTRL_CODE_FONT;
    *out++ = fontId;
    // Each step writes a separator or a line break (up to 7 bytes), then a glyph.
    while (*source != EOS && out + 8 <= end)
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
        u32 wordWidth = GetStringWidth(fontId, word, 0);
        if (lineWidth)
        {
            if (lineWidth + spaceWidth + wordWidth > width)
            {
                out = BreakMoveDescriptionLine(out, fontId);
                lineWidth = 0;
                lines++;
            }
            else
            {
                *out++ = CHAR_SPACE;
                lineWidth += spaceWidth;
            }
        }
        for (u32 i = 0; i < length && out + 8 <= end; i++)
        {
            u8 glyph[] = {word[i], EOS};
            u32 glyphWidth = GetStringWidth(fontId, glyph, 0);
            if (lineWidth + glyphWidth > width)
            {
                out = BreakMoveDescriptionLine(out, fontId);
                lineWidth = 0;
                lines++;
            }
            *out++ = word[i];
            lineWidth += glyphWidth;
        }
    }
    *out++ = EXT_CTRL_CODE_BEGIN;
    *out++ = EXT_CTRL_CODE_FONT;
    *out++ = FONT_NORMAL;
    *out = EOS;
    return lines;
}

const u8 *GetRandomizedMoveDescription(u16 move, u32 width)
{
    static const u8 fonts[] = {FONT_SMALL, FONT_SMALL_NARROW, FONT_SMALL_NARROWER};
    for (u32 i = 0; i < ARRAY_COUNT(fonts); i++)
    {
        if (WrapMoveDescription(GetMoveDescription(move), width, fonts[i]) <= MOVE_DESCRIPTION_MAX_LINES)
            break;
    }
    return sMoveDescription;
}

bool8 ScriptPrepareRandomizedTMText(struct ScriptContext *ctx)
{
    u32 move = GetItemTMHMMoveId(ScriptReadHalfword(ctx));
    StringCopy(gStringVar2, GetMoveName(move));
    StringCopy(gStringVar3, GetMoveDescription(move));
    return FALSE;
}

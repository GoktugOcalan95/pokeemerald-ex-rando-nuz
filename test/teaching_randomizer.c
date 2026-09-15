#include "global.h"
#include "event_data.h"
#include "item.h"
#include "list_menu.h"
#include "move.h"
#include "move_randomizer.h"
#include "party_menu.h"
#include "player_teachable_moves.h"
#include "pokemon.h"
#include "random.h"
#include "save.h"
#include "string_util.h"
#include "teaching_randomizer.h"
#include "text.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "constants/characters.h"
#include "constants/flags.h"
#include "constants/field_specials.h"

u32 Test_BuildScrollableMultichoiceItems(u32 menu, u32 count, struct ListMenuItem *items);

TEST("Teaching randomizer uses one unique TM and tutor pool and leaves HMs fixed")
{
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    for (u32 seed = 0; seed < 16; seed++)
    {
        bool8 used[MOVES_COUNT] = {0};
        gSaveBlock2Ptr->playerTrainerId[0] = seed;
        for (u32 index = NUM_TECHNICAL_MACHINES + 1; index <= NUM_ALL_MACHINES; index++)
        {
            EXPECT_EQ(GetTMHMMoveId(index), gTMHMItemMoveIds[index].moveId);
            used[GetTMHMMoveId(index)] = TRUE;
        }
        for (u32 index = 1; index <= NUM_TECHNICAL_MACHINES; index++)
        {
            u32 move = GetTMHMMoveId(index);
            EXPECT(IsRandomizerMoveAllowed(move));
            EXPECT(!used[move]);
            used[move] = TRUE;
            EXPECT_EQ(GetItemTMHMMoveId(GetTMHMItemId(index)), move);
            EXPECT_EQ(ItemIdToBattleMoveId(GetTMHMItemId(index)), move);
            EXPECT_EQ(GetTMHMItemIdFromMoveId(move), GetTMHMItemId(index));
        }
        for (u32 index = 0; gTutorMoves[index] != MOVE_UNAVAILABLE; index++)
        {
            u32 move = GetTutorMove(index);
            EXPECT(IsRandomizerMoveAllowed(move));
            EXPECT(!used[move]);
            used[move] = TRUE;
            EXPECT_EQ(GetRandomizedTutorMove(gTutorMoves[index]), move);
        }
    }
    EXPECT_EQ(GetItemTMHMMoveId(ITEM_POTION), MOVE_NONE);
    EXPECT_EQ(GetTMHMItemIdFromMoveId(MOVE_NONE), ITEM_NONE);
    EXPECT_EQ(GetTutorMove(999), MOVE_UNAVAILABLE);
    FlagClear(FLAG_RUN_RULE_TMS_TUTORS);
    for (u32 index = 0; index <= NUM_ALL_MACHINES; index++)
        EXPECT_EQ(GetTMHMMoveId(index), gTMHMItemMoveIds[index].moveId);
    for (u32 index = 0; gTutorMoves[index] != MOVE_UNAVAILABLE; index++)
        EXPECT_EQ(GetTutorMove(index), gTutorMoves[index]);
}

static bool32 HasNaturalMove(enum Species species, enum Move move)
{
    const u16 *teachable = GetSpeciesTeachableLearnset(species);
    const u16 *eggs = GetSpeciesEggMoves(species);
    const struct LevelUpMove *levels = gSpeciesInfo[species].levelUpLearnset;
    for (u32 i = 0; teachable[i] != MOVE_UNAVAILABLE; i++)
        if (teachable[i] == move)
            return TRUE;
    for (u32 i = 0; eggs[i] != MOVE_UNAVAILABLE; i++)
        if (eggs[i] == move)
            return TRUE;
    if (levels != NULL)
        for (u32 i = 0; levels[i].move != LEVEL_UP_MOVE_END; i++)
            if (levels[i].move == move)
                return TRUE;
    return FALSE;
}

TEST("Teaching randomizer compatibility follows assigned moves and both level-up lists")
{
    const u16 species[] = {SPECIES_BULBASAUR, SPECIES_MAGIKARP, SPECIES_MEW, SPECIES_SMEARGLE, SPECIES_GENGAR};
    u32 naturalOnly = 0, randomizedOnly = 0, rejected = 0, sourceMismatch = 0;
    gTestRunnerState.timeoutSeconds = 180;
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    for (u32 seed = 0; seed < 16; seed++)
    {
        gSaveBlock2Ptr->playerTrainerId[0] = seed;
        for (u32 randomized = 0; randomized < 2; randomized++)
        {
            if (randomized)
                FlagSet(FLAG_RUN_RULE_LEARNSETS);
            else
                FlagClear(FLAG_RUN_RULE_LEARNSETS);
            for (u32 s = 0; s < ARRAY_COUNT(species); s++)
            {
                const struct LevelUpMove *levels = GetSpeciesLevelUpLearnset(species[s]);
                for (u32 i = 0; i < 80; i++)
                {
                    u32 move = i < 50 ? GetTMHMMoveId(i + 1) : GetTutorMove(i - 50);
                    bool32 natural = HasNaturalMove(species[s], move), current = FALSE;
                    for (u32 j = 0; levels[j].move != LEVEL_UP_MOVE_END; j++)
                        current |= levels[j].move == move;
                    EXPECT_EQ(CanPlayerLearnTeachableMove(species[s], move), natural || current);
                    naturalOnly += natural && !current;
                    randomizedOnly += !natural && current;
                    rejected += !natural && !current;
                    sourceMismatch += HasNaturalMove(species[s], GetOriginalTeachingMove(move)) != (natural || current);
                    FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
                    EXPECT(CanPlayerLearnTeachableMove(species[s], move));
                    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
                }
            }
        }
    }
    EXPECT_GT(naturalOnly, 0);
    EXPECT_GT(randomizedOnly, 0);
    EXPECT_GT(rejected, 0);
    EXPECT_GT(sourceMismatch, 0);
    EXPECT(!CanPlayerLearnTeachableMove(SPECIES_EGG, GetTutorMove(0)));
    FlagClear(FLAG_RUN_RULE_LEARNSETS);
    FlagClear(FLAG_RUN_RULE_TMS_TUTORS);
}

TEST("Teaching randomizer Pokedex enumeration matches effective compatibility without duplicates")
{
    bool32 full;
    PARAMETRIZE { full = FALSE; }
    PARAMETRIZE { full = TRUE; }
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    if (full)
        FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    bool8 shown[MOVES_COUNT] = {0};
    u32 count = GetPlayerTeachableMoveCount(SPECIES_BULBASAUR);
    for (u32 i = 0; i < count; i++)
    {
        u32 move = GetPlayerTeachableMove(SPECIES_BULBASAUR, i);
        EXPECT_NE(move, MOVE_NONE);
        EXPECT(!shown[move]);
        shown[move] = TRUE;
        EXPECT(CanPlayerLearnTeachableMove(SPECIES_BULBASAUR, move));
    }
    EXPECT_EQ(GetPlayerTeachableMove(SPECIES_BULBASAUR, count), MOVE_NONE);
    for (u32 index = 1; index <= NUM_ALL_MACHINES; index++)
        EXPECT_EQ(shown[GetTMHMMoveId(index)], CanPlayerLearnTeachableMove(SPECIES_BULBASAUR, GetTMHMMoveId(index)));
    for (u32 index = 0; GetTutorMove(index) != MOVE_UNAVAILABLE; index++)
        EXPECT_EQ(shown[GetTutorMove(index)], CanPlayerLearnTeachableMove(SPECIES_BULBASAUR, GetTutorMove(index)));
}

TEST("Teaching randomizer saves stable assignments without advancing gameplay RNG")
{
    u16 moves[80];
    bool8 compatible[80];
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    SeedRng(123);
    u32 next = Random();
    SeedRng(123);
    for (u32 i = 0; i < 80; i++)
    {
        moves[i] = i < 50 ? GetTMHMMoveId(i + 1) : GetTutorMove(i - 50);
        compatible[i] = CanPlayerLearnTeachableMove(SPECIES_BULBASAUR, moves[i]);
    }
    EXPECT_EQ(Random(), next);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    gSaveBlock2Ptr->playerTrainerId[3] ^= 0x80;
    bool32 changed = FALSE;
    for (u32 i = 0; i < 50; i++)
        changed |= GetTMHMMoveId(i + 1) != moves[i];
    EXPECT(changed);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    for (u32 i = 0; i < 80; i++)
    {
        EXPECT_EQ(i < 50 ? GetTMHMMoveId(i + 1) : GetTutorMove(i - 50), moves[i]);
        EXPECT_EQ(CanPlayerLearnTeachableMove(SPECIES_BULBASAUR, moves[i]), compatible[i]);
    }
}

TEST("Teaching randomizer script and Frontier tutors resolve matching assigned moves")
{
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    gSpecialVar_0x8005 = MOVE_SWAGGER;
    RUN_OVERWORLD_SCRIPT(
        callnative ScriptRandomizeTutorMove;
    );
    EXPECT_EQ(gSpecialVar_0x8005, GetRandomizedTutorMove(MOVE_SWAGGER));
    EXPECT_EQ(StringCompare(gStringVar1, GetMoveName(gSpecialVar_0x8005)), 0);
    RUN_OVERWORLD_SCRIPT(
        callnative ScriptPrepareRandomizedTMText;
        .2byte ITEM_TM_BULLET_SEED;
    );
    EXPECT_EQ(StringCompare(gStringVar2, GetMoveName(GetItemTMHMMoveId(ITEM_TM_BULLET_SEED))), 0);
    EXPECT_EQ(StringCompare(gStringVar3, GetMoveDescription(GetItemTMHMMoveId(ITEM_TM_BULLET_SEED))), 0);
    for (u32 tutor = 0; tutor < 2; tutor++)
        for (u32 i = 0; i < 10; i++)
        {
            u32 move = GetFrontierTutorMove(tutor, i);
            EXPECT_NE(move, MOVE_NONE);
            EXPECT_EQ(GetRandomizedTutorMove(GetOriginalTeachingMove(move)), move);
        }
}

TEST("Teaching randomizer move descriptions and Frontier labels fit their windows")
{
    struct ListMenuItem items[11];
    SetDefaultFontsPointer();
    FlagSet(FLAG_RUN_RULE_TMS_TUTORS);
    FlagSet(FLAG_RUN_RULE_FROSTBITE);
    for (u32 move = 1; move < MOVES_COUNT; move++)
    {
        if (!IsRandomizerMoveAllowed(move))
            continue;
        const u8 *description = GetRandomizedMoveDescription(move, 96);
        if (GetStringWidth(FONT_NORMAL, description, 0) > 96)
            Test_MgbaPrintf("Description for %S: width %d", GetMoveName(move), GetStringWidth(FONT_NORMAL, description, 0));
        EXPECT_LE(GetStringWidth(FONT_NORMAL, description, 0), 96);
        u32 lines = 1;
        for (u32 i = 0; description[i] != EOS; i++)
            if (description[i] == CHAR_NEWLINE)
                lines++;
        EXPECT_LE(lines, 4);
        EXPECT_LE(GetStringWidth(FONT_SMALL, GetMoveName(move), 0), 86);
    }
    for (u32 tutor = 0; tutor < 2; tutor++)
    {
        u32 menu = tutor ? SCROLL_MULTI_BF_MOVE_TUTOR_2 : SCROLL_MULTI_BF_MOVE_TUTOR_1;
        EXPECT_EQ(Test_BuildScrollableMultichoiceItems(menu, 11, items), 11);
        for (u32 i = 0; i < 10; i++)
        {
            EXPECT_EQ(items[i].id, i);
            EXPECT(GetStringWidth(FONT_NORMAL, items[i].name, 0) <= 112);
        }
    }
    for (u32 i = 1; i <= NUM_TECHNICAL_MACHINES; i++)
    {
        u8 description[256];
        StringCopy(description, GetItemDescription(GetTMHMItemId(i)));
        EXPECT_EQ(StringCompare(description, GetRandomizedMoveDescription(GetTMHMMoveId(i), 106)), 0);
    }
}

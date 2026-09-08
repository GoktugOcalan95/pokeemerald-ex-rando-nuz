#include "global.h"
#include "event_data.h"
#include "move.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/items.h"

TEST("Setup move PP: exact move list, PP bonuses, learning, restoration, and PP items")
{
    static const enum Move limitedMoves[] = {
        MOVE_AGILITY, MOVE_AUTOTOMIZE, MOVE_CLANGOROUS_SOUL, MOVE_COTTON_GUARD,
        MOVE_DRAGON_DANCE, MOVE_MINIMIZE, MOVE_NASTY_PLOT, MOVE_QUIVER_DANCE,
        MOVE_ROCK_POLISH, MOVE_SHELL_SMASH, MOVE_SHIFT_GEAR, MOVE_SWORDS_DANCE,
        MOVE_TAIL_GLOW, MOVE_TIDY_UP, MOVE_VICTORY_DANCE,
    };
    struct Pokemon mon;
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }

    if (enabled)
        FlagSet(FLAG_RUN_RULE_SETUP_MOVE_PP);
    else
        FlagClear(FLAG_RUN_RULE_SETUP_MOVE_PP);
    for (enum Move move = MOVE_NONE; move < MOVES_COUNT; move++)
    {
        bool32 limited = FALSE;
        for (u32 i = 0; i < ARRAY_COUNT(limitedMoves); i++)
            if (move == limitedMoves[i])
                limited = enabled;
        EXPECT_EQ(GetMovePP(move), limited ? 1 : gMovesInfo[move].pp);
        for (u32 bonus = 0; bonus <= 3; bonus++)
            EXPECT_EQ(CalculatePPWithBonus(move, bonus, 0), limited ? 1 : gMovesInfo[move].pp + gMovesInfo[move].pp * 20 * bonus / 100);
    }
    for (u32 i = 0; i < ARRAY_COUNT(limitedMoves); i++)
    {
        CreateRandomMon(&mon, SPECIES_WOBBUFFET, 5);
        SetMonMoveSlot(&mon, limitedMoves[i], 0);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_PP1), GetMovePP(limitedMoves[i]));
        EXPECT_EQ(ExecuteTableBasedItemEffect(&mon, ITEM_PP_UP, 0, 0), enabled);
        EXPECT_EQ(ExecuteTableBasedItemEffect(&mon, ITEM_PP_MAX, 0, 0), enabled);
        u32 pp = 0;
        SetMonData(&mon, MON_DATA_PP1, &pp);
        EXPECT_EQ(ExecuteTableBasedItemEffect(&mon, ITEM_ETHER, 0, 0), FALSE);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_PP1), min(10, CalculatePPWithBonus(limitedMoves[i], GetMonData(&mon, MON_DATA_PP_BONUSES), 0)));
        SetMonData(&mon, MON_DATA_PP1, &pp);
        MonRestorePP(&mon);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_PP1), CalculatePPWithBonus(limitedMoves[i], GetMonData(&mon, MON_DATA_PP_BONUSES), 0));
    }
    FlagClear(FLAG_RUN_RULE_SETUP_MOVE_PP);
}

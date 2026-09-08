#include "global.h"
#include "event_data.h"
#include "frostbite.h"
#include "item.h"
#include "move.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Frostbite descriptions follow the saved rule and restore when disabled")
{
    static const enum Move moves[] = {MOVE_ICE_PUNCH, MOVE_ICE_BEAM, MOVE_BLIZZARD, MOVE_TRI_ATTACK, MOVE_POWDER_SNOW, MOVE_ICE_FANG, MOVE_FREEZE_DRY, MOVE_FREEZING_GLARE};
    static const enum Item items[] = {ITEM_ICE_HEAL, ITEM_ASPEAR_BERRY, ITEM_TM13, ITEM_TM14};

    FlagSet(FLAG_RUN_RULE_FROSTBITE);
    EXPECT(IsFrostbiteEnabled());
    for (u32 i = 0; i < ARRAY_COUNT(moves); i++)
        EXPECT(GetMoveDescription(moves[i]) != gMovesInfo[moves[i]].description);
    for (u32 i = 0; i < ARRAY_COUNT(items); i++)
        EXPECT(GetItemDescription(items[i]) != gItemsInfo[items[i]].description);
    EXPECT(GetAbilityDescription(ABILITY_MAGMA_ARMOR) != gAbilitiesInfo[ABILITY_MAGMA_ARMOR].description);
    EXPECT_EQ(GetMoveDescription(MOVE_TACKLE), gMovesInfo[MOVE_TACKLE].description);
    EXPECT_EQ(GetItemDescription(ITEM_POTION), gItemsInfo[ITEM_POTION].description);
    EXPECT_EQ(GetAbilityDescription(ABILITY_STURDY), gAbilitiesInfo[ABILITY_STURDY].description);
    FlagClear(FLAG_RUN_RULE_FROSTBITE);
    EXPECT(!IsFrostbiteEnabled());
    for (u32 i = 0; i < ARRAY_COUNT(moves); i++)
        EXPECT_EQ(GetMoveDescription(moves[i]), gMovesInfo[moves[i]].description);
    for (u32 i = 0; i < ARRAY_COUNT(items); i++)
        EXPECT_EQ(GetItemDescription(items[i]), gItemsInfo[items[i]].description);
    EXPECT_EQ(GetAbilityDescription(ABILITY_MAGMA_ARMOR), gAbilitiesInfo[ABILITY_MAGMA_ARMOR].description);
}

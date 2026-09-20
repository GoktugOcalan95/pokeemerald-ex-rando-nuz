#include "global.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "save.h"
#include "test/overworld_script.h"
#include "constants/vars.h"

asm(".set VAR_WHICH_FOSSIL_REVIVED, " STR(VAR_WHICH_FOSSIL_REVIVED) "\n");

TEST("Devon fossils consume one item and revive the matching or seeded species")
{
    static const u16 items[] = {ITEM_HELIX_FOSSIL, ITEM_DOME_FOSSIL, ITEM_OLD_AMBER, ITEM_ROOT_FOSSIL, ITEM_CLAW_FOSSIL, ITEM_ARMOR_FOSSIL, ITEM_SKULL_FOSSIL, ITEM_COVER_FOSSIL, ITEM_PLUME_FOSSIL, ITEM_JAW_FOSSIL, ITEM_SAIL_FOSSIL, ITEM_FOSSILIZED_BIRD, ITEM_FOSSILIZED_FISH, ITEM_FOSSILIZED_DRAKE, ITEM_FOSSILIZED_DINO};
    static const u16 species[] = {SPECIES_OMANYTE, SPECIES_KABUTO, SPECIES_AERODACTYL, SPECIES_LILEEP, SPECIES_ANORITH, SPECIES_SHIELDON, SPECIES_CRANIDOS, SPECIES_TIRTOUGA, SPECIES_ARCHEN, SPECIES_TYRUNT, SPECIES_AMAURA, SPECIES_ARCTOZOLT, SPECIES_ARCTOVISH, SPECIES_DRACOVISH, SPECIES_DRACOZOLT};
    u32 index = 0;
    bool32 randomized = FALSE;
    for (u32 i = 0; i < ARRAY_COUNT(items); i++)
    {
        PARAMETRIZE { index = i; randomized = FALSE; }
        PARAMETRIZE { index = i; randomized = TRUE; }
    }
    InitEventData();
    ClearBag();
    ZeroPlayerPartyMons();
    if (randomized)
        FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    AddBagItem(items[index], 2);
    gSpecialVar_0x8008 = items[index];
    RUN_OVERWORLD_SCRIPT(call RustboroCity_DevonCorp_2F_EventScript_ConsumeFossil;);
    EXPECT_EQ(CountTotalItemQuantityInBag(items[index]), 1);
    EXPECT_EQ(VarGet(VAR_FOSSIL_RESURRECTION_STATE), 1);
    EXPECT_EQ(VarGet(VAR_WHICH_FOSSIL_REVIVED), items[index]);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    InitEventData();
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(VarGet(VAR_WHICH_FOSSIL_REVIVED), items[index]);
    RUN_OVERWORLD_SCRIPT(call RustboroCity_DevonCorp_2F_EventScript_PreviewFossil;);
    u16 expected = VarGet(VAR_TEMP_TRANSFERRED_SPECIES);
    RUN_OVERWORLD_SCRIPT(call RustboroCity_DevonCorp_2F_EventScript_GiveFossil;);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), expected);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_LEVEL), 20);
    if (!randomized)
        EXPECT_EQ(expected, species[index]);
    InitEventData();
    ZeroPlayerPartyMons();
}

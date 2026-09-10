#include "global.h"
#include "event_data.h"
#include "post_juan.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/vars.h"

TEST("Post-Juan unlocks require Juan and open every puzzle with either story setting")
{
    static const u16 flags[] = {FLAG_REGI_DOORS_OPENED, FLAG_SYS_BRAILLE_DIG,
        FLAG_SYS_REGIROCK_PUZZLE_COMPLETED, FLAG_SYS_BRAILLE_REGICE_COMPLETED,
        FLAG_SYS_REGISTEEL_PUZZLE_COMPLETED, FLAG_WALLACE_GOES_TO_SKY_PILLAR};
    bool32 removeStory;
    PARAMETRIZE { removeStory = FALSE; }
    PARAMETRIZE { removeStory = TRUE; }

    FlagClear(FLAG_POST_JUAN_LEGENDARIES_UNLOCKED);
    FlagClear(FLAG_BADGE08_GET);
    if (removeStory)
        FlagSet(FLAG_RUN_RULE_REMOVE_STORY);
    else
        FlagClear(FLAG_RUN_RULE_REMOVE_STORY);
    for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
        FlagClear(flags[i]);
    VarSet(VAR_SKY_PILLAR_STATE, 0);
    VarSet(VAR_SKY_PILLAR_RAYQUAZA_CRY_DONE, 0);
    EXPECT(!TryUnlockPostJuanLegendaries());
    EXPECT_EQ(VarGet(VAR_SKY_PILLAR_STATE), 0);
    FlagSet(FLAG_BADGE08_GET);
    FlagSet(FLAG_DEFEATED_RAYQUAZA);
    FlagSet(FLAG_DEFEATED_REGIROCK);
    EXPECT(TryUnlockPostJuanLegendaries());
    for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
        EXPECT(FlagGet(flags[i]));
    EXPECT_EQ(VarGet(VAR_SKY_PILLAR_STATE), 2);
    EXPECT_EQ(VarGet(VAR_SKY_PILLAR_RAYQUAZA_CRY_DONE), 1);
    EXPECT(FlagGet(FLAG_DEFEATED_RAYQUAZA));
    EXPECT(FlagGet(FLAG_DEFEATED_REGIROCK));
    EXPECT(!TryUnlockPostJuanLegendaries());
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    FlagClear(FLAG_POST_JUAN_LEGENDARIES_UNLOCKED);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(!TryUnlockPostJuanLegendaries());
    FlagClear(FLAG_POST_JUAN_LEGENDARIES_UNLOCKED);
    FlagClear(FLAG_BADGE08_GET);
    FlagClear(FLAG_RUN_RULE_REMOVE_STORY);
    FlagClear(FLAG_DEFEATED_RAYQUAZA);
    FlagClear(FLAG_DEFEATED_REGIROCK);
    for (u32 i = 0; i < ARRAY_COUNT(flags); i++)
        FlagClear(flags[i]);
}

#include "global.h"
#include "berry.h"
#include "berry_plots.h"
#include "event_data.h"
#include "item.h"
#include "save.h"
#include "test/test.h"
#include "constants/berry.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/flags.h"

extern const u8 BerryTreeScript[];

TEST("Berry plots keep uncollected balls and restore all 88 collected plots without duplicate tree IDs")
{
    bool8 used[BERRY_TREES_COUNT] = {0};
    struct ObjectEventTemplate template;

    for (u32 flag = FLAG_ITEM_BERRY_PLOT_ROUTE_102_ORAN; flag <= FLAG_ITEM_BERRY_PLOT_ROUTE_130_LIECHI; flag++)
    {
        template = (struct ObjectEventTemplate){.flagId = flag, .graphicsId = OBJ_EVENT_GFX_ITEM_BALL, .x = 24, .y = 2, .localId = 7};
        FlagClear(flag);
        EXPECT(!RestoreCollectedBerryPlot(&template));
        EXPECT_EQ(template.graphicsId, OBJ_EVENT_GFX_ITEM_BALL);
        FlagSet(flag);
        EXPECT(RestoreCollectedBerryPlot(&template));
        EXPECT_EQ(template.graphicsId, OBJ_EVENT_GFX_BERRY_TREE);
        EXPECT_EQ(template.movementType, MOVEMENT_TYPE_BERRY_TREE_GROWTH);
        EXPECT_EQ(template.script, BerryTreeScript);
        EXPECT_EQ(template.flagId, flag);
        EXPECT_EQ(template.x, 24);
        EXPECT_EQ(template.y, 2);
        EXPECT_EQ(template.localId, 7);
        EXPECT(template.trainerRange_berryTreeId < BERRY_TREES_COUNT);
        EXPECT(!used[template.trainerRange_berryTreeId]);
        used[template.trainerRange_berryTreeId] = TRUE;
        FlagClear(flag);
    }
}

TEST("Berry plots preserve collection and gardening through growth harvest replanting and save load")
{
    struct ObjectEventTemplate template = {.flagId = FLAG_ITEM_BERRY_PLOT_ROUTE_102_ORAN};
    struct BerryTree *tree;
    u32 treeId;

    ClearBag();
    ClearBerryTrees();
    FlagSet(template.flagId);
    EXPECT(RestoreCollectedBerryPlot(&template));
    treeId = template.trainerRange_berryTreeId;
    EXPECT_EQ(treeId, BERRY_TREE_ROUTE_102_ORAN);
    EXPECT_EQ(GetStageByBerryTreeId(treeId), BERRY_STAGE_NO_BERRY);
    PlantBerryTree(treeId, BERRY_ID_ORAN, BERRY_STAGE_PLANTED, TRUE);
    tree = GetBerryTreeInfo(treeId);
    for (u32 i = 0; i < 8 && tree->stage != BERRY_STAGE_BERRIES; i++)
        EXPECT(BerryTreeGrow(tree));
    EXPECT_EQ((u32)tree->stage, BERRY_STAGE_BERRIES);
    EXPECT(tree->berryYield > 0);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    ClearBerryTrees();
    FlagClear(template.flagId);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(RestoreCollectedBerryPlot(&template));
    EXPECT_EQ(GetStageByBerryTreeId(treeId), BERRY_STAGE_BERRIES);
    gSelectedObjectEvent = 0;
    gObjectEvents[0].trainerRange_berryTreeId = treeId;
    ObjectEventInteractionPickBerryTree();
    EXPECT_EQ(gSpecialVar_0x8004, TRUE);
    EXPECT(CheckBagHasItem(ITEM_ORAN_BERRY, 1));
    RemoveBerryTree(treeId);
    EXPECT_EQ(GetStageByBerryTreeId(treeId), BERRY_STAGE_NO_BERRY);
    PlantBerryTree(treeId, BERRY_ID_PECHA, BERRY_STAGE_PLANTED, TRUE);
    EXPECT(RestoreCollectedBerryPlot(&template));
    EXPECT_EQ(GetBerryTypeByBerryTreeId(treeId), BERRY_ID_PECHA);
    EXPECT_EQ(template.script, BerryTreeScript);
    ClearBerryTrees();
    FlagClear(template.flagId);
    ClearBag();
}

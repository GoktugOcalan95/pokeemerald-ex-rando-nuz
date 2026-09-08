#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "save.h"
#include "script_movement.h"
#include "sprite.h"
#include "task.h"
#include "test/test.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/flags.h"

static void InitPickupObjects(void)
{
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
    for (u32 i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        gObjectEvents[i].active = TRUE;
        gObjectEvents[i].localId = i + 1;
        gObjectEvents[i].mapNum = MAP_NUM(MAP_ROUTE116);
        gObjectEvents[i].mapGroup = MAP_GROUP(MAP_ROUTE116);
        gObjectEvents[i].graphicsId = OBJ_EVENT_GFX_ITEM_BALL;
        gObjectEvents[i].currentCoords.x = i + 7;
        gObjectEvents[i].currentCoords.y = 10;
    }
}

TEST("Visible pickups: movement completion tracks all 24 objects without corrupting IDs")
{
    static const u8 movement[] = {MOVEMENT_ACTION_STEP_END};
    ResetTasks();
    ResetSpriteData();
    InitPickupObjects();
    for (u32 i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        EXPECT(!ScriptMovement_StartObjectMovementScript(i + 1, MAP_NUM(MAP_ROUTE116), MAP_GROUP(MAP_ROUTE116), movement));
        EXPECT(!ScriptMovement_IsObjectMovementFinished(i + 1, MAP_NUM(MAP_ROUTE116), MAP_GROUP(MAP_ROUTE116)));
    }
    EXPECT(!ScriptMovement_IsAllObjectMovementFinished());
    RunTasks();
    for (u32 i = 0; i < OBJECT_EVENTS_COUNT; i++)
        EXPECT(ScriptMovement_IsObjectMovementFinished(i + 1, MAP_NUM(MAP_ROUTE116), MAP_GROUP(MAP_ROUTE116)));
    EXPECT(ScriptMovement_IsAllObjectMovementFinished());

    EXPECT(!ScriptMovement_StartObjectMovementScript(OBJECT_EVENTS_COUNT, MAP_NUM(MAP_ROUTE116), MAP_GROUP(MAP_ROUTE116), movement));
    EXPECT(!ScriptMovement_IsAllObjectMovementFinished());
    EXPECT(ScriptMovement_IsObjectMovementFinished(1, MAP_NUM(MAP_ROUTE116), MAP_GROUP(MAP_ROUTE116)));
    RunTasks();
    EXPECT(ScriptMovement_IsAllObjectMovementFinished());
    ScriptMovement_UnfreezeObjectEvents();
    for (u32 i = 0; i < OBJECT_EVENTS_COUNT; i++)
        EXPECT(!gObjectEvents[i].frozen);
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
}

TEST("Visible pickups: all object slots and collection flags survive saving and loading")
{
    InitPickupObjects();
    for (u32 flag = FLAG_ITEM_BERRY_PLOT_ROUTE_102_ORAN; flag <= FLAG_ITEM_BERRY_PLOT_ROUTE_130_LIECHI; flag++)
    {
        if (flag & 1)
            FlagSet(flag);
        else
            FlagClear(flag);
    }
    FlagSet(FLAG_HIDDEN_ITEM_ABANDONED_SHIP_RM_6_KEY);
    FlagClear(FLAG_HIDDEN_ITEM_TRICK_HOUSE_NUGGET);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
    for (u32 flag = FLAG_ITEM_BERRY_PLOT_ROUTE_102_ORAN; flag <= FLAG_ITEM_BERRY_PLOT_ROUTE_130_LIECHI; flag++)
        FlagClear(flag);
    FlagClear(FLAG_HIDDEN_ITEM_ABANDONED_SHIP_RM_6_KEY);
    FlagSet(FLAG_HIDDEN_ITEM_TRICK_HOUSE_NUGGET);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    for (u32 i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        EXPECT(gObjectEvents[i].active);
        EXPECT_EQ(gObjectEvents[i].localId, i + 1);
        EXPECT_EQ(gObjectEvents[i].graphicsId, OBJ_EVENT_GFX_ITEM_BALL);
        EXPECT_EQ(gObjectEvents[i].currentCoords.x, i + 7);
    }
    for (u32 flag = FLAG_ITEM_BERRY_PLOT_ROUTE_102_ORAN; flag <= FLAG_ITEM_BERRY_PLOT_ROUTE_130_LIECHI; flag++)
    {
        EXPECT_EQ(FlagGet(flag), flag & 1);
        FlagClear(flag);
    }
    EXPECT(FlagGet(FLAG_HIDDEN_ITEM_ABANDONED_SHIP_RM_6_KEY));
    EXPECT(!FlagGet(FLAG_HIDDEN_ITEM_TRICK_HOUSE_NUGGET));
    FlagClear(FLAG_HIDDEN_ITEM_ABANDONED_SHIP_RM_6_KEY);
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
}

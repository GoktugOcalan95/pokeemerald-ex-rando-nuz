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

TEST("Visible pickups: all 24 object slots survive saving and loading")
{
    InitPickupObjects();
    ClearSaveData();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    for (u32 i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        EXPECT(gObjectEvents[i].active);
        EXPECT_EQ(gObjectEvents[i].localId, i + 1);
        EXPECT_EQ(gObjectEvents[i].graphicsId, OBJ_EVENT_GFX_ITEM_BALL);
        EXPECT_EQ(gObjectEvents[i].currentCoords.x, i + 7);
    }
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
}

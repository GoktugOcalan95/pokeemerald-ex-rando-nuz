#include "global.h"
#include "mach_bike_assist.h"
#include "bike.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "field_control_avatar.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "main.h"
#include "overworld.h"
#include "script.h"
#include "sprite.h"
#include "test/test.h"
#include "constants/event_objects.h"
#include "constants/layouts.h"
#include "constants/metatile_behaviors.h"

#define ASSIST_X 22
#define ASSIST_Y 22

static void InitAssistField(struct MapLayout *layout, struct Tileset *tileset, const u16 *attributes)
{
    ClearBag();
    FlagClear(FLAG_BADGE01_GET);
    VarSet(VAR_MOUNTED_BIKE, ITEM_MACH_BIKE);
    ScriptContext_Init();
    ResetSpriteData();
    ClearPlayerAvatarInfo();
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_ROUTE110), MAP_NUM(MAP_ROUTE110));
    InitMap();
    *layout = *gMapHeader.mapLayout;
    *tileset = *layout->primaryTileset;
    tileset->metatileAttributes = attributes;
    layout->primaryTileset = tileset;
    gMapHeader.mapLayout = layout;
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_MACH_BIKE;
    gObjectEvents[0].active = TRUE;
    gObjectEvents[0].isPlayer = TRUE;
    gObjectEvents[0].inanimate = TRUE;
    gObjectEvents[0].localId = LOCALID_PLAYER;
    gObjectEvents[0].currentElevation = ELEVATION_DEFAULT;
    gObjectEvents[0].previousElevation = ELEVATION_DEFAULT;
    gObjectEvents[0].currentCoords.x = ASSIST_X;
    gObjectEvents[0].currentCoords.y = ASSIST_Y;
    gObjectEvents[0].previousCoords = gObjectEvents[0].currentCoords;
    gObjectEvents[0].facingDirection = DIR_NORTH;
    gObjectEvents[0].movementDirection = DIR_NORTH;
    gSprites[0].anims = gDummySpriteAnimTable;
    for (u32 y = ASSIST_Y - 11; y <= ASSIST_Y + 11; y++)
        for (u32 x = ASSIST_X - 11; x <= ASSIST_X + 11; x++)
            gBackupMapLayout.map[y * gBackupMapLayout.width + x] = ELEVATION_DEFAULT << MAPGRID_ELEVATION_SHIFT;
    for (u32 x = ASSIST_X - 11; x <= ASSIST_X + 11; x++)
        MapGridSetMetatileIdAt(x, ASSIST_Y - 1, MAPGRID_IMPASSABLE);
}

TEST("Mach assistance finds the nearest reachable opening and favors right through distance ten")
{
    struct MapHeader savedMap = gMapHeader;
    struct MapLayout layout;
    struct Tileset tileset;
    const u16 attributes[] = {MB_NORMAL};
    InitAssistField(&layout, &tileset, attributes);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_NONE);
    MapGridSetMetatileIdAt(ASSIST_X + 11, ASSIST_Y - 1, 0);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_NONE);
    MapGridSetMetatileIdAt(ASSIST_X + 10, ASSIST_Y - 1, 0);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_EAST);
    MapGridSetMetatileIdAt(ASSIST_X - 3, ASSIST_Y - 1, 0);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_WEST);
    MapGridSetMetatileIdAt(ASSIST_X + 3, ASSIST_Y - 1, 0);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_EAST);
    MapGridSetMetatileIdAt(ASSIST_X - 2, ASSIST_Y - 1, 0);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_WEST);
    MapGridSetMetatileIdAt(ASSIST_X - 1, ASSIST_Y, MAPGRID_IMPASSABLE);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_EAST);
    MapGridSetMetatileIdAt(ASSIST_X, ASSIST_Y - 1, 0);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_NONE);
    gMapHeader = savedMap;
}

TEST("Mach assistance obeys hold B Fortree and Machro switching priority")
{
    struct MapHeader savedMap = gMapHeader;
    struct MapLayout layout;
    struct Tileset tileset;
    const u16 attributes[] = {MB_NORMAL};
    InitAssistField(&layout, &tileset, attributes);
    MapGridSetMetatileIdAt(ASSIST_X + 1, ASSIST_Y - 1, 0);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, 0), DIR_NONE);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NONE, B_BUTTON), DIR_NONE);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_ACRO_BIKE;
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_NONE);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_MACH_BIKE;
    VarSet(VAR_MOUNTED_BIKE, ITEM_BICYCLE);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_EAST);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, L_BUTTON | B_BUTTON), DIR_NONE);
    gMapHeader.mapLayoutId = LAYOUT_FORTREE_CITY_GYM;
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_NONE);
    gMapHeader = savedMap;
}

TEST("Mach assistance cannot search through terrain or moving objects and permits cracked floors")
{
    struct MapHeader savedMap = gMapHeader;
    struct MapLayout layout;
    struct Tileset tileset;
    u16 attributes[] = {MB_NORMAL, MB_POND_WATER};
    InitAssistField(&layout, &tileset, attributes);
    MapGridSetMetatileIdAt(ASSIST_X + 3, ASSIST_Y - 1, 0);
    MapGridSetMetatileIdAt(ASSIST_X + 2, ASSIST_Y, 1);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_NONE);
    attributes[1] = MB_CRACKED_FLOOR;
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_EAST);
    gObjectEvents[1].active = TRUE;
    gObjectEvents[1].currentCoords.x = ASSIST_X + 2;
    gObjectEvents[1].currentCoords.y = ASSIST_Y;
    gObjectEvents[1].previousCoords = gObjectEvents[1].currentCoords;
    gObjectEvents[1].currentElevation = ELEVATION_DEFAULT;
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_NONE);
    gObjectEvents[1].currentCoords.y++;
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_NONE);
    gObjectEvents[1].active = FALSE;
    gBackupMapLayout.map[ASSIST_Y * gBackupMapLayout.width + ASSIST_X + 2] = 4 << MAPGRID_ELEVATION_SHIFT;
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_NONE);
    gMapHeader = savedMap;
}

TEST("Mach assistance defers to an eligible automatic Cut without starting a script during its probe")
{
    struct MapHeader savedMap = gMapHeader;
    struct MapLayout layout;
    struct Tileset tileset;
    const u16 attributes[] = {MB_NORMAL};
    const struct MapHeader *treeMap = Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_ROUTE104), MAP_NUM(MAP_ROUTE104));
    const struct ObjectEventTemplate *tree = NULL;
    for (u32 i = 0; i < treeMap->events->objectEventCount; i++)
        if (treeMap->events->objectEvents[i].script == EventScript_CutTree)
            tree = &treeMap->events->objectEvents[i];
    ASSUME(tree != NULL);
    InitAssistField(&layout, &tileset, attributes);
    MapGridSetMetatileIdAt(ASSIST_X + 1, ASSIST_Y - 1, 0);
    gObjectEvents[1].active = TRUE;
    gObjectEvents[1].localId = tree->localId;
    gObjectEvents[1].mapGroup = MAP_GROUP(MAP_ROUTE104);
    gObjectEvents[1].mapNum = MAP_NUM(MAP_ROUTE104);
    gObjectEvents[1].currentCoords.x = ASSIST_X;
    gObjectEvents[1].currentCoords.y = ASSIST_Y - 1;
    gObjectEvents[1].previousCoords = gObjectEvents[1].currentCoords;
    gObjectEvents[1].currentElevation = ELEVATION_DEFAULT;
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_EAST);
    EXPECT(AddBagItem(ITEM_HM01, 1));
    FlagSet(FLAG_BADGE01_GET);
    EXPECT(CanUseAutomaticHMInDirection(DIR_NORTH));
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_NONE);
    EXPECT(!ArePlayerFieldControlsLocked());
    ClearBag();
    FlagClear(FLAG_BADGE01_GET);
    gMapHeader = savedMap;
}

static bool8 AssistMovementCallback(struct ObjectEvent *object, struct Sprite *sprite)
{
    return FALSE;
}

TEST("Mach assistance advances one ordinary tile while retaining high speed and momentum")
{
    struct MapHeader savedMap = gMapHeader;
    struct MapLayout layout;
    struct Tileset tileset;
    const u16 attributes[] = {MB_NORMAL};
    InitAssistField(&layout, &tileset, attributes);
    MapGridSetMetatileIdAt(ASSIST_X + 3, ASSIST_Y - 1, 0);
    gPlayerAvatar.runningState = MOVING;
    gPlayerAvatar.bikeFrameCounter = 2;
    gPlayerAvatar.bikeSpeed = PLAYER_SPEED_FASTER;
    gMain.heldKeys = B_BUTTON;
    MovePlayerOnBike(DIR_NORTH, 0, B_BUTTON);
    EXPECT_EQ(gObjectEvents[0].movementActionId, GetWalkFasterMovementAction(DIR_EAST));
    EXPECT_EQ(gPlayerAvatar.bikeFrameCounter, 2);
    EXPECT_EQ(gPlayerAvatar.bikeSpeed, PLAYER_SPEED_FASTER);
    for (u32 frame = 0; frame < 32 && !ObjectEventCheckHeldMovementStatus(&gObjectEvents[0]); frame++)
        UpdateObjectEventCurrentMovement(&gObjectEvents[0], &gSprites[0], AssistMovementCallback);
    EXPECT(ObjectEventCheckHeldMovementStatus(&gObjectEvents[0]));
    EXPECT_EQ(gObjectEvents[0].currentCoords.x, ASSIST_X + 1);
    EXPECT_EQ(gObjectEvents[0].currentCoords.y, ASSIST_Y);
    ObjectEventClearHeldMovementIfFinished(&gObjectEvents[0]);
    MapGridSetMetatileIdAt(ASSIST_X + 3, ASSIST_Y - 1, MAPGRID_IMPASSABLE);
    MovePlayerOnBike(DIR_NORTH, 0, B_BUTTON);
    EXPECT_EQ(gPlayerAvatar.bikeSpeed, PLAYER_SPEED_STANDING);
    EXPECT_EQ(gPlayerAvatar.bikeFrameCounter, 0);
    gMain.heldKeys = 0;
    gMapHeader = savedMap;
}

TEST("Mach assistance uses relative right for every heading and does not cross map edges")
{
    static const enum Direction right[] = {DIR_NONE, DIR_WEST, DIR_EAST, DIR_NORTH, DIR_SOUTH};
    struct MapHeader savedMap = gMapHeader;
    struct MapLayout layout;
    struct Tileset tileset;
    const u16 attributes[] = {MB_NORMAL};
    InitAssistField(&layout, &tileset, attributes);
    for (u32 x = ASSIST_X - 11; x <= ASSIST_X + 11; x++)
        MapGridSetMetatileIdAt(x, ASSIST_Y - 1, 0);
    for (enum Direction direction = DIR_SOUTH; direction <= DIR_EAST; direction++)
    {
        s16 x = ASSIST_X, y = ASSIST_Y;
        MoveCoords(direction, &x, &y);
        MapGridSetMetatileIdAt(x, y, MAPGRID_IMPASSABLE);
        EXPECT_EQ(GetMachBikeAssistanceDirection(direction, B_BUTTON), right[direction]);
        MapGridSetMetatileIdAt(x, y, 0);
    }
    gObjectEvents[0].currentCoords.x = MAP_OFFSET;
    MapGridSetMetatileIdAt(MAP_OFFSET, ASSIST_Y, 0);
    MapGridSetMetatileIdAt(MAP_OFFSET, ASSIST_Y - 1, MAPGRID_IMPASSABLE);
    MapGridSetMetatileIdAt(MAP_OFFSET + 1, ASSIST_Y, MAPGRID_IMPASSABLE);
    EXPECT_EQ(GetMachBikeAssistanceDirection(DIR_NORTH, B_BUTTON), DIR_NONE);
    gMapHeader = savedMap;
}

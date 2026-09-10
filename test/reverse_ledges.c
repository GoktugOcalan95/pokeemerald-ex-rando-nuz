#include "global.h"
#include "bike.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "overworld.h"
#include "script.h"
#include "test/test.h"
#include "constants/event_objects.h"
#include "constants/metatile_behaviors.h"

static void InitLedgeField(struct MapLayout *layout, struct Tileset *tileset, const u16 *attributes)
{
    ScriptContext_Init();
    ClearPlayerAvatarInfo();
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_ROUTE104), MAP_NUM(MAP_ROUTE104));
    InitMap();
    *layout = *gMapHeader.mapLayout;
    *tileset = *layout->primaryTileset;
    tileset->metatileAttributes = attributes;
    layout->primaryTileset = tileset;
    gMapHeader.mapLayout = layout;
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_ACRO_BIKE;
    gPlayerAvatar.acroBikeState = ACRO_STATE_BUNNY_HOP;
    gObjectEvents[0].active = TRUE;
    gObjectEvents[0].isPlayer = TRUE;
    gObjectEvents[0].localId = LOCALID_PLAYER;
    gObjectEvents[0].currentElevation = ELEVATION_DEFAULT;
    gObjectEvents[0].previousElevation = ELEVATION_DEFAULT;
    gObjectEvents[0].currentCoords.x = 12;
    gObjectEvents[0].currentCoords.y = 12;
    gObjectEvents[0].previousCoords = gObjectEvents[0].currentCoords;
    for (u32 y = 9; y <= 15; y++)
        for (u32 x = 9; x <= 15; x++)
            gBackupMapLayout.map[y * gBackupMapLayout.width + x] = ELEVATION_DEFAULT << MAPGRID_ELEVATION_SHIFT;
}

TEST("Reverse ledges allow every opposite direction indoors and outdoors in Acro mode")
{
    static const u16 ledges[] = {MB_JUMP_NORTH, MB_JUMP_SOUTH, MB_JUMP_EAST, MB_JUMP_WEST};
    struct MapHeader savedMap = gMapHeader;
    struct MapLayout layout;
    struct Tileset tileset;
    u16 attributes[] = {MB_NORMAL, MB_JUMP_SOUTH};
    InitLedgeField(&layout, &tileset, attributes);
    for (u32 indoors = 0; indoors < 2; indoors++)
    {
        gMapHeader.mapType = indoors ? MAP_TYPE_INDOOR : MAP_TYPE_ROUTE;
        for (enum Direction direction = DIR_SOUTH; direction <= DIR_EAST; direction++)
        {
            s16 x = 12, y = 12;
            MoveCoords(direction, &x, &y);
            attributes[1] = ledges[direction - 1];
            MapGridSetMetatileIdAt(x, y, 1 | MAPGRID_IMPASSABLE);
            EXPECT(CanReverseLedgeHop(x, y, direction));
            EXPECT_EQ(CheckForObjectEventCollision(&gObjectEvents[0], x, y, direction, attributes[1]), COLLISION_LEDGE_JUMP);
            MapGridSetMetatileIdAt(x, y, 0);
        }
    }
    gMapHeader = savedMap;
}

TEST("Reverse ledges require deliberate bunny hopping and retain ordinary forward jumps")
{
    struct MapHeader savedMap = gMapHeader;
    struct MapLayout layout;
    struct Tileset tileset;
    u16 attributes[] = {MB_NORMAL, MB_JUMP_SOUTH};
    InitLedgeField(&layout, &tileset, attributes);
    MapGridSetMetatileIdAt(12, 11, 1 | MAPGRID_IMPASSABLE);
    for (u32 state = ACRO_STATE_NORMAL; state <= ACRO_STATE_SLOPE; state++)
    {
        gPlayerAvatar.acroBikeState = state;
        EXPECT_EQ(CanReverseLedgeHop(12, 11, DIR_NORTH), state == ACRO_STATE_BUNNY_HOP);
    }
    gPlayerAvatar.acroBikeState = ACRO_STATE_BUNNY_HOP;
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_MACH_BIKE;
    EXPECT(!CanReverseLedgeHop(12, 11, DIR_NORTH));
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_ON_FOOT;
    EXPECT(!CanReverseLedgeHop(12, 11, DIR_NORTH));
    EXPECT_EQ(GetLedgeJumpDirection(12, 11, DIR_SOUTH), DIR_SOUTH);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_ACRO_BIKE;
    gMapHeader.allowCycling = FALSE;
    EXPECT(!CanReverseLedgeHop(12, 11, DIR_NORTH));
    gMapHeader = savedMap;
}

TEST("Reverse ledges reject blocked water occupied and out of map landings across elevations")
{
    struct MapHeader savedMap = gMapHeader;
    struct MapLayout layout;
    struct Tileset tileset;
    u16 attributes[] = {MB_NORMAL, MB_JUMP_SOUTH};
    InitLedgeField(&layout, &tileset, attributes);
    MapGridSetMetatileIdAt(12, 11, 1 | MAPGRID_IMPASSABLE);
    MapGridSetMetatileIdAt(12, 10, MAPGRID_IMPASSABLE);
    EXPECT(!CanReverseLedgeHop(12, 11, DIR_NORTH));
    MapGridSetMetatileIdAt(12, 10, 0);
    attributes[0] = MB_POND_WATER;
    EXPECT(!CanReverseLedgeHop(12, 11, DIR_NORTH));
    attributes[0] = MB_NORMAL;
    gBackupMapLayout.map[10 * gBackupMapLayout.width + 12] = 4 << MAPGRID_ELEVATION_SHIFT;
    EXPECT(CanReverseLedgeHop(12, 11, DIR_NORTH));
    gObjectEvents[1].active = TRUE;
    gObjectEvents[1].currentCoords.x = 12;
    gObjectEvents[1].currentCoords.y = 10;
    gObjectEvents[1].currentElevation = 4;
    EXPECT(!CanReverseLedgeHop(12, 11, DIR_NORTH));
    gObjectEvents[1].previousCoords = gObjectEvents[1].currentCoords;
    gObjectEvents[1].currentCoords.y = 9;
    EXPECT(!CanReverseLedgeHop(12, 11, DIR_NORTH));
    gBackupMapLayout.map[10 * gBackupMapLayout.width + 12] = ELEVATION_MULTI_LEVEL << MAPGRID_ELEVATION_SHIFT;
    gObjectEvents[1].currentElevation = ELEVATION_DEFAULT;
    EXPECT(!CanReverseLedgeHop(12, 11, DIR_NORTH));
    gObjectEvents[1].active = FALSE;
    MapGridSetMetatileIdAt(12, MAP_OFFSET, 1 | MAPGRID_IMPASSABLE);
    EXPECT(!CanReverseLedgeHop(12, MAP_OFFSET, DIR_NORTH));
    gMapHeader = savedMap;
}

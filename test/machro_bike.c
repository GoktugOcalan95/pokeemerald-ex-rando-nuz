#include "global.h"
#include "machro_bike.h"
#include "bike.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "overworld.h"
#include "registered_items.h"
#include "save.h"
#include "script.h"
#include "sprite.h"
#include "task.h"
#include "test/test.h"
#include "constants/event_objects.h"
#include "constants/metatile_labels.h"
#include "constants/metatile_behaviors.h"

static void InitMachroField(void)
{
    ClearBag();
    ResetTasks();
    ResetSpriteData();
    ScriptContext_Init();
    ClearPlayerAvatarInfo();
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_ROUTE104), MAP_NUM(MAP_ROUTE104));
    InitMap();
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_MACH_BIKE;
    gPlayerAvatar.objectEventId = 0;
    gObjectEvents[0].active = TRUE;
    gObjectEvents[0].isPlayer = TRUE;
    gObjectEvents[0].localId = LOCALID_PLAYER;
    gObjectEvents[0].currentCoords.x = 10;
    gObjectEvents[0].currentCoords.y = 10;
    gObjectEvents[0].previousCoords = gObjectEvents[0].currentCoords;
    gObjectEvents[0].facingDirection = DIR_SOUTH;
    gObjectEvents[0].movementDirection = DIR_SOUTH;
    gObjectEvents[0].currentElevation = ELEVATION_DEFAULT;
    gObjectEvents[0].previousElevation = ELEVATION_DEFAULT;
    gBackupMapLayout.map[10 * gBackupMapLayout.width + 10] = METATILE_General_Grass | (ELEVATION_DEFAULT << MAPGRID_ELEVATION_SHIFT);
    FlagClear(FLAG_SYS_CYCLING_ROAD);
    FlagClear(FLAG_MACHRO_ACRO_MODE);
    VarSet(VAR_MOUNTED_BIKE, ITEM_BICYCLE);
    AddBagItem(ITEM_BICYCLE, 1);
}

TEST("Machro switches only with a new B press while L is held and preserves its mode in the save")
{
    struct MapHeader savedMap = gMapHeader;
    InitMachroField();
    EXPECT_EQ(GetBikeItemMode(ITEM_BICYCLE), PLAYER_AVATAR_FLAG_MACH_BIKE);
    EXPECT(CanSwitchMachroBike());
    EXPECT(!TrySwitchMachroBike(B_BUTTON, B_BUTTON));
    EXPECT(!TrySwitchMachroBike(0, L_BUTTON | B_BUTTON));
    EXPECT(TrySwitchMachroBike(B_BUTTON, L_BUTTON | B_BUTTON));
    EXPECT(TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_ACRO_BIKE));
    EXPECT_EQ(VarGet(VAR_MOUNTED_BIKE), ITEM_BICYCLE);
    EXPECT(FlagGet(FLAG_MACHRO_ACRO_MODE));
    EXPECT(!TrySwitchMachroBike(0, L_BUTTON | B_BUTTON));
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    FlagClear(FLAG_MACHRO_ACRO_MODE);
    VarSet(VAR_MOUNTED_BIKE, ITEM_NONE);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(GetBikeItemMode(VarGet(VAR_MOUNTED_BIKE)), PLAYER_AVATAR_FLAG_ACRO_BIKE);
    ClearPlayerAvatarInfo();
    SetPlayerAvatarExtraStateTransition(gObjectEvents[0].graphicsId, PLAYER_AVATAR_FLAG_CONTROLLABLE);
    EXPECT(TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_ACRO_BIKE));
    EXPECT_EQ(VarGet(VAR_MOUNTED_BIKE), ITEM_BICYCLE);
    SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_ON_FOOT);
    EXPECT_EQ(VarGet(VAR_MOUNTED_BIKE), ITEM_NONE);
    EXPECT_EQ(GetBikeItemMode(ITEM_BICYCLE), PLAYER_AVATAR_FLAG_ACRO_BIKE);
    EXPECT_EQ(GetBikeItemMode(ITEM_MACH_BIKE), PLAYER_AVATAR_FLAG_MACH_BIKE);
    EXPECT_EQ(GetBikeItemMode(ITEM_ACRO_BIKE), PLAYER_AVATAR_FLAG_ACRO_BIKE);
    ClearBag();
    gMapHeader = savedMap;
}

TEST("Machro rejects movement tricks unsafe terrain and ordinary bikes without queuing a switch")
{
    struct MapHeader savedMap = gMapHeader;
    InitMachroField();
    for (u32 state = ACRO_STATE_TURNING; state <= ACRO_STATE_SLOPE; state++)
    {
        gPlayerAvatar.acroBikeState = state;
        EXPECT(!TrySwitchMachroBike(B_BUTTON, L_BUTTON | B_BUTTON));
        EXPECT(!FlagGet(FLAG_MACHRO_ACRO_MODE));
    }
    gPlayerAvatar.acroBikeState = ACRO_STATE_NORMAL;
    gPlayerAvatar.runningState = MOVING;
    EXPECT(!CanSwitchMachroBike());
    gPlayerAvatar.runningState = NOT_MOVING;
    gPlayerAvatar.tileTransitionState = T_TILE_TRANSITION;
    EXPECT(!CanSwitchMachroBike());
    gPlayerAvatar.tileTransitionState = T_NOT_MOVING;
    gPlayerAvatar.preventStep = TRUE;
    EXPECT(!CanSwitchMachroBike());
    gPlayerAvatar.preventStep = FALSE;
    gObjectEvents[0].heldMovementActive = TRUE;
    EXPECT(!CanSwitchMachroBike());
    gObjectEvents[0].heldMovementFinished = TRUE;
    EXPECT(CanSwitchMachroBike());
    gObjectEvents[0].heldMovementActive = FALSE;
    FlagSet(FLAG_SYS_CYCLING_ROAD);
    EXPECT(!CanSwitchMachroBike());
    FlagClear(FLAG_SYS_CYCLING_ROAD);
    gPlayerAvatar.flags |= PLAYER_AVATAR_FLAG_FORCED_MOVE;
    EXPECT(!CanSwitchMachroBike());
    gPlayerAvatar.flags &= ~PLAYER_AVATAR_FLAG_FORCED_MOVE;
    MapGridSetMetatileIdAt(10, 10, METATILE_General_Grass | MAPGRID_IMPASSABLE);
    EXPECT(!CanSwitchMachroBike());
    MapGridSetMetatileIdAt(10, 10, METATILE_General_Grass);
    VarSet(VAR_MOUNTED_BIKE, ITEM_MACH_BIKE);
    EXPECT(!CanSwitchMachroBike());
    VarSet(VAR_MOUNTED_BIKE, ITEM_ACRO_BIKE);
    EXPECT(!CanSwitchMachroBike());
    VarSet(VAR_MOUNTED_BIKE, ITEM_BICYCLE);
    EXPECT(CanSwitchMachroBike());
    EXPECT(!TrySwitchMachroBike(0, L_BUTTON | B_BUTTON));
    EXPECT(!FlagGet(FLAG_MACHRO_ACRO_MODE));
    ClearBag();
    gMapHeader = savedMap;
}

TEST("Machro and ordinary Rydel exchanges preserve registrations and handle stored bikes")
{
    static const u16 bikes[] = {ITEM_MACH_BIKE, ITEM_ACRO_BIKE, ITEM_BICYCLE};
    u32 first, second;
    PARAMETRIZE { first = 0; second = 1; }
    PARAMETRIZE { first = 0; second = 2; }
    PARAMETRIZE { first = 1; second = 0; }
    PARAMETRIZE { first = 1; second = 2; }
    PARAMETRIZE { first = 2; second = 0; }
    PARAMETRIZE { first = 2; second = 1; }
    ClearBag();
    FlagClear(FLAG_RECEIVED_BIKE);
    VarSet(VAR_MOUNTED_BIKE, ITEM_NONE);
    memset(gSaveBlock1Ptr->registeredItems, 0, sizeof(gSaveBlock1Ptr->registeredItems));
    EXPECT_EQ(GiveOrExchangeBike(ITEM_POTION), 0);
    EXPECT(!FlagGet(FLAG_RECEIVED_BIKE));
    EXPECT_EQ(GiveOrExchangeBike(bikes[first]), 1);
    EXPECT(FlagGet(FLAG_RECEIVED_BIKE));
    EXPECT(RegisterItem(3, bikes[first]));
    EXPECT_EQ(GiveOrExchangeBike(bikes[first]), 2);
    EXPECT_EQ(GiveOrExchangeBike(bikes[second]), 1);
    EXPECT(!CheckBagHasItem(bikes[first], 1));
    EXPECT_EQ(CountTotalItemQuantityInBag(bikes[second]), 1);
    EXPECT_EQ(GetRegisteredItemSlot(bikes[second]), 3);
    EXPECT(RemoveBagItem(bikes[second], 1));
    EXPECT_EQ(GiveOrExchangeBike(bikes[first]), 3);
    EXPECT(!CheckBagHasItem(bikes[first], 1));
    ClearBag();
    FlagClear(FLAG_RECEIVED_BIKE);
    VarSet(VAR_MOUNTED_BIKE, ITEM_NONE);
}

TEST("Machro rejects mode changes on slopes rails water and forced movement tiles")
{
    static const u16 behaviors[] = {
        MB_MUDDY_SLOPE, MB_BUMPY_SLOPE, MB_VERTICAL_RAIL, MB_HORIZONTAL_RAIL,
        MB_ISOLATED_VERTICAL_RAIL, MB_ISOLATED_HORIZONTAL_RAIL, MB_POND_WATER,
        MB_WALK_NORTH,
    };
    struct MapHeader savedMap = gMapHeader;
    struct MapLayout layout;
    struct Tileset tileset;
    u16 attributes[1];
    InitMachroField();
    layout = *gMapHeader.mapLayout;
    tileset = *layout.primaryTileset;
    tileset.metatileAttributes = attributes;
    layout.primaryTileset = &tileset;
    gMapHeader.mapLayout = &layout;
    MapGridSetMetatileIdAt(10, 10, 0);
    for (u32 i = 0; i < ARRAY_COUNT(behaviors); i++)
    {
        attributes[0] = behaviors[i];
        EXPECT(!CanSwitchMachroBike());
    }
    attributes[0] = MB_NORMAL;
    EXPECT(CanSwitchMachroBike());
    ClearBag();
    gMapHeader = savedMap;
}

TEST("Machro first receipt can be retried after a full Bag and exchanges work with no spare slot")
{
    ClearBag();
    FlagClear(FLAG_RECEIVED_BIKE);
    VarSet(VAR_MOUNTED_BIKE, ITEM_NONE);
    while (AddBagItem(ITEM_OLD_ROD, MAX_BAG_ITEM_CAPACITY))
        ;
    EXPECT_EQ(GiveOrExchangeBike(ITEM_BICYCLE), 0);
    EXPECT(!FlagGet(FLAG_RECEIVED_BIKE));
    EXPECT(RemoveBagItem(ITEM_OLD_ROD, MAX_BAG_ITEM_CAPACITY));
    EXPECT_EQ(GiveOrExchangeBike(ITEM_BICYCLE), 1);
    EXPECT_EQ(GiveOrExchangeBike(ITEM_ACRO_BIKE), 1);
    EXPECT(!CheckBagHasItem(ITEM_BICYCLE, 1));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_ACRO_BIKE), 1);
    ClearBag();
    FlagClear(FLAG_RECEIVED_BIKE);
}

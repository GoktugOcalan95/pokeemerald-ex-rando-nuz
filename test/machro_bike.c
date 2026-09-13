#include "global.h"
#include "machro_bike.h"
#include "bike.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "load_save.h"
#include "palette.h"
#include "overworld.h"
#include "registered_items.h"
#include "save.h"
#include "script.h"
#include "sprite.h"
#include "task.h"
#include "test/test.h"
#include "constants/event_objects.h"
#include "constants/rgb.h"
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

TEST("Machro frame colors follow both riders through switching remounting and save restoration")
{
    static const u16 graphics[GENDER_COUNT][2] = {
        [MALE] = {OBJ_EVENT_GFX_BRENDAN_MACHRO_MACH, OBJ_EVENT_GFX_BRENDAN_MACHRO_ACRO},
        [FEMALE] = {OBJ_EVENT_GFX_MAY_MACHRO_MACH, OBJ_EVENT_GFX_MAY_MACHRO_ACRO},
    };
    static const u16 palettes[GENDER_COUNT][2] = {
        [MALE] = {OBJ_EVENT_PAL_TAG_BRENDAN_MACHRO_MACH, OBJ_EVENT_PAL_TAG_BRENDAN_MACHRO_ACRO},
        [FEMALE] = {OBJ_EVENT_PAL_TAG_MAY_MACHRO_MACH, OBJ_EVENT_PAL_TAG_MAY_MACHRO_ACRO},
    };
    struct MapHeader savedMap = gMapHeader;
    enum Gender gender;
    u32 mode;
    PARAMETRIZE { gender = MALE; mode = 0; }
    PARAMETRIZE { gender = MALE; mode = 1; }
    PARAMETRIZE { gender = FEMALE; mode = 0; }
    PARAMETRIZE { gender = FEMALE; mode = 1; }

    InitMachroField();
    FreeAllSpritePalettes();
    gSprites[0].inUse = TRUE;
    gPlayerAvatar.gender = gender;
    SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_ON_FOOT);
    if (mode)
        FlagSet(FLAG_MACHRO_ACRO_MODE);
    UseBikeItem(ITEM_BICYCLE);
    for (u32 i = 0; i < 20; i++)
    {
        EXPECT_EQ(gObjectEvents[0].graphicsId, graphics[gender][mode]);
        EXPECT_EQ(GetPlayerAvatarGraphicsIdByCurrentState(), graphics[gender][mode]);
        EXPECT_EQ(GetPlayerAvatarGenderByGraphicsId(gObjectEvents[0].graphicsId), gender);
        EXPECT_EQ(GetSpritePaletteTagByPaletteNum(gSprites[0].oam.paletteNum), palettes[gender][mode]);
        EXPECT_EQ(gSprites[0].images, GetObjectEventGraphicsInfo(graphics[gender][mode])->images);
        EXPECT_EQ(gPlttBufferUnfaded[OBJ_PLTT_ID(gSprites[0].oam.paletteNum) + (gender == MALE ? 7 : 12)], mode ? RGB(31, 19, 5) : RGB(5, 20, 31));
        EXPECT(TrySwitchMachroBike(B_BUTTON, L_BUTTON | B_BUTTON));
        mode ^= 1;
    }

    SaveObjectEvents();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
    VarSet(VAR_MOUNTED_BIKE, ITEM_NONE);
    FlagClear(FLAG_MACHRO_ACRO_MODE);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    LoadObjectEvents();
    ClearPlayerAvatarInfo();
    gPlayerAvatar.gender = GetPlayerAvatarGenderByGraphicsId(gObjectEvents[0].graphicsId);
    SetPlayerAvatarExtraStateTransition(gObjectEvents[0].graphicsId, PLAYER_AVATAR_FLAG_CONTROLLABLE);
    EXPECT_EQ(gPlayerAvatar.gender, gender);
    EXPECT_EQ(gObjectEvents[0].graphicsId, graphics[gender][mode]);
    EXPECT_EQ(GetSpritePaletteTagByPaletteNum(gSprites[0].oam.paletteNum), palettes[gender][mode]);
    EXPECT(TestPlayerAvatarFlags(mode ? PLAYER_AVATAR_FLAG_ACRO_BIKE : PLAYER_AVATAR_FLAG_MACH_BIKE));

    UseBikeItem(ITEM_BICYCLE);
    EXPECT_EQ(gObjectEvents[0].graphicsId, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_NORMAL));
    UseBikeItem(ITEM_BICYCLE);
    EXPECT_EQ(gObjectEvents[0].graphicsId, graphics[gender][mode]);
    EXPECT_EQ(GetSpritePaletteTagByPaletteNum(gSprites[0].oam.paletteNum), palettes[gender][mode]);

    UseBikeItem(ITEM_BICYCLE);
    UseBikeItem(ITEM_MACH_BIKE);
    EXPECT_EQ(gObjectEvents[0].graphicsId, gender == MALE ? OBJ_EVENT_GFX_BRENDAN_MACH_BIKE : OBJ_EVENT_GFX_MAY_MACH_BIKE);
    EXPECT_EQ(GetSpritePaletteTagByPaletteNum(gSprites[0].oam.paletteNum), gender == MALE ? OBJ_EVENT_PAL_TAG_BRENDAN : OBJ_EVENT_PAL_TAG_MAY);
    UseBikeItem(ITEM_MACH_BIKE);
    UseBikeItem(ITEM_ACRO_BIKE);
    EXPECT_EQ(gObjectEvents[0].graphicsId, gender == MALE ? OBJ_EVENT_GFX_BRENDAN_ACRO_BIKE : OBJ_EVENT_GFX_MAY_ACRO_BIKE);
    EXPECT_EQ(GetSpritePaletteTagByPaletteNum(gSprites[0].oam.paletteNum), gender == MALE ? OBJ_EVENT_PAL_TAG_BRENDAN : OBJ_EVENT_PAL_TAG_MAY);
    UseBikeItem(ITEM_ACRO_BIKE);
    FreeAllSpritePalettes();
    ClearBag();
    gMapHeader = savedMap;
}

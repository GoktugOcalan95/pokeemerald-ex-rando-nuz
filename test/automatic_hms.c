#include "global.h"
#include "bike.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "field_effect.h"
#include "field_control_avatar.h"
#include "field_move.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "pokemon.h"
#include "save.h"
#include "script.h"
#include "sprite.h"
#include "task.h"
#include "test/test.h"
#include "constants/event_objects.h"
#include "constants/field_effects.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "constants/metatile_labels.h"

extern bool8 ScrCmd_checkfieldmove(struct ScriptContext *ctx);
extern bool32 Test_TryAutomaticHM(void);
extern bool32 Test_TryDive(bool32 emerge);
extern bool32 Test_TryUsePuzzleHM(void);
extern bool32 Test_StartMenuHasFly(u32 *count);

static void ResetHMProgress(void)
{
    ClearBag();
    for (u32 flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
        FlagClear(flag);
    FlagClear(FLAG_SYS_USE_FLASH);
    FlagClear(FLAG_SYS_USE_STRENGTH);
}

TEST("Automatic HMs require both the correct HM and badge, independently of party moves")
{
    static const struct {
        enum FieldMove move;
        u16 item;
        u16 badge;
    } cases[] = {
        {FIELD_MOVE_CUT, ITEM_HM01, FLAG_BADGE01_GET},
        {FIELD_MOVE_FLASH, ITEM_HM05, FLAG_BADGE02_GET},
        {FIELD_MOVE_ROCK_SMASH, ITEM_HM06, FLAG_BADGE03_GET},
        {FIELD_MOVE_STRENGTH, ITEM_HM04, FLAG_BADGE04_GET},
        {FIELD_MOVE_SURF, ITEM_HM03, FLAG_BADGE05_GET},
        {FIELD_MOVE_FLY, ITEM_HM02, FLAG_BADGE06_GET},
        {FIELD_MOVE_DIVE, ITEM_HM08, FLAG_BADGE07_GET},
        {FIELD_MOVE_WATERFALL, ITEM_HM07, FLAG_BADGE08_GET},
    };

    CreateRandomMon(&gParties[B_TRAINER_PLAYER][0], SPECIES_MAGIKARP, 5);
    for (u32 i = 0; i < ARRAY_COUNT(cases); i++)
    {
        ResetHMProgress();
        SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], FieldMove_GetMoveId(cases[i].move), 0);
        EXPECT(!IsFieldMoveUnlocked(cases[i].move));
        FlagSet(cases[i].badge);
        EXPECT(!IsFieldMoveUnlocked(cases[i].move));
        FlagClear(cases[i].badge);
        EXPECT(AddBagItem(cases[i].item, 1));
        EXPECT(!IsFieldMoveUnlocked(cases[i].move));
        FlagSet(cases[i].badge);
        SetMonMoveSlot(&gParties[B_TRAINER_PLAYER][0], MOVE_SPLASH, 0);
        EXPECT(IsFieldMoveUnlocked(cases[i].move));
        EXPECT(RemoveBagItem(cases[i].item, 1));
        EXPECT(!IsFieldMoveUnlocked(cases[i].move));
    }
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_DIG));
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_TELEPORT));
    ResetHMProgress();
}

TEST("Automatic HMs script checks enforce progression even without a party or the optional badge check")
{
    const u8 args[] = {FIELD_MOVE_DIVE, FALSE};
    struct ScriptContext ctx = {0};
    ResetHMProgress();
    memset(gParties[B_TRAINER_PLAYER], 0, sizeof(gParties[B_TRAINER_PLAYER]));
    EXPECT(AddBagItem(ITEM_HM08, 1));
    ctx.scriptPtr = args;
    ScrCmd_checkfieldmove(&ctx);
    EXPECT_EQ(gSpecialVar_Result, PARTY_SIZE);
    FlagSet(FLAG_BADGE07_GET);
    ctx.scriptPtr = args;
    ScrCmd_checkfieldmove(&ctx);
    EXPECT_EQ(gSpecialVar_Result, 0);
    EXPECT_EQ(gSpecialVar_0x8004, SPECIES_NONE);
    ResetHMProgress();
}

TEST("Automatic HMs inventory and badge gates survive saving and loading")
{
    ResetHMProgress();
    EXPECT(AddBagItem(ITEM_HM02, 1));
    EXPECT(AddBagItem(ITEM_HM05, 1));
    EXPECT(AddBagItem(ITEM_HM08, 1));
    FlagSet(FLAG_BADGE02_GET);
    FlagSet(FLAG_BADGE06_GET);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    ResetHMProgress();
    FlagSet(FLAG_BADGE07_GET);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_FLY));
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_FLASH));
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_DIVE));
    ResetHMProgress();
}

TEST("Automatic HMs Flash lights cave floors and preserves gym and Pyramid darkness rules")
{
    struct MapHeader savedMap = gMapHeader;
    ResetHMProgress();
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_GRANITE_CAVE_B1F), MAP_NUM(MAP_GRANITE_CAVE_B1F));
    SetDefaultFlashLevel();
    EXPECT_GT(GetFlashLevel(), 1);
    EXPECT(AddBagItem(ITEM_HM05, 1));
    EXPECT(!FieldMove_CanUseAutomaticFlash());
    FlagSet(FLAG_BADGE02_GET);
    EXPECT(FieldMove_CanUseAutomaticFlash());
    SetDefaultFlashLevel();
    EXPECT_EQ(GetFlashLevel(), 1);
    EXPECT(FlagGet(FLAG_SYS_USE_FLASH));
    FlagClear(FLAG_SYS_USE_FLASH);
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_GRANITE_CAVE_B2F), MAP_NUM(MAP_GRANITE_CAVE_B2F));
    SetDefaultFlashLevel();
    EXPECT_EQ(GetFlashLevel(), 1);
    gMapHeader.mapLayoutId = LAYOUT_DEWFORD_TOWN_GYM;
    EXPECT(!FieldMove_CanUseAutomaticFlash());
    gMapHeader.mapLayoutId = LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR;
    EXPECT(!FieldMove_CanUseAutomaticFlash());
    gMapHeader.cave = FALSE;
    SetDefaultFlashLevel();
    EXPECT_EQ(GetFlashLevel(), 0);
    gMapHeader = savedMap;
    ResetHMProgress();
}

static void InitHMField(void)
{
    ResetHMProgress();
    ResetTasks();
    FieldEffectActiveListClear();
    ResetSpriteData();
    ScriptContext_Init();
    ClearPlayerAvatarInfo();
    memset(gObjectEvents, 0, sizeof(gObjectEvents));
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_ROUTE104), MAP_NUM(MAP_ROUTE104));
    InitMap();
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_ON_FOOT;
    gPlayerAvatar.objectEventId = 0;
    gObjectEvents[0].active = TRUE;
    gObjectEvents[0].isPlayer = TRUE;
    gObjectEvents[0].localId = LOCALID_PLAYER;
    gObjectEvents[0].currentElevation = ELEVATION_DEFAULT;
    gObjectEvents[0].previousElevation = ELEVATION_DEFAULT;
    gObjectEvents[0].currentCoords.x = 10;
    gObjectEvents[0].currentCoords.y = 10;
    gObjectEvents[0].previousCoords = gObjectEvents[0].currentCoords;
    gObjectEvents[0].facingDirection = DIR_EAST;
    gObjectEvents[0].movementDirection = DIR_EAST;
    for (u32 x = 10; x <= 12; x++)
        gBackupMapLayout.map[10 * gBackupMapLayout.width + x] = METATILE_General_Grass | (ELEVATION_DEFAULT << MAPGRID_ELEVATION_SHIFT);
}

TEST("Automatic HMs Strength respects blocked destinations, badge gates and Acro tricks")
{
    struct ObjectEvent *boulder = &gObjectEvents[1];
    InitHMField();
    boulder->active = TRUE;
    boulder->graphicsId = OBJ_EVENT_GFX_PUSHABLE_BOULDER;
    boulder->currentCoords.x = 11;
    boulder->currentCoords.y = 10;
    boulder->previousCoords = boulder->currentCoords;
    boulder->currentElevation = ELEVATION_DEFAULT;
    boulder->previousElevation = ELEVATION_DEFAULT;
    EXPECT(AddBagItem(ITEM_HM04, 1));
    EXPECT_EQ(CheckForObjectEventCollision(&gObjectEvents[0], 11, 10, DIR_EAST, 0), COLLISION_OBJECT_EVENT);
    FlagSet(FLAG_BADGE04_GET);
    MapGridSetMetatileIdAt(12, 10, METATILE_General_Grass | MAPGRID_IMPASSABLE);
    EXPECT_EQ(CheckForObjectEventCollision(&gObjectEvents[0], 11, 10, DIR_EAST, 0), COLLISION_OBJECT_EVENT);
    EXPECT(!gPlayerAvatar.preventStep);
    MapGridSetMetatileIdAt(12, 10, METATILE_General_Grass);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_ACRO_BIKE;
    gPlayerAvatar.acroBikeState = ACRO_STATE_SIDE_JUMP;
    EXPECT_EQ(CheckForObjectEventCollision(&gObjectEvents[0], 11, 10, DIR_EAST, 0), COLLISION_OBJECT_EVENT);
    gPlayerAvatar.acroBikeState = ACRO_STATE_NORMAL;
    EXPECT_EQ(CheckForObjectEventCollision(&gObjectEvents[0], 11, 10, DIR_EAST, 0), COLLISION_PUSHED_BOULDER);
    EXPECT(gPlayerAvatar.preventStep);
    EXPECT(!FlagGet(FLAG_SYS_USE_STRENGTH));
    ResetTasks();
    UnlockPlayerFieldControls();
    ClearPlayerAvatarInfo();
    ResetHMProgress();
}

TEST("Automatic HMs Cut starts from forward bike input and selects the actual obstacle")
{
    InitHMField();
    const struct ObjectEventTemplate *tree = NULL;
    for (u32 i = 0; i < gMapHeader.events->objectEventCount; i++)
    {
        if (gMapHeader.events->objectEvents[i].script == EventScript_CutTree)
        {
            tree = &gMapHeader.events->objectEvents[i];
            break;
        }
    }
    ASSUME(tree != NULL);
    gObjectEvents[1].active = TRUE;
    gObjectEvents[1].localId = tree->localId;
    gObjectEvents[1].mapGroup = MAP_GROUP(MAP_ROUTE104);
    gObjectEvents[1].mapNum = MAP_NUM(MAP_ROUTE104);
    gObjectEvents[1].graphicsId = tree->graphicsId;
    gObjectEvents[1].currentElevation = ELEVATION_DEFAULT;
    gObjectEvents[1].previousElevation = ELEVATION_DEFAULT;
    gObjectEvents[1].currentCoords.x = 11;
    gObjectEvents[1].currentCoords.y = 10;
    gObjectEvents[1].previousCoords = gObjectEvents[1].currentCoords;
    EXPECT(!Test_TryAutomaticHM());
    EXPECT(AddBagItem(ITEM_HM01, 1));
    FlagSet(FLAG_BADGE01_GET);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_ACRO_BIKE;
    gPlayerAvatar.acroBikeState = ACRO_STATE_SIDE_JUMP;
    EXPECT(!Test_TryAutomaticHM());
    gPlayerAvatar.acroBikeState = ACRO_STATE_NORMAL;
    EXPECT(Test_TryAutomaticHM());
    EXPECT_EQ(gSpecialVar_LastTalked, tree->localId);
    EXPECT_EQ(gSelectedObjectEvent, 1);
    EXPECT(ScriptContext_IsEnabled());
    ScriptContext_Init();
    UnlockPlayerFieldControls();
    ResetHMProgress();
}

TEST("Automatic HMs Fly replaces Exit without increasing the Start menu size")
{
    u32 before, after;
    ResetHMProgress();
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKENAV_GET);
    EXPECT(!Test_StartMenuHasFly(&before));
    EXPECT_EQ(before, 8);
    EXPECT(AddBagItem(ITEM_HM02, 1));
    EXPECT(!Test_StartMenuHasFly(&after));
    FlagSet(FLAG_BADGE06_GET);
    EXPECT(Test_StartMenuHasFly(&after));
    EXPECT_EQ(before, after);
    EXPECT(RemoveBagItem(ITEM_HM02, 1));
    EXPECT(!Test_StartMenuHasFly(&after));
    EXPECT_EQ(before, after);
    ResetHMProgress();
}

TEST("Automatic HMs Surf starts from shore without teaching and refuses occupied water")
{
    InitHMField();
    gBackupMapLayout.map[10 * gBackupMapLayout.width + 11] = METATILE_General_RoughWater | (1 << MAPGRID_ELEVATION_SHIFT);
    EXPECT(IsPlayerFacingSurfableFishableWater());
    EXPECT(!Test_TryAutomaticHM());
    EXPECT(AddBagItem(ITEM_HM03, 1));
    EXPECT(!Test_TryAutomaticHM());
    FlagSet(FLAG_BADGE05_GET);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_MACH_BIKE;
    EXPECT(Test_TryAutomaticHM());
    EXPECT_EQ(gPlayerAvatar.bikeSpeed, 0);
    ScriptContext_Init();
    gObjectEvents[1].active = TRUE;
    gObjectEvents[1].currentElevation = 1;
    gObjectEvents[1].previousElevation = 1;
    gObjectEvents[1].currentCoords.x = 11;
    gObjectEvents[1].currentCoords.y = 10;
    gObjectEvents[1].previousCoords = gObjectEvents[1].currentCoords;
    EXPECT(!Test_TryAutomaticHM());
    ResetHMProgress();
}

TEST("Automatic HMs Waterfall requires surfing north and badge eight")
{
    bool32 found = FALSE;
    InitHMField();
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_EVER_GRANDE_CITY), MAP_NUM(MAP_EVER_GRANDE_CITY));
    InitMap();
    for (u32 y = MAP_OFFSET; y < gBackupMapLayout.height - MAP_OFFSET; y++)
    {
        for (u32 x = MAP_OFFSET; x < gBackupMapLayout.width - MAP_OFFSET; x++)
        {
            if (MetatileBehavior_IsWaterfall(MapGridGetMetatileBehaviorAt(x, y)))
            {
                gObjectEvents[0].currentCoords.x = x;
                gObjectEvents[0].currentCoords.y = y + 1;
                found = TRUE;
                break;
            }
        }
        if (found)
            break;
    }
    ASSUME(found);
    gObjectEvents[0].previousCoords = gObjectEvents[0].currentCoords;
    gObjectEvents[0].facingDirection = DIR_NORTH;
    gObjectEvents[0].movementDirection = DIR_NORTH;
    EXPECT(AddBagItem(ITEM_HM07, 1));
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_SURFING;
    EXPECT(!Test_TryAutomaticHM());
    FlagSet(FLAG_BADGE08_GET);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_ON_FOOT;
    EXPECT(!Test_TryAutomaticHM());
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_SURFING;
    EXPECT(Test_TryAutomaticHM());
    ScriptContext_Init();
    gObjectEvents[0].movementDirection = DIR_SOUTH;
    EXPECT(!Test_TryAutomaticHM());
    ResetHMProgress();
}

TEST("Automatic HMs Dive and surfacing keep deliberate actions gated by badge seven")
{
    InitHMField();
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_ROUTE124), MAP_NUM(MAP_ROUTE124));
    InitMap();
    gBackupMapLayout.map[10 * gBackupMapLayout.width + 10] = METATILE_General_RoughDeepWater | (1 << MAPGRID_ELEVATION_SHIFT);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_SURFING;
    EXPECT_EQ(TrySetDiveWarp(), 2);
    EXPECT(AddBagItem(ITEM_HM08, 1));
    EXPECT(!Test_TryDive(FALSE));
    FlagSet(FLAG_BADGE07_GET);
    EXPECT(Test_TryDive(FALSE));
    ScriptContext_Init();
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_UNDERWATER_ROUTE124), MAP_NUM(MAP_UNDERWATER_ROUTE124));
    InitMap();
    EXPECT_EQ(TrySetDiveWarp(), 1);
    FlagClear(FLAG_BADGE07_GET);
    EXPECT(!Test_TryDive(TRUE));
    FlagSet(FLAG_BADGE07_GET);
    EXPECT(Test_TryDive(TRUE));
    ScriptContext_Init();
    ResetHMProgress();
}

TEST("Automatic HMs puzzle actions require the correct position and HM progression")
{
    InitHMField();
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_ANCIENT_TOMB);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_ANCIENT_TOMB);
    gSaveBlock1Ptr->pos.x = 8;
    gSaveBlock1Ptr->pos.y = 25;
    FlagClear(FLAG_SYS_REGISTEEL_PUZZLE_COMPLETED);
    EXPECT(!Test_TryUsePuzzleHM());
    EXPECT(AddBagItem(ITEM_HM05, 1));
    FlagSet(FLAG_BADGE02_GET);
    gSaveBlock1Ptr->pos.x = 7;
    EXPECT(!Test_TryUsePuzzleHM());
    gSaveBlock1Ptr->pos.x = 8;
    EXPECT(Test_TryUsePuzzleHM());
    EXPECT_EQ(gFieldEffectArguments[0], PARTY_SIZE);
    EXPECT(FieldEffectActiveListContains(FLDEFF_USE_TOMB_PUZZLE_EFFECT));
    ResetTasks();
    FieldEffectActiveListRemove(FLDEFF_USE_TOMB_PUZZLE_EFFECT);
    ResetHMProgress();
}

TEST("Automatic HMs field effects skip Pokemon presentation without reading a party slot")
{
    ResetTasks();
    FieldEffectActiveListClear();
    memset(gParties[B_TRAINER_PLAYER], 0, sizeof(gParties[B_TRAINER_PLAYER]));
    gFieldEffectArguments[0] = PARTY_SIZE;
    FieldEffectStart(FLDEFF_FIELD_MOVE_SHOW_MON_INIT);
    EXPECT(!FieldEffectActiveListContains(FLDEFF_FIELD_MOVE_SHOW_MON_INIT));
    EXPECT(!FieldEffectActiveListContains(FLDEFF_FIELD_MOVE_SHOW_MON));
    EXPECT_EQ(gFieldEffectArguments[0], PARTY_SIZE);
}

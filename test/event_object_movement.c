#include "global.h"
#include "event_object_movement.h"
#include "sprite.h"
#include "fieldmap.h"
#include "overworld.h"
#include "script.h"
#include "script_movement.h"
#include "field_player_avatar.h"
#include "task.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/maps.h"
#include "test/test.h"

static const u32 sFrame16x32[256 / sizeof(u32)] = {0};
static const u32 sFrame32x32[512 / sizeof(u32)] = {0};

static const struct OamData sOam16x32 = {
    .shape = SPRITE_SHAPE(16x32),
    .size = SPRITE_SIZE(16x32),
};

static const struct OamData sOam32x32 = {
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
};

static const struct SpriteFrameImage sImages16x32[] = {
    {.data = sFrame16x32, .size = sizeof(sFrame16x32)},
};

static const struct SpriteFrameImage sImages32x32[] = {
    {.data = sFrame32x32, .size = sizeof(sFrame32x32)},
};

static const struct SpriteTemplate sSpriteTemplate16x32 = {
    .tileTag = TAG_NONE,
    .paletteTag = TAG_NONE,
    .oam = &sOam16x32,
    .images = sImages16x32,
    .callback = SpriteCallbackDummy,
};

static const struct ObjectEventGraphicsInfo sGraphicsInfo32x32 = {
    .tileTag = TAG_NONE,
    .size = sizeof(sFrame32x32),
    .oam = &sOam32x32,
    .images = sImages32x32,
};

extern u16 LoadSheetGraphicsInfo(const struct ObjectEventGraphicsInfo *info, u16 uuid, struct Sprite *sprite);

TEST("LoadSheetGraphicsInfo reallocates non-sheet sprites when frame size changes")
{
    u16 i;
    u16 tileNum;
    u16 tileCount;
    u8 spriteId;
    struct Sprite *sprite;

    ASSUME(sOam16x32.size == sOam32x32.size);
    ASSUME(sOam16x32.shape != sOam32x32.shape);

    ResetSpriteData();
    spriteId = CreateSprite(&sSpriteTemplate16x32, 0, 0, 0);
    ASSUME(spriteId != MAX_SPRITES);

    sprite = &gSprites[spriteId];
    tileNum = sprite->oam.tileNum;
    tileCount = sGraphicsInfo32x32.images->size / TILE_SIZE_4BPP;

    EXPECT_EQ(sprite->images->size, sizeof(sFrame16x32));
    EXPECT_EQ(sGraphicsInfo32x32.images->size, sizeof(sFrame32x32));

    LoadSheetGraphicsInfo(&sGraphicsInfo32x32, 0, sprite);

    for (i = 0; i < tileCount; i++)
        EXPECT(SpriteTileAllocBitmapOp(tileNum + i, 2) != 0);

    DestroySprite(sprite);
}

static bool8 CutsceneMovementCallback(struct ObjectEvent *objectEvent, struct Sprite *sprite)
{
    return FALSE;
}

static u32 MeasureCutsceneMovement(u32 action, u32 speed, bool32 locked, bool32 isPlayer, u32 localId, s16 *x, s16 *y)
{
    struct ObjectEvent *objectEvent = &gObjectEvents[0];
    struct Sprite *sprite = &gSprites[0];
    u32 frames;

    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F), MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F));
    InitMap();
    ResetSpriteData();
    memset(objectEvent, 0, sizeof(*objectEvent));
    objectEvent->active = TRUE;
    objectEvent->inanimate = TRUE;
    objectEvent->localId = localId;
    objectEvent->isPlayer = isPlayer;
    objectEvent->graphicsId = OBJ_EVENT_GFX_BOY_1;
    objectEvent->currentCoords.x = 10;
    objectEvent->currentCoords.y = 10;
    objectEvent->previousCoords = objectEvent->currentCoords;
    sprite->inUse = TRUE;
    sprite->x = 0;
    sprite->y = 0;
    sprite->anims = gDummySpriteAnimTable;
    gSaveBlock2Ptr->optionsTextSpeed = speed;
    if (locked)
        LockPlayerFieldControls();
    else
        UnlockPlayerFieldControls();
    EXPECT_EQ(ObjectEventSetHeldMovement(objectEvent, action), FALSE);
    for (frames = 0; frames < 128 && !ObjectEventCheckHeldMovementStatus(objectEvent); frames++)
        UpdateObjectEventCurrentMovement(objectEvent, sprite, CutsceneMovementCallback);
    EXPECT_LT(frames, 128);
    *x = sprite->x;
    *y = sprite->y;
    EXPECT_EQ(ObjectEventClearHeldMovementIfFinished(objectEvent), TRUE);
    EXPECT(!ObjectEventIsHeldMovementActive(objectEvent));
    UnlockPlayerFieldControls();
    return frames;
}

TEST("Cutscene movement doubles travel speed without changing endpoints")
{
    static const u8 actions[] = {
        MOVEMENT_ACTION_WALK_SLOW_DOWN, MOVEMENT_ACTION_WALK_SLOW_UP,
        MOVEMENT_ACTION_WALK_NORMAL_LEFT, MOVEMENT_ACTION_WALK_NORMAL_RIGHT,
        MOVEMENT_ACTION_WALK_FAST_DOWN, MOVEMENT_ACTION_WALK_FASTER_UP,
        MOVEMENT_ACTION_PLAYER_RUN_LEFT,
        MOVEMENT_ACTION_WALK_NORMAL_DIAGONAL_UP_LEFT,
        MOVEMENT_ACTION_WALK_SLOW_DIAGONAL_DOWN_RIGHT,
        MOVEMENT_ACTION_WALK_FAST_DIAGONAL_UP_RIGHT,
        MOVEMENT_ACTION_WALK_SLOW_STAIRS_DOWN,
    };
    u32 speed;
    bool32 isPlayer;
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    s16 normalX, normalY, fastX, fastY;

    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_INSTANT; isPlayer = FALSE; }
    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_AUTO; isPlayer = FALSE; }
    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_INSTANT; isPlayer = TRUE; }
    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_AUTO; isPlayer = TRUE; }

    for (u32 i = 0; i < ARRAY_COUNT(actions); i++)
    {
        u32 normal = MeasureCutsceneMovement(actions[i], OPTIONS_TEXT_SPEED_MID, TRUE, isPlayer, 1, &normalX, &normalY);
        u32 fast = MeasureCutsceneMovement(actions[i], speed, TRUE, isPlayer, 1, &fastX, &fastY);
        EXPECT_EQ(fast, (normal + 1) / 2);
        EXPECT_EQ(fastX, normalX);
        EXPECT_EQ(fastY, normalY);
    }
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Cutscene movement preserves other text speeds and free player camera follower movement")
{
    u32 speed = OPTIONS_TEXT_SPEED_AUTO;
    u32 localId = 1;
    bool32 locked = TRUE;
    bool32 isPlayer = FALSE;
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    s16 x, y;

    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_MID; }
    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_FAST; }
    PARAMETRIZE { locked = FALSE; }
    PARAMETRIZE { isPlayer = TRUE; locked = FALSE; }
    PARAMETRIZE { isPlayer = TRUE; speed = OPTIONS_TEXT_SPEED_MID; }
    PARAMETRIZE { isPlayer = TRUE; speed = OPTIONS_TEXT_SPEED_FAST; }
    PARAMETRIZE { localId = LOCALID_CAMERA; }
    PARAMETRIZE { localId = OBJ_EVENT_ID_FOLLOWER; }
    PARAMETRIZE { localId = OBJ_EVENT_ID_NPC_FOLLOWER; }

    EXPECT_EQ(MeasureCutsceneMovement(MOVEMENT_ACTION_WALK_NORMAL_RIGHT, speed, locked, isPlayer, localId, &x, &y), 16);
    EXPECT_EQ(x, 16);
    EXPECT_EQ(y, 0);
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Cutscene movement preserves delays jumps and slides")
{
    u32 action;
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    s16 normalX, normalY, fastX, fastY;

    PARAMETRIZE { action = MOVEMENT_ACTION_DELAY_16; }
    PARAMETRIZE { action = MOVEMENT_ACTION_JUMP_RIGHT; }
    PARAMETRIZE { action = MOVEMENT_ACTION_SLIDE_RIGHT; }
    PARAMETRIZE { action = MOVEMENT_ACTION_RIDE_WATER_CURRENT_RIGHT; }
    PARAMETRIZE { action = MOVEMENT_ACTION_WALK_IN_PLACE_NORMAL_DOWN; }

    u32 normal = MeasureCutsceneMovement(action, OPTIONS_TEXT_SPEED_MID, TRUE, FALSE, 1, &normalX, &normalY);
    EXPECT_EQ(MeasureCutsceneMovement(action, OPTIONS_TEXT_SPEED_AUTO, TRUE, FALSE, 1, &fastX, &fastY), normal);
    EXPECT_EQ(fastX, normalX);
    EXPECT_EQ(fastY, normalY);
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Cutscene movement recalls blocking followers at double speed")
{
    static const u8 movement[] = {MOVEMENT_ACTION_WALK_NORMAL_RIGHT, MOVEMENT_ACTION_STEP_END};
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    u32 savedPlayerId = gPlayerAvatar.objectEventId;
    s16 x, y;

    MeasureCutsceneMovement(MOVEMENT_ACTION_FACE_RIGHT, OPTIONS_TEXT_SPEED_AUTO, TRUE, FALSE, 1, &x, &y);
    ResetTasks();
    gPlayerAvatar.objectEventId = 2;
    gObjectEvents[1] = (struct ObjectEvent){
        .active = TRUE,
        .localId = OBJ_EVENT_ID_FOLLOWER,
        .spriteId = 1,
        .movementType = MOVEMENT_TYPE_FOLLOW_PLAYER,
        .currentCoords = {11, 10},
        .previousCoords = {11, 10},
    };
    gSprites[1].data[1] = 1;
    LockPlayerFieldControls();
    EXPECT_EQ(ScriptMovement_StartObjectMovementScript(1, 0, 0, movement), FALSE);
    RunTasks();
    UpdateObjectEventCurrentMovement(&gObjectEvents[0], &gSprites[0], CutsceneMovementCallback);
    EXPECT_EQ(gSprites[0].data[5], 2);
    RunTasks();
    EXPECT_EQ(gSprites[1].data[1], 0);
    EXPECT(gObjectEvents[1].heldMovementActive);
    EXPECT_EQ(gObjectEvents[1].movementActionId, MOVEMENT_ACTION_ENTER_POKEBALL);
    ScriptMovement_UnfreezeObjectEvents();
    memset(&gObjectEvents[1], 0, sizeof(gObjectEvents[1]));
    UnlockPlayerFieldControls();
    gPlayerAvatar.objectEventId = savedPlayerId;
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Cutscene movement scripts finish all steps before releasing their wait")
{
    static const u8 movement[] = {
        MOVEMENT_ACTION_WALK_NORMAL_RIGHT, MOVEMENT_ACTION_DELAY_16,
        MOVEMENT_ACTION_WALK_NORMAL_LEFT, MOVEMENT_ACTION_STEP_END,
    };
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    u32 frames;
    bool32 isPlayer;
    s16 x, y;

    PARAMETRIZE { isPlayer = FALSE; }
    PARAMETRIZE { isPlayer = TRUE; }

    MeasureCutsceneMovement(MOVEMENT_ACTION_FACE_RIGHT, OPTIONS_TEXT_SPEED_AUTO, TRUE, isPlayer, 1, &x, &y);
    ResetTasks();
    LockPlayerFieldControls();
    EXPECT_EQ(ScriptMovement_StartObjectMovementScript(1, 0, 0, movement), FALSE);
    EXPECT(!ScriptMovement_IsAllObjectMovementFinished());
    for (frames = 0; frames < 64 && !ScriptMovement_IsAllObjectMovementFinished(); frames++)
    {
        RunTasks();
        UpdateObjectEventCurrentMovement(&gObjectEvents[0], &gSprites[0], CutsceneMovementCallback);
    }
    EXPECT_GE(frames, 32);
    EXPECT_LT(frames, 40);
    EXPECT(ScriptMovement_IsObjectMovementFinished(1, 0, 0));
    EXPECT_EQ(gObjectEvents[0].currentCoords.x, 10);
    EXPECT_EQ(gSprites[0].x, 0);
    EXPECT(!gObjectEvents[0].heldMovementActive);
    ScriptMovement_UnfreezeObjectEvents();
    UnlockPlayerFieldControls();
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

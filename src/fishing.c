#include "global.h"
#include "main.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "field_effect_helpers.h"
#include "field_player_avatar.h"
#include "menu.h"
#include "metatile_behavior.h"
#include "random.h"
#include "script.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "tv.h"
#include "wild_encounter.h"
#include "config/fishing.h"

static void Task_Fishing(u8);
static bool32 Fishing_Init(struct Task *);
static bool32 Fishing_GetRodOut(struct Task *);
static bool32 Fishing_WaitBeforeDots(struct Task *);
static bool32 Fishing_InitDots(struct Task *);
static bool32 Fishing_ShowDots(struct Task *);
static bool32 Fishing_CheckForBite(struct Task *);
static bool32 Fishing_StartEncounter(struct Task *);
static bool32 Fishing_NotEvenNibble(struct Task *);
static bool32 Fishing_NoMon(struct Task *);
static bool32 Fishing_PutRodAway(struct Task *);
static bool32 Fishing_EndNoMon(struct Task *);
static void AlignFishingAnimationFrames(void);
static bool32 DoesFishingMinigameAllowCancel(void);

static const u8 sText_NotEvenANibble[] = _("Not even a nibble…{PAUSE_UNTIL_PRESS}");

#define FISHING_CHAIN_SHINY_STREAK_MAX 20

enum
{
    FISHING_INIT,
    FISHING_GET_ROD_OUT,
    FISHING_WAIT_BEFORE_DOTS,
    FISHING_INIT_DOTS,
    FISHING_SHOW_DOTS,
    FISHING_CHECK_FOR_BITE,
    FISHING_START_ENCOUNTER,
    FISHING_NOT_EVEN_NIBBLE,
    FISHING_NO_MON,
    FISHING_PUT_ROD_AWAY,
    FISHING_END_NO_MON,
};

static bool32 (*const sFishingStateFuncs[])(struct Task *) =
{
    [FISHING_INIT]                  = Fishing_Init,
    [FISHING_GET_ROD_OUT]           = Fishing_GetRodOut,
    [FISHING_WAIT_BEFORE_DOTS]      = Fishing_WaitBeforeDots,
    [FISHING_INIT_DOTS]             = Fishing_InitDots,
    [FISHING_SHOW_DOTS]             = Fishing_ShowDots,
    [FISHING_CHECK_FOR_BITE]        = Fishing_CheckForBite,
    [FISHING_START_ENCOUNTER]       = Fishing_StartEncounter,
    [FISHING_NOT_EVEN_NIBBLE]       = Fishing_NotEvenNibble,
    [FISHING_NO_MON]                = Fishing_NoMon,
    [FISHING_PUT_ROD_AWAY]          = Fishing_PutRodAway,
    [FISHING_END_NO_MON]            = Fishing_EndNoMon,
};

#define tStep              data[0]
#define tFrameCounter      data[1]
#define tNumDots           data[2]
#define tDotsRequired      data[3]
#define tPlayerGfxId       data[14]
#define tFishingRod        data[15]

void StartFishing(u8 rod)
{
    u8 taskId = CreateTask(Task_Fishing, 0xFF);

    gTasks[taskId].tFishingRod = rod;
    Task_Fishing(taskId);
}

static void Task_Fishing(u8 taskId)
{
    while (sFishingStateFuncs[gTasks[taskId].tStep](&gTasks[taskId]))
        ;
}

static bool32 Fishing_Init(struct Task *task)
{
    LockPlayerFieldControls();
    gPlayerAvatar.preventStep = TRUE;
    task->tStep = FISHING_GET_ROD_OUT;
    return FALSE;
}

static bool32 Fishing_GetRodOut(struct Task *task)
{
    struct ObjectEvent *playerObjEvent;

    task->tPlayerGfxId = gObjectEvents[gPlayerAvatar.objectEventId].graphicsId;
    playerObjEvent = &gObjectEvents[gPlayerAvatar.objectEventId];
    ObjectEventClearHeldMovementIfActive(playerObjEvent);
    playerObjEvent->enableAnim = TRUE;
    SetPlayerAvatarFishing(playerObjEvent->facingDirection);
    task->tStep = FISHING_WAIT_BEFORE_DOTS;
    return FALSE;
}

static bool32 Fishing_WaitBeforeDots(struct Task *task)
{
    AlignFishingAnimationFrames();

    // Wait one second
    task->tFrameCounter++;
    if (task->tFrameCounter >= 60)
        task->tStep = FISHING_INIT_DOTS;
    return FALSE;
}

static bool32 Fishing_InitDots(struct Task *task)
{
    LoadMessageBoxAndFrameGfx(0, TRUE);
    task->tStep = FISHING_SHOW_DOTS;
    task->tFrameCounter = 0;
    task->tNumDots = 0;
    // Every cast waits the same 5 dots and then always hooks.
    task->tDotsRequired = 5;
    return TRUE;
}

static bool32 Fishing_ShowDots(struct Task *task)
{
    const u8 dot[] = _("·");

    AlignFishingAnimationFrames();
    task->tFrameCounter++;
    if (JOY_NEW(A_BUTTON))
    {
        if (!DoesFishingMinigameAllowCancel())
            return FALSE;

        task->tStep = FISHING_NOT_EVEN_NIBBLE;
        return TRUE;
    }
    else
    {
        if (task->tFrameCounter >= 20)
        {
            task->tFrameCounter = 0;
            if (task->tNumDots >= task->tDotsRequired)
                task->tStep = FISHING_CHECK_FOR_BITE;
            else
            {
                AddTextPrinterParameterized(0, FONT_NORMAL, dot, task->tNumDots * 8, 1, 0, NULL);
                task->tNumDots++;
            }
        }
        return FALSE;
    }
}

static bool32 Fishing_CheckForBite(struct Task *task)
{
    AlignFishingAnimationFrames();
    if (!DoesCurrentMapHaveFishingMons())
    {
        task->tStep = FISHING_NOT_EVEN_NIBBLE;
        return TRUE;
    }

    task->tStep = FISHING_START_ENCOUNTER;
    task->tFrameCounter = 0;
    return FALSE;
}

static bool32 Fishing_StartEncounter(struct Task *task)
{
    if (task->tFrameCounter == 0)
        AlignFishingAnimationFrames();

    RunTextPrinters();

    if (task->tFrameCounter == 0)
    {
        if (!IsTextPrinterActiveOnWindow(0))
        {
            struct ObjectEvent *playerObjEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

            ObjectEventSetGraphicsId(playerObjEvent, task->tPlayerGfxId);
            ObjectEventTurn(playerObjEvent, playerObjEvent->movementDirection);
            if (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_SURFING)
                SetSurfBlob_PlayerOffset(gObjectEvents[gPlayerAvatar.objectEventId].fieldEffectSpriteId, FALSE, 0);
            gSprites[gPlayerAvatar.spriteId].x2 = 0;
            gSprites[gPlayerAvatar.spriteId].y2 = 0;
            ClearDialogWindowAndFrame(0, TRUE);
            task->tFrameCounter++;
            return FALSE;
        }
    }

    if (task->tFrameCounter != 0)
    {
        gPlayerAvatar.preventStep = FALSE;
        UnlockPlayerFieldControls();
        FishingWildEncounter(task->tFishingRod);
        RecordFishingAttemptForTV(TRUE);
        DestroyTask(FindTaskIdByFunc(Task_Fishing));
    }
    return FALSE;
}

static bool32 Fishing_NotEvenNibble(struct Task *task)
{
    gChainFishingDexNavStreak = 0;
    AlignFishingAnimationFrames();
    StartSpriteAnim(&gSprites[gPlayerAvatar.spriteId], GetFishingNoCatchDirectionAnimNum(GetPlayerFacingDirection()));
    FillWindowPixelBuffer(0, PIXEL_FILL(1));
    AddTextPrinterParameterized2(0, FONT_NORMAL, sText_NotEvenANibble, 1, 0, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
    task->tStep = FISHING_NO_MON;
    return TRUE;
}

static bool32 Fishing_NoMon(struct Task *task)
{
    AlignFishingAnimationFrames();
    task->tStep = FISHING_PUT_ROD_AWAY;
    return FALSE;
}

static bool32 Fishing_PutRodAway(struct Task *task)
{
    AlignFishingAnimationFrames();
    if (gSprites[gPlayerAvatar.spriteId].animEnded)
    {
        struct ObjectEvent *playerObjEvent = &gObjectEvents[gPlayerAvatar.objectEventId];

        ObjectEventSetGraphicsId(playerObjEvent, task->tPlayerGfxId);
        ObjectEventTurn(playerObjEvent, playerObjEvent->movementDirection);
        if (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_SURFING)
            SetSurfBlob_PlayerOffset(gObjectEvents[gPlayerAvatar.objectEventId].fieldEffectSpriteId, FALSE, 0);
        gSprites[gPlayerAvatar.spriteId].x2 = 0;
        gSprites[gPlayerAvatar.spriteId].y2 = 0;
        task->tStep = FISHING_END_NO_MON;
    }
    return FALSE;
}

static bool32 Fishing_EndNoMon(struct Task *task)
{
    RunTextPrinters();
    if (!IsTextPrinterActiveOnWindow(0))
    {
        gPlayerAvatar.preventStep = FALSE;
        UnlockPlayerFieldControls();
        UnfreezeObjectEvents();
        ClearDialogWindowAndFrame(0, TRUE);
        RecordFishingAttemptForTV(FALSE);
        DestroyTask(FindTaskIdByFunc(Task_Fishing));
    }
    return FALSE;
}

static bool32 DoesFishingMinigameAllowCancel(void)
{
    switch (I_FISHING_MINIGAME)
    {
    case GEN_1:
    case GEN_2:
        return FALSE;
    case GEN_3:
    default:
            return TRUE;
    }
}

#undef tStep
#undef tFrameCounter
#undef tNumDots
#undef tDotsRequired
#undef tPlayerGfxId
#undef tFishingRod

static void AlignFishingAnimationFrames(void)
{
    struct Sprite *playerSprite = &gSprites[gPlayerAvatar.spriteId];
    u8 animCmdIndex;
    u8 animType;

    AnimateSprite(playerSprite);
    playerSprite->x2 = 0;
    playerSprite->y2 = 0;
    animCmdIndex = playerSprite->animCmdIndex;
    if (playerSprite->anims[playerSprite->animNum][animCmdIndex].type == -1)
    {
        animCmdIndex--;
    }
    else
    {
        playerSprite->animDelayCounter++;
        if (playerSprite->anims[playerSprite->animNum][animCmdIndex].type == -1)
            animCmdIndex--;
    }
    animType = playerSprite->anims[playerSprite->animNum][animCmdIndex].type;
    if (animType == 1 || animType == 2 || animType == 3)
    {
        playerSprite->x2 = 8;
        if (GetPlayerFacingDirection() == 3)
            playerSprite->x2 = -8;
    }
    if (animType == 5)
        playerSprite->y2 = -8;
    if (animType == 10 || animType == 11)
        playerSprite->y2 = 8;
    if (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_SURFING)
        SetSurfBlob_PlayerOffset(gObjectEvents[gPlayerAvatar.objectEventId].fieldEffectSpriteId, TRUE, playerSprite->y2);
}

void UpdateChainFishingStreak()
{
    if (!I_FISHING_CHAIN)
        return;

    if (gChainFishingDexNavStreak == MAX_u8)
        return;

    gChainFishingDexNavStreak++;
}

u32 CalculateChainFishingShinyRolls(void)
{
    if (!I_FISHING_CHAIN || !gIsFishingEncounter)
        return 0;
    u32 a = 2 * min(gChainFishingDexNavStreak, FISHING_CHAIN_SHINY_STREAK_MAX);
    return a;
}

bool32 ShouldUseFishingEnvironmentInBattle()
{
    return (I_FISHING_ENVIRONMENT >= GEN_4 && gIsFishingEncounter);
}

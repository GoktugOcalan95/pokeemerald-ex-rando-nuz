#include "global.h"
#include "bg.h"
#include "dma3.h"
#include "main.h"
#include "menu.h"
#include "menu_helpers.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "test/test.h"

static bool8 sMessageFinished;

static void MessageFinished(u8 taskId)
{
    sMessageFinished = TRUE;
    DestroyTask(taskId);
}

TEST("Bag message timing holds completed Auto feedback before continuing")
{
    u32 speed;
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    const struct BgTemplate bg = { .bg = 0, .charBaseIndex = 0, .mapBaseIndex = 30 };
    const struct WindowTemplate windows[] =
    {
        { .bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 26, .height = 4, .baseBlock = 128 },
        DUMMY_WIN_TEMPLATE,
    };

    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_AUTO; }
    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_INSTANT; }

    DeactivateAllTextPrinters();
    ClearDma3Requests();
    ResetBgsAndClearDma3BusyFlags(FALSE);
    InitBgFromTemplate(&bg);
    InitWindows(windows);
    SetDefaultFontsPointer();
    gSaveBlock2Ptr->optionsTextSpeed = speed;
    gTextFlags = (TextFlags){0};
    gMain.newKeys = 0;
    gMain.heldKeys = 0;
    sMessageFinished = FALSE;
    u8 taskId = CreateTask(MessageFinished, 0);
    DisplayMessageAndContinueTaskWithMinimumDuration(taskId, 0, 10, 13, FONT_NORMAL, GetPlayerTextSpeedDelay(),
        gText_WontHaveEffect, MessageFinished, speed == OPTIONS_TEXT_SPEED_AUTO ? 64 : 0);

    if (speed == OPTIONS_TEXT_SPEED_AUTO)
    {
        u32 frame = 0;
        do
        {
            gTasks[taskId].func(taskId);
            ProcessDma3Requests();
            EXPECT(!sMessageFinished);
        } while (IsTextPrinterActiveOnWindow(0) && ++frame < 128);
        EXPECT(!IsTextPrinterActiveOnWindow(0));
        for (u32 frame = 1; frame < 64; frame++)
        {
            gMain.newKeys = A_BUTTON | B_BUTTON;
            gTasks[taskId].func(taskId);
            EXPECT(!sMessageFinished);
        }
        gTasks[taskId].func(taskId);
    }
    else
    {
        for (u32 frame = 0; frame < 128; frame++)
        {
            gTasks[taskId].func(taskId);
            ProcessDma3Requests();
            EXPECT(!sMessageFinished);
        }
        gMain.newKeys = A_BUTTON;
        for (u32 frame = 0; frame < 16 && !sMessageFinished; frame++)
        {
            gTasks[taskId].func(taskId);
            ProcessDma3Requests();
        }
    }
    EXPECT(sMessageFinished);

    DeactivateAllTextPrinters();
    FreeAllWindowBuffers();
    ResetBgsAndClearDma3BusyFlags(FALSE);
    gTextFlags = (TextFlags){0};
    gMain.newKeys = 0;
    gMain.heldKeys = 0;
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

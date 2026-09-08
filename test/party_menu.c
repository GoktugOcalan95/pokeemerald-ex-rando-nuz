#include "global.h"
#include "battle.h"
#include "bg.h"
#include "dma3.h"
#include "main.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "party_menu.h"
#include "pokemon.h"
#include "test/test.h"

#define TEST_MENU_DIR_DOWN     1
#define TEST_MENU_DIR_UP      -1
#define TEST_MENU_DIR_RIGHT    2

static void SetTestPartySize(enum BattleTrainer trainer, u8 partySize)
{
    for (u32 i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&gParties[trainer][i]);

    for (u32 i = 0; i < partySize; i++)
        CreateMon(&gParties[trainer][i], SPECIES_WOBBUFFET, 50, 0, OTID_STRUCT_PRESET(0));

    gPartiesCount[trainer] = partySize;
}

TEST("Full multi partner party menu stops down navigation at partner party count")
{
    SetTestPartySize(B_TRAINER_PLAYER, PARTY_SIZE);
    SetTestPartySize(B_TRAINER_PARTNER, 2);
    gPartyMenu.layout = PARTY_LAYOUT_MULTI_FULL_PARTNER;

    EXPECT_EQ(Test_UpdatePartySelectionSingleLayout(1, TEST_MENU_DIR_DOWN, FALSE, 0), PARTY_SIZE + 1);
}

TEST("Full multi partner party menu allows down navigation through partner party count")
{
    SetTestPartySize(B_TRAINER_PLAYER, 2);
    SetTestPartySize(B_TRAINER_PARTNER, PARTY_SIZE);
    gPartyMenu.layout = PARTY_LAYOUT_MULTI_FULL_PARTNER;

    EXPECT_EQ(Test_UpdatePartySelectionSingleLayout(1, TEST_MENU_DIR_DOWN, FALSE, 0), 2);
}

TEST("Full multi partner party menu wraps cancel up to partner party count")
{
    SetTestPartySize(B_TRAINER_PLAYER, PARTY_SIZE);
    SetTestPartySize(B_TRAINER_PARTNER, 2);
    gPartyMenu.layout = PARTY_LAYOUT_MULTI_FULL_PARTNER;

    EXPECT_EQ(Test_UpdatePartySelectionSingleLayout(PARTY_SIZE + 1, TEST_MENU_DIR_UP, FALSE, 0), 1);
}

TEST("Party menu messages remain readable in Auto and wait for input in Instant")
{
    u32 speed;
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    u8 taskId;
    static const struct BgTemplate bg = { .bg = 0, .charBaseIndex = 0, .mapBaseIndex = 30 };
    static const struct WindowTemplate windows[] =
    {
        { .bg = 0, .width = 1, .height = 1, .baseBlock = 1 },
        { .bg = 0, .width = 1, .height = 1, .baseBlock = 2 },
        { .bg = 0, .width = 1, .height = 1, .baseBlock = 3 },
        { .bg = 0, .width = 1, .height = 1, .baseBlock = 4 },
        { .bg = 0, .width = 1, .height = 1, .baseBlock = 5 },
        { .bg = 0, .width = 1, .height = 1, .baseBlock = 6 },
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
    taskId = DisplayPartyMenuMessage(gText_WontHaveEffect, TRUE);

    for (u32 frame = 0; frame < 48; frame++)
    {
        EXPECT(IsPartyMenuTextPrinterActive());
        gTasks[taskId].func(taskId);
        ProcessDma3Requests();
    }
    EXPECT(IsPartyMenuTextPrinterActive());
    if (speed == OPTIONS_TEXT_SPEED_INSTANT)
    {
        for (u32 frame = 0; frame < 60; frame++)
        {
            gTasks[taskId].func(taskId);
            EXPECT(IsPartyMenuTextPrinterActive());
        }
        gMain.newKeys = A_BUTTON;
    }
    for (u32 frame = 0; frame < 16 && IsPartyMenuTextPrinterActive(); frame++)
    {
        gTasks[taskId].func(taskId);
        ProcessDma3Requests();
    }
    EXPECT(!IsPartyMenuTextPrinterActive());

    DeactivateAllTextPrinters();
    FreeAllWindowBuffers();
    ResetBgsAndClearDma3BusyFlags(FALSE);
    gTextFlags = (TextFlags){0};
    gMain.newKeys = 0;
    gMain.heldKeys = 0;
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

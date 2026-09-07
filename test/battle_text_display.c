#include "global.h"
#include "battle.h"
#include "battle_message.h"
#include "bg.h"
#include "dma3.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "string_util.h"
#include "text.h"
#include "window.h"
#include "test/test.h"

#define TEXT_WINDOW_BYTES (26 * 4 * 32)

static const struct WindowTemplate sTestWindows[] =
{
    { .bg = 0, .width = 26, .height = 4, .baseBlock = 1 },
    { .bg = 0, .tilemapTop = 5, .width = 26, .height = 4, .baseBlock = 105 },
    DUMMY_WIN_TEMPLATE,
};

static void InitDisplay(u32 speed)
{
    DeactivateAllTextPrinters();
    ClearDma3Requests();
    ResetBgsAndClearDma3BusyFlags(FALSE);
    const struct BgTemplate bg = { .bg = 0, .charBaseIndex = 0, .mapBaseIndex = 30 };
    InitBgFromTemplate(&bg);
    InitWindows(sTestWindows);
    SetDefaultFontsPointer();
    gSaveBlock2Ptr->optionsTextSpeed = speed;
    gTextFlags = (TextFlags){0};
    gMain.newKeys = 0;
    gMain.heldKeys = 0;
    gBattleTypeFlags = 0;
    gBattleScripting.windowsType = 0;
}

static void EndDisplay(void)
{
    FreeAllWindowBuffers();
    ResetBgsAndClearDma3BusyFlags(FALSE);
    gTextFlags = (TextFlags){0};
    gMain.newKeys = 0;
    gMain.heldKeys = 0;
}

static void Frame(void)
{
    UpdateTextPrinterDisplayTimers();
    RunTextPrinters();
    ProcessDma3Requests();
}

static void FinishPrinting(void)
{
    u32 frames = 0;
    while (IsTextPrinterActiveOnWindow(B_WIN_MSG) && frames++ < 512)
        Frame();
    EXPECT(!IsTextPrinterActiveOnWindow(B_WIN_MSG));
}

static struct TextPrinterTemplate NarrationTemplate(u32 windowId, const u8 *text)
{
    return (struct TextPrinterTemplate){
        .type = WINDOW_TEXT_PRINTER,
        .windowId = windowId,
        .fontId = FONT_NORMAL,
        .currentChar = text,
        .y = 1,
        .currentY = 1,
        .color = { .foreground = 1, .background = 15, .accent = 15, .shadow = 6 },
    };
}

static void DrawExpected(const u8 *text)
{
    struct TextPrinterTemplate template = NarrationTemplate(1, text);
    FillWindowPixelBuffer(1, PIXEL_FILL(15));
    AddTextPrinter(&template, 0, NULL);
}

TEST("Battle text display holds replacements for real frames at every text speed")
{
    u32 speed;
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    static EWRAM_DATA u8 pixels[TEXT_WINDOW_BYTES];
    u8 nextText[64];

    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_MID; }
    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_FAST; }
    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_INSTANT; }
    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_AUTO; }

    InitDisplay(speed);
    BattlePutTextOnWindow(COMPOUND_STRING("Mudkip used Fly!"), B_WIN_MSG);
    FinishPrinting();
    memcpy(pixels, gWindows[0].tileData, sizeof(pixels));
    EXPECT(!IsTextWindowDisplayComplete(B_WIN_MSG));

    StringCopy(nextText, COMPOUND_STRING("Mudkip flew up high!"));
    BattlePutTextOnWindow(nextText, B_WIN_MSG);
    StringCopy(nextText, COMPOUND_STRING("Changed buffer"));
    for (u32 i = 0; i < 100; i++)
        RunTextPrinters();
    EXPECT(IsTextPrinterActiveOnWindow(B_WIN_MSG));
    EXPECT_EQ(memcmp(pixels, gWindows[0].tileData, sizeof(pixels)), 0);
    for (u32 i = 1; i < B_MIN_TEXT_DISPLAY_FRAMES; i++)
        Frame();
    EXPECT_EQ(memcmp(pixels, gWindows[0].tileData, sizeof(pixels)), 0);
    Frame();
    FinishPrinting();
    EXPECT_NE(memcmp(pixels, gWindows[0].tileData, sizeof(pixels)), 0);

    DrawExpected(COMPOUND_STRING("Mudkip flew up high!"));
    EXPECT_EQ(memcmp(gWindows[0].tileData, gWindows[1].tileData, sizeof(pixels)), 0);
    EndDisplay();
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Battle text display counts existing waits and gives blank clears no dwell")
{
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;

    InitDisplay(OPTIONS_TEXT_SPEED_INSTANT);
    BattlePutTextOnWindow(COMPOUND_STRING("A critical hit!"), B_WIN_MSG);
    FinishPrinting();
    for (u32 i = 0; i < 64; i++)
        Frame();
    EXPECT(IsTextWindowDisplayComplete(B_WIN_MSG));
    BattlePutTextOnWindow(COMPOUND_STRING(""), B_WIN_MSG);
    Frame();
    EXPECT(!IsTextPrinterActiveOnWindow(B_WIN_MSG));
    BattlePutTextOnWindow(COMPOUND_STRING("Next"), B_WIN_MSG);
    Frame();
    EXPECT(!IsTextPrinterActiveOnWindow(B_WIN_MSG));
    EndDisplay();
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Battle text display protects Auto page clears and scrolling")
{
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    const u8 *text;
    static EWRAM_DATA u8 pixels[TEXT_WINDOW_BYTES];

    PARAMETRIZE { text = COMPOUND_STRING("Fainted!\pNext"); }
    PARAMETRIZE { text = COMPOUND_STRING("First\nSecond\lThird"); }

    InitDisplay(OPTIONS_TEXT_SPEED_AUTO);
    BattlePutTextOnWindow(text, B_WIN_MSG);
    Frame();
    memcpy(pixels, gWindows[0].tileData, sizeof(pixels));
    for (u32 i = 1; i < B_MIN_TEXT_DISPLAY_FRAMES; i++)
        Frame();
    EXPECT_EQ(memcmp(pixels, gWindows[0].tileData, sizeof(pixels)), 0);
    FinishPrinting();
    EXPECT_NE(memcmp(pixels, gWindows[0].tileData, sizeof(pixels)), 0);
    EXPECT_EQ(gMain.newKeys, 0);
    EXPECT_EQ(Menu_ProcessInputNoWrap(), MENU_NOTHING_CHOSEN);
    EndDisplay();
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Battle text display remembers early confirmation only for the current page")
{
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    static EWRAM_DATA u8 pixels[TEXT_WINDOW_BYTES];

    InitDisplay(OPTIONS_TEXT_SPEED_INSTANT);
    struct TextPrinterTemplate template = NarrationTemplate(B_WIN_MSG, COMPOUND_STRING("First\pSecond\p"));
    EXPECT(AddTextPrinterWithMinimumDisplayTime(&template, 1, B_MIN_TEXT_DISPLAY_FRAMES, TRUE));
    Frame();
    memcpy(pixels, gWindows[0].tileData, sizeof(pixels));
    gMain.newKeys = A_BUTTON;
    Frame();
    gMain.newKeys = 0;
    for (u32 i = 2; i < B_MIN_TEXT_DISPLAY_FRAMES; i++)
        Frame();
    EXPECT_EQ(memcmp(pixels, gWindows[0].tileData, sizeof(pixels)), 0);
    for (u32 i = 0; i < 64; i++)
        Frame();
    EXPECT(IsTextPrinterActiveOnWindow(B_WIN_MSG));
    EXPECT_NE(memcmp(pixels, gWindows[0].tileData, sizeof(pixels)), 0);
    gMain.newKeys = B_BUTTON;
    Frame();
    gMain.newKeys = 0;
    FinishPrinting();
    EndDisplay();
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Battle text display queues clears and multiple messages in order")
{
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    static EWRAM_DATA u8 first[TEXT_WINDOW_BYTES];

    InitDisplay(OPTIONS_TEXT_SPEED_INSTANT);
    BattlePutTextOnWindow(COMPOUND_STRING("First"), B_WIN_MSG);
    Frame();
    memcpy(first, gWindows[0].tileData, sizeof(first));
    BattlePutTextOnWindow(COMPOUND_STRING(""), B_WIN_MSG);
    BattlePutTextOnWindow(COMPOUND_STRING("Second"), B_WIN_MSG);
    BattlePutTextOnWindow(COMPOUND_STRING("Third"), B_WIN_MSG);
    for (u32 i = 1; i < B_MIN_TEXT_DISPLAY_FRAMES; i++)
        Frame();
    EXPECT_EQ(memcmp(first, gWindows[0].tileData, sizeof(first)), 0);
    Frame();
    EXPECT(IsTextPrinterActiveOnWindow(B_WIN_MSG));
    EXPECT_NE(memcmp(first, gWindows[0].tileData, sizeof(first)), 0);
    FinishPrinting();
    DrawExpected(COMPOUND_STRING("Third"));
    EXPECT_EQ(memcmp(gWindows[0].tileData, gWindows[1].tileData, sizeof(first)), 0);
    EndDisplay();
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Battle text display teardown cancels pending text and leaves ordinary printers unchanged")
{
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;

    InitDisplay(OPTIONS_TEXT_SPEED_AUTO);
    BattlePutTextOnWindow(COMPOUND_STRING("First"), B_WIN_MSG);
    Frame();
    BattlePutTextOnWindow(COMPOUND_STRING("Pending"), B_WIN_MSG);
    RemoveWindow(B_WIN_MSG);
    EXPECT(!IsTextPrinterActiveOnWindow(B_WIN_MSG));
    EXPECT_EQ(AddWindow(&sTestWindows[0]), B_WIN_MSG);
    AddTextPrinterParameterized(B_WIN_MSG, FONT_NORMAL, COMPOUND_STRING("Overworld\p"), 0, 1, 1, NULL);
    Frame();
    EXPECT(!IsTextPrinterActiveOnWindow(B_WIN_MSG));
    EndDisplay();
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Battle text display does not count hidden fade frames or delay battle menu labels")
{
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    static EWRAM_DATA u8 pixels[TEXT_WINDOW_BYTES];

    InitDisplay(OPTIONS_TEXT_SPEED_AUTO);
    BattlePutTextOnWindow(COMPOUND_STRING("Mudkip used Fly!"), B_WIN_MSG);
    FinishPrinting();
    memcpy(pixels, gWindows[0].tileData, sizeof(pixels));
    BattlePutTextOnWindow(COMPOUND_STRING("Mudkip flew up high!"), B_WIN_MSG);
    gPaletteFade.active = TRUE;
    for (u32 i = 0; i < 64; i++)
        Frame();
    EXPECT_EQ(memcmp(pixels, gWindows[0].tileData, sizeof(pixels)), 0);
    gPaletteFade.active = FALSE;
    BattlePutTextOnWindow(COMPOUND_STRING("Choose"), B_WIN_ACTION_PROMPT);
    EXPECT(!IsTextPrinterActiveOnWindow(B_WIN_ACTION_PROMPT));
    EXPECT(IsTextPrinterActiveOnWindow(B_WIN_MSG));
    for (u32 i = 1; i < B_MIN_TEXT_DISPLAY_FRAMES; i++)
        Frame();
    EXPECT_EQ(memcmp(pixels, gWindows[0].tileData, sizeof(pixels)), 0);
    FinishPrinting();
    EndDisplay();
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Battle text display preserves zero speed printing and automatic scroll flags")
{
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    struct TextPrinterTemplate template;

    InitDisplay(OPTIONS_TEXT_SPEED_MID);
    template = NarrationTemplate(B_WIN_MSG, COMPOUND_STRING("Recorded battle message"));
    EXPECT(AddTextPrinterWithMinimumDisplayTime(&template, 0, B_MIN_TEXT_DISPLAY_FRAMES, TRUE));
    Frame();
    EXPECT(!IsTextPrinterActiveOnWindow(B_WIN_MSG));
    for (u32 i = 0; i < B_MIN_TEXT_DISPLAY_FRAMES; i++)
        Frame();

    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_INSTANT;
    gTextFlags.autoScroll = TRUE;
    template = NarrationTemplate(B_WIN_MSG, COMPOUND_STRING("Link page\p"));
    EXPECT(AddTextPrinterWithMinimumDisplayTime(&template, 1, B_MIN_TEXT_DISPLAY_FRAMES, TRUE));
    gTextFlags.autoScroll = FALSE;
    Frame();
    for (u32 i = 0; i < B_MIN_TEXT_DISPLAY_FRAMES; i++)
        Frame();
    EXPECT(!IsTextPrinterActiveOnWindow(B_WIN_MSG));
    EndDisplay();
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

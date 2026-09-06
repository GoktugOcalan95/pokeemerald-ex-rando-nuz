#include "global.h"
#include "main.h"
#include "menu.h"
#include "script.h"
#include "text.h"
#include "test/test.h"

extern bool8 ScrCmd_waitbuttonpress(struct ScriptContext *ctx);

TEST("Text speed Auto advances page and script waits without choosing menu answers")
{
    struct TextPrinter printer = {0};
    struct ScriptContext ctx = {0};
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;

    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_AUTO;
    gTextFlags.forceMidTextSpeed = FALSE;
    gMain.newKeys = 0;
    gMain.heldKeys = 0;

    EXPECT(TextPrinterWait(&printer));
    EXPECT(TextPrinterWaitWithDownArrow(&printer));
    EXPECT(ScrCmd_waitbuttonpress(&ctx));
    EXPECT(ctx.nativePtr());
    EXPECT_EQ(gMain.newKeys, 0);
    EXPECT_EQ(gMain.heldKeys, 0);
    EXPECT_EQ(Menu_ProcessInputNoWrap(), MENU_NOTHING_CHOSEN);

    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Text speed Medium Fast and Instant wait for a new button press")
{
    struct TextPrinter printer = {0};
    struct ScriptContext ctx = {0};
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    u32 speed;

    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_INSTANT; }
    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_MID; }
    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_FAST; }

    gSaveBlock2Ptr->optionsTextSpeed = speed;
    gTextFlags.forceMidTextSpeed = FALSE;
    gTextFlags.autoScroll = FALSE;
    gMain.newKeys = 0;
    gMain.heldKeys = A_BUTTON;

    EXPECT(!TextPrinterWait(&printer));
    EXPECT(ScrCmd_waitbuttonpress(&ctx));
    EXPECT(!ctx.nativePtr());
    gMain.newKeys = A_BUTTON;
    EXPECT(TextPrinterWait(&printer));
    EXPECT(ctx.nativePtr());

    gMain.newKeys = 0;
    gMain.heldKeys = 0;
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Text speed respects forced medium speed")
{
    struct TextPrinter printer = {0};
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;

    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_AUTO;
    gTextFlags.forceMidTextSpeed = TRUE;
    gTextFlags.autoScroll = FALSE;
    gMain.newKeys = 0;
    EXPECT_EQ(GetPlayerTextSpeed(), OPTIONS_TEXT_SPEED_MID);
    EXPECT(!TextPrinterWait(&printer));
    EXPECT_EQ((u32)gSaveBlock2Ptr->optionsTextSpeed, OPTIONS_TEXT_SPEED_AUTO);

    gTextFlags.forceMidTextSpeed = FALSE;
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Text speed choices fit beside the Options label")
{
    u32 width = GetStringWidth(FONT_NORMAL, COMPOUND_STRING("AUTO"), 0)
              + GetStringWidth(FONT_NORMAL, COMPOUND_STRING("MID"), 0)
              + GetStringWidth(FONT_NORMAL, COMPOUND_STRING("FAST"), 0)
              + GetStringWidth(FONT_NORMAL, COMPOUND_STRING("INSTANT"), 0)
              + 3 * 4;

    EXPECT_GT(198 - width, 8 + GetStringWidth(FONT_NORMAL, COMPOUND_STRING("TEXT SPEED"), 0));
}

TEST("Text speed validates current settings")
{
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    u32 speed;

    for (speed = OPTIONS_TEXT_SPEED_MID; speed <= OPTIONS_TEXT_SPEED_AUTO; speed++)
    {
        gSaveBlock2Ptr->optionsTextSpeed = speed;
        EXPECT_EQ(GetSavedTextSpeed(), speed);
    }

    for (speed = 0; speed < 8; speed++)
    {
        if (speed >= OPTIONS_TEXT_SPEED_MID && speed <= OPTIONS_TEXT_SPEED_AUTO)
            continue;
        gSaveBlock2Ptr->optionsTextSpeed = speed;
        EXPECT_EQ(GetSavedTextSpeed(), OPTIONS_TEXT_SPEED_MID);
        EXPECT_EQ((u32)gSaveBlock2Ptr->optionsTextSpeed, OPTIONS_TEXT_SPEED_MID);
    }
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

TEST("Text speed Instant and Auto share instant rendering settings")
{
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    u32 speed;

    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_INSTANT; }
    PARAMETRIZE { speed = OPTIONS_TEXT_SPEED_AUTO; }

    gSaveBlock2Ptr->optionsTextSpeed = speed;
    gTextFlags.forceMidTextSpeed = FALSE;
    EXPECT(IsPlayerTextSpeedInstant());
    EXPECT_EQ(GetPlayerTextSpeedDelay(), 1);
    EXPECT_EQ(GetPlayerTextSpeedModifier(), TEXT_SPEED_INSTANT_MODIFIER);
    EXPECT_EQ(GetPlayerTextScrollSpeed(), 6);
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
}

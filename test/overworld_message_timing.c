#include "global.h"
#include "field_message_box.h"
#include "main.h"
#include "menu.h"
#include "test/test.h"

TEST("Overworld feedback waits 48 completed frames in Auto and accepts earlier A or B")
{
    u32 oldSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_AUTO;
    gMain.newKeys = 0;
    FieldMessage_StartInputWait(48);
    for (u32 i = 0; i < 48; i++)
    {
        EXPECT(!FieldMessage_WaitForInput());
        gMain.vblankCounter1++;
    }
    EXPECT(FieldMessage_WaitForInput());
    FieldMessage_StartInputWait(48);
    gMain.newKeys = A_BUTTON;
    EXPECT(FieldMessage_WaitForInput());
    gMain.vblankCounter1++;
    FieldMessage_StartInputWait(48);
    gMain.newKeys = B_BUTTON;
    EXPECT(FieldMessage_WaitForInput());
    gMain.newKeys = 0;
    gSaveBlock2Ptr->optionsTextSpeed = oldSpeed;
}

TEST("Overworld acknowledgements need distinct presses and other speeds never auto-dismiss")
{
    u32 oldSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    for (u32 speed = OPTIONS_TEXT_SPEED_MID; speed <= OPTIONS_TEXT_SPEED_AUTO; speed++)
    {
        gSaveBlock2Ptr->optionsTextSpeed = speed;
        FieldMessage_StartInputWait(speed == OPTIONS_TEXT_SPEED_AUTO ? 0 : 48);
        gMain.newKeys = 0;
        gMain.vblankCounter1 += 100;
        EXPECT(!FieldMessage_WaitForInput());
        EXPECT_EQ(Menu_ProcessInputNoWrap(), MENU_NOTHING_CHOSEN);
        gMain.newKeys = A_BUTTON;
        EXPECT(FieldMessage_WaitForInput());
        FieldMessage_StartInputWait(0);
        EXPECT(!FieldMessage_WaitForInput());
        gMain.newKeys = 0;
        gMain.vblankCounter1++;
        EXPECT(!FieldMessage_WaitForInput());
        gMain.newKeys = B_BUTTON;
        EXPECT(FieldMessage_WaitForInput());
    }
    gMain.newKeys = 0;
    gSaveBlock2Ptr->optionsTextSpeed = oldSpeed;
}

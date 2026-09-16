#include "global.h"
#include "battle.h"
#include "main.h"
#include "palette.h"
#include "text.h"
#include "test/test.h"
#include "constants/characters.h"

bool32 Test_ShouldAdvanceLevelUpPanel(void);

TEST("Battle reward text Auto holds each stat panel for sixty four visible frames")
{
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_AUTO;
    gPaletteFade.active = FALSE;
    gDisableTextPrinters = FALSE;
    gMain.newKeys = A_BUTTON;
    for (u32 panel = 0; panel < 2; panel++)
    {
        gPauseCounterBattle = 0;
        for (u32 frame = 0; frame < 63; frame++)
            EXPECT(!Test_ShouldAdvanceLevelUpPanel());
        gPaletteFade.active = TRUE;
        EXPECT(!Test_ShouldAdvanceLevelUpPanel());
        gPaletteFade.active = FALSE;
        EXPECT(Test_ShouldAdvanceLevelUpPanel());
    }
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_INSTANT;
    gMain.newKeys = 0;
    EXPECT(!Test_ShouldAdvanceLevelUpPanel());
    gMain.newKeys = A_BUTTON;
    EXPECT(Test_ShouldAdvanceLevelUpPanel());
}

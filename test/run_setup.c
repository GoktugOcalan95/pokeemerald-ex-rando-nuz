#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "run_setup.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Run setup detaches the main menu tilemap and clears its highlight mask")
{
    static EWRAM_DATA u16 mainMenuTilemap[BG_SCREEN_SIZE / sizeof(u16)];
    static const struct BgTemplate bgTemplate =
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    };
    u16 previousDisplayControl = GetGpuReg(REG_OFFSET_DISPCNT);
    u16 displayControl = DISPCNT_BG0_ON | DISPCNT_OBJ_ON | DISPCNT_WIN0_ON;

    ResetBgsAndClearDma3BusyFlags(FALSE);
    InitBgFromTemplate(&bgTemplate);
    SetBgTilemapBuffer(0, mainMenuTilemap);
    SetGpuReg(REG_OFFSET_DISPCNT, displayControl);
    RunSetup_PrepareDisplay();

    EXPECT_EQ(GetBgTilemapBuffer(0), NULL);
    EXPECT_EQ(GetGpuReg(REG_OFFSET_DISPCNT), displayControl & ~DISPCNT_WIN0_ON);
    ResetBgsAndClearDma3BusyFlags(FALSE);
    SetGpuReg(REG_OFFSET_DISPCNT, previousDisplayControl);
}

TEST("Run setup clears its tilemap before the introduction")
{
    static EWRAM_DATA u16 runSetupTilemap[BG_SCREEN_SIZE / sizeof(u16)];
    bool32 tilemapCleared = TRUE;
    static const struct BgTemplate bgTemplate =
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    };

    memset(runSetupTilemap, 0xFF, sizeof(runSetupTilemap));
    ResetBgsAndClearDma3BusyFlags(FALSE);
    InitBgFromTemplate(&bgTemplate);
    SetBgTilemapBuffer(0, runSetupTilemap);
    RunSetup_ClearDisplayTilemap();

    for (u32 y = 0; y < DISPLAY_TILE_HEIGHT; y++)
    {
        for (u32 x = 0; x < DISPLAY_TILE_WIDTH; x++)
        {
            if (runSetupTilemap[y * 32 + x] != 0)
                tilemapCleared = FALSE;
        }
    }
    EXPECT(tilemapCleared);
    ResetBgsAndClearDma3BusyFlags(FALSE);
}

TEST("Run setup drafts start with defaults after discard")
{
    RunSetup_Begin();
    RunSetup_SetFullCompatibility(TRUE);
    EXPECT(RunSetup_GetFullCompatibility());

    RunSetup_Discard();
    RunSetup_Begin();
    EXPECT(!RunSetup_GetFullCompatibility());
    RunSetup_Discard();
}

TEST("Run setup confirmation Back retains the draft")
{
    RunSetup_Begin();
    RunSetup_SetFullCompatibility(TRUE);
    RunSetup_EnterConfirmation();
    RunSetup_ReturnToDraft();

    EXPECT(RunSetup_GetFullCompatibility());
    RunSetup_Discard();
}

TEST("Run setup draft does not change the loaded save")
{
    FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    RunSetup_Begin();
    EXPECT(!RunSetup_GetFullCompatibility());
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();

    EXPECT(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY));
    RunSetup_Discard();
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
}

TEST("Run setup applies a confirmed draft to the new save")
{
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    RunSetup_Begin();
    RunSetup_SetFullCompatibility(TRUE);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();

    EXPECT(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY));

    InitEventData();
    RunSetup_ApplyToNewGame();
    EXPECT(!FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY));
}

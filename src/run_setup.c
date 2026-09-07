#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "run_setup.h"
#include "constants/flags.h"

enum RunSetupState
{
    RUN_SETUP_INACTIVE,
    RUN_SETUP_DRAFT,
    RUN_SETUP_CONFIRMATION,
    RUN_SETUP_CONFIRMED,
};

struct RunSetupDraft
{
    bool8 fullCompatibility;
    bool8 reusableTMs;
    enum RunSetupState state:8;
};

static EWRAM_DATA struct RunSetupDraft sRunSetupDraft = {0};

void RunSetup_Begin(void)
{
    sRunSetupDraft.fullCompatibility = FALSE;
    sRunSetupDraft.reusableTMs = FALSE;
    sRunSetupDraft.state = RUN_SETUP_DRAFT;
}

void RunSetup_Discard(void)
{
    memset(&sRunSetupDraft, 0, sizeof(sRunSetupDraft));
}

void RunSetup_EnterConfirmation(void)
{
    if (sRunSetupDraft.state == RUN_SETUP_DRAFT)
        sRunSetupDraft.state = RUN_SETUP_CONFIRMATION;
}

void RunSetup_ReturnToDraft(void)
{
    if (sRunSetupDraft.state == RUN_SETUP_CONFIRMATION)
        sRunSetupDraft.state = RUN_SETUP_DRAFT;
}

void RunSetup_Confirm(void)
{
    if (sRunSetupDraft.state == RUN_SETUP_CONFIRMATION)
        sRunSetupDraft.state = RUN_SETUP_CONFIRMED;
}

void RunSetup_ApplyToNewGame(void)
{
#if IS_FRLG
    RunSetup_Discard();
#else
    if (sRunSetupDraft.state == RUN_SETUP_CONFIRMED && sRunSetupDraft.fullCompatibility)
        FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    else
        FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);

    if (sRunSetupDraft.state == RUN_SETUP_CONFIRMED && sRunSetupDraft.reusableTMs)
        FlagSet(FLAG_RUN_RULE_REUSABLE_TMS);
    else
        FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);

    RunSetup_Discard();
#endif
}

void RunSetup_PrepareDisplay(void)
{
    UnsetBgTilemapBuffer(0);
    ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_WIN1_ON | DISPCNT_OBJWIN_ON);
    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);
}

void RunSetup_ClearDisplayTilemap(void)
{
    FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, DISPLAY_TILE_WIDTH, DISPLAY_TILE_HEIGHT);
}

bool32 RunSetup_GetFullCompatibility(void)
{
    return sRunSetupDraft.fullCompatibility;
}

void RunSetup_SetFullCompatibility(bool32 enabled)
{
    if (sRunSetupDraft.state == RUN_SETUP_DRAFT)
        sRunSetupDraft.fullCompatibility = enabled;
}

bool32 RunSetup_GetReusableTMs(void)
{
    return sRunSetupDraft.reusableTMs;
}

void RunSetup_SetReusableTMs(bool32 enabled)
{
    if (sRunSetupDraft.state == RUN_SETUP_DRAFT)
        sRunSetupDraft.reusableTMs = enabled;
}

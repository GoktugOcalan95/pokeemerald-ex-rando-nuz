#ifndef GUARD_RUN_SETUP_H
#define GUARD_RUN_SETUP_H

#define RUN_SETUP_VISIBLE_ROWS 4

enum RunSetupPreset
{
    RUN_SETUP_PRESET_VANILLA,
    RUN_SETUP_PRESET_NUZLOCKE,
    RUN_SETUP_PRESET_BISHEY,
    RUN_SETUP_PRESET_COUNT,
    RUN_SETUP_PRESET_CUSTOM = RUN_SETUP_PRESET_COUNT,
};

void RunSetup_SetPreset(enum RunSetupPreset preset);
enum RunSetupPreset RunSetup_GetPreset(void);

u32 RunSetup_GetScrollTop(u32 selection, u32 top, u32 count);
void RunSetup_Begin(void);
void RunSetup_Discard(void);
void RunSetup_EnterConfirmation(void);
void RunSetup_ReturnToDraft(void);
void RunSetup_Confirm(void);
void RunSetup_ApplyToNewGame(void);
void RunSetup_PrepareDisplay(void);
void RunSetup_ClearDisplayTilemap(void);
void RunSetup_ClearDisplayGraphics(void);
bool32 RunSetup_GetFullCompatibility(void);
void RunSetup_SetFullCompatibility(bool32 enabled);

bool32 RunSetup_GetReusableTMs(void);
void RunSetup_SetReusableTMs(bool32 enabled);

bool32 RunSetup_GetNoEVGain(void);
void RunSetup_SetNoEVGain(bool32 enabled);

bool32 RunSetup_GetOpponentHPPercentage(void);
void RunSetup_SetOpponentHPPercentage(bool32 enabled);

bool32 RunSetup_GetLevelCaps(void);
void RunSetup_SetLevelCaps(bool32 enabled);

bool32 RunSetup_GetFrostbite(void);
void RunSetup_SetFrostbite(bool32 enabled);

bool32 RunSetup_GetSetupMovePP(void);
void RunSetup_SetSetupMovePP(bool32 enabled);

bool32 RunSetup_GetInstantCatch(void);
void RunSetup_SetInstantCatch(bool32 enabled);

#endif // GUARD_RUN_SETUP_H

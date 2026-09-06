#ifndef GUARD_RUN_SETUP_H
#define GUARD_RUN_SETUP_H

void RunSetup_Begin(void);
void RunSetup_Discard(void);
void RunSetup_EnterConfirmation(void);
void RunSetup_ReturnToDraft(void);
void RunSetup_Confirm(void);
void RunSetup_ApplyToNewGame(void);
void RunSetup_PrepareDisplay(void);
void RunSetup_ClearDisplayTilemap(void);
bool32 RunSetup_GetFullCompatibility(void);
void RunSetup_SetFullCompatibility(bool32 enabled);

#endif // GUARD_RUN_SETUP_H

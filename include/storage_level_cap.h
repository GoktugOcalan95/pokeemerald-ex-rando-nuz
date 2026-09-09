#ifndef GUARD_STORAGE_LEVEL_CAP_H
#define GUARD_STORAGE_LEVEL_CAP_H

struct Pokemon;
struct BoxPokemon;

bool32 StorageLevelCap_Begin(bool32 fromBox, u32 boxId, u32 position);
void StorageLevelCap_Finish(void);
u32 StorageLevelCap_GetPartySlot(void);
bool32 StorageLevelCap_IsBoxed(void);
struct Pokemon *GetEvolutionPartyMon(u32 slot);
struct BoxPokemon *StorageLevelCap_GetSplitEvolutionSlot(void);

#endif // GUARD_STORAGE_LEVEL_CAP_H

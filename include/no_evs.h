#ifndef GUARD_NO_EVS_H
#define GUARD_NO_EVS_H

struct Pokemon;
struct BoxPokemon;

bool32 NormalizeBoxMonEVs(struct BoxPokemon *mon);
void NormalizeMonEVs(struct Pokemon *mon);
void NormalizeStoredMonEVs(void);
bool32 IsEVRelatedItem(u16 item);
bool32 IsItemAllowedByNoEVs(u16 item);

#endif

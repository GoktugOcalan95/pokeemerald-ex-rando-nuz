#ifndef GUARD_ABILITY_RANDOMIZER_H
#define GUARD_ABILITY_RANDOMIZER_H

struct Pokemon;

u16 GetRandomizedSpeciesAbility(u16 species, u32 slot);
bool32 IsRandomizerAbilityAllowed(u16 ability);
u32 GetNativeFormAbilitySlots(u16 species);
u32 GetAbilityItemTargetSlot(struct Pokemon *mon, u16 item);

#endif

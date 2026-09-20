#ifndef GUARD_SPECIES_RANDOMIZER_H
#define GUARD_SPECIES_RANDOMIZER_H

struct Pokemon;
struct ScriptContext;
struct WildPokemon;

#include "constants/species_randomizer.h"

u16 GetBirchRescueSpecies(void);
void SetBirchChaseGraphics(void);
u16 GetRandomizerSpeciesFamily(u16 species);
bool32 IsRandomizerSpeciesLegendary(u16 species);
bool32 IsRandomizerSpeciesEligible(u16 species);
u16 PickRandomizerSpecies(bool32 legendary, u32 domain, u32 source, u32 slot, u16 excludedFamily1, u16 excludedFamily2);
u16 RandomizeEncounterSpecies(u16 original, u32 domain, u32 source, u32 slot);
u16 RandomizeWildSlot(const struct WildPokemon *table, u32 slot);
u16 RandomizeStarterSpecies(u16 original, u32 slot);
void PrepareRandomizedEncounterMon(struct Pokemon *mon);
void RandomizeSpeciesFromScript(struct ScriptContext *ctx);

#endif

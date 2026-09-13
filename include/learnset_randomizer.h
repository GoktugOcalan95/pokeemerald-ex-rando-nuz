#ifndef GUARD_LEARNSET_RANDOMIZER_H
#define GUARD_LEARNSET_RANDOMIZER_H

struct LevelUpMove;
// Returned tables remain valid until four other species are requested.
const struct LevelUpMove *GetRandomizedLearnset(u16 species, const struct LevelUpMove *original);

#endif

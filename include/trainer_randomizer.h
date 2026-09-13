#ifndef GUARD_TRAINER_RANDOMIZER_H
#define GUARD_TRAINER_RANDOMIZER_H

struct Pokemon;
struct Trainer;
const struct Trainer *GetRunTrainer(u16 trainerId);
bool32 IsRunTrainerBattle(u16 trainerId);
bool32 CreateRunTrainerParty(struct Pokemon *party, u16 trainerId);
u16 GetRunTrainerSpecies(u16 trainerId, u32 slot, u16 original);

#endif

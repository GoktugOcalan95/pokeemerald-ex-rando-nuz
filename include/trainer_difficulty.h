#ifndef GUARD_TRAINER_DIFFICULTY_H
#define GUARD_TRAINER_DIFFICULTY_H

enum RunTrainerDifficulty
{
    RUN_TRAINER_NORMAL,
    RUN_TRAINER_HARD,
    RUN_TRAINER_UNFAIR,
};

bool32 UsesRunTrainerKnowledge(void);
u64 GetRunTrainerAIFlags(u16 trainerId, u64 authored);
u32 GetRunTrainerDifficulty(void);
bool32 IsRunTrainerBoss(u16 trainerId);
u32 GetRunTrainerPartySize(u16 trainerId, u32 originalCount);
u32 GetRunTrainerLevel(u16 trainerId, u32 level);
void GetRunTrainerSpecialSlots(u16 trainerId, u32 count, u32 protectedSlot, u32 *legendarySlot, u32 *megaSlot);
u16 PickRunTrainerMega(u16 trainerId, u32 slot, u16 *stone);

#endif

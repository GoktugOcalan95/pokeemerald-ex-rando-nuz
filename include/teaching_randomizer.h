#ifndef GUARD_TEACHING_RANDOMIZER_H
#define GUARD_TEACHING_RANDOMIZER_H

struct ScriptContext;

u16 GetRandomizedMachineMove(u32 index);
u16 GetTutorMove(u32 index);
u16 GetRandomizedTutorMove(u16 original);
u16 GetOriginalTeachingMove(u16 assigned);
u16 GetRandomizedTeachingMove(u16 original);
u16 GetFrontierTutorMove(u32 tutor, u32 index);
const u8 *GetRandomizedMoveDescription(u16 move, u32 width);
bool8 ScriptRandomizeTutorMove(struct ScriptContext *ctx);

#endif

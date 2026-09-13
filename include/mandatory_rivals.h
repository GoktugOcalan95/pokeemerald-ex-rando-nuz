#ifndef GUARD_MANDATORY_RIVALS_H
#define GUARD_MANDATORY_RIVALS_H

struct ScriptContext;
bool32 HasWonMandatoryRival(u32 rival);
u32 GetMandatoryRivalBoundary(u16 map, s16 fromX, s16 fromY, s16 toX, s16 toY);
bool32 TryStartMandatoryRivalBoundaryScript(u32 direction);
bool8 MandatoryRivals_SelectLilycoveRival(struct ScriptContext *ctx);
bool8 MandatoryRivals_GetLilycoveSprite(struct ScriptContext *ctx);

#endif

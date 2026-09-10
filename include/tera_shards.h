#ifndef GUARD_TERA_SHARDS_H
#define GUARD_TERA_SHARDS_H

struct BoxPokemon;
struct Pokemon;
bool32 CanApplyTeraShard(struct BoxPokemon *mon, u16 item);
bool32 ApplyTeraShard(struct Pokemon *mon, u16 item);

#endif

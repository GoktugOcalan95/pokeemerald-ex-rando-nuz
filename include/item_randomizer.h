#ifndef GUARD_ITEM_RANDOMIZER_H
#define GUARD_ITEM_RANDOMIZER_H

struct ScriptContext;
struct Pokemon;

enum ItemRewardDomain
{
    ITEM_REWARD_PICKUP = 1,
    ITEM_REWARD_HIDDEN,
    ITEM_REWARD_GIFT,
    ITEM_REWARD_PC,
    ITEM_REWARD_WILD_HELD,
    ITEM_REWARD_TRAINER_HELD,
    ITEM_REWARD_GIFT_HELD,
    ITEM_REWARD_TRADE_HELD,
    ITEM_REWARD_FACILITY_HELD,
    ITEM_REWARD_PRIZE,
};

enum ItemPrizeSource
{
    ITEM_PRIZE_ARENA,
    ITEM_PRIZE_PALACE,
    ITEM_PRIZE_PYRAMID,
    ITEM_PRIZE_VERDANTURF_TENT,
    ITEM_PRIZE_FALLARBOR_TENT,
    ITEM_PRIZE_SLATEPORT_TENT,
    ITEM_PRIZE_TRAINER_HILL,
    ITEM_PRIZE_TRAINER_TOWER,
    ITEM_PRIZE_POKEMON_JUMP,
    ITEM_PRIZE_DODRIO,
};

bool32 IsRandomizedRewardItemAllowed(u16 item);
bool32 IsRandomizedLootItemAllowed(u16 item);
u16 RandomizeItemReward(u16 original, u32 domain, u32 source, u32 slot);
void RandomizePickupFromScript(void);
void RandomizeHiddenItemFromScript(void);
void RandomizeGiftFromScript(struct ScriptContext *ctx);
void RandomizeFreeGiftFromScript(struct ScriptContext *ctx);

bool32 AddAuthoredItemReward(u16 original, u16 item, u16 count);

#endif

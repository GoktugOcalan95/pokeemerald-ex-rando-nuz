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
};

bool32 IsRandomizedRewardItemAllowed(u16 item);
u16 RandomizeItemReward(u16 original, u32 domain, u32 source, u32 slot);
void RandomizePickupFromScript(void);
void RandomizeHiddenItemFromScript(void);
void RandomizeGiftFromScript(struct ScriptContext *ctx);

#endif

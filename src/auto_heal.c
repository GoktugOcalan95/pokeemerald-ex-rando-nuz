#include "global.h"
#include "auto_heal.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "constants/flags.h"

void AutoHealAfterBattle(void)
{
    if (!FlagGet(FLAG_RUN_RULE_AUTO_HEAL)
     || (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_RECORDED
                           | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_TRAINER_HILL | BATTLE_TYPE_SAFARI
                           | BATTLE_TYPE_CATCH_TUTORIAL))
     || gBattleOutcome == B_OUTCOME_LOST || gBattleOutcome == B_OUTCOME_DREW
     || gBattleOutcome == B_OUTCOME_FORFEITED)
        return;

    for (u32 i = 0; i < gPartiesCount[B_TRAINER_PLAYER]; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(mon, MON_DATA_IS_EGG) && GetMonData(mon, MON_DATA_HP) != 0)
            HealPokemon(mon);
    }

    if (CheckBagHasItem(ITEM_TERA_ORB, 1))
        FlagSet(B_FLAG_TERA_ORB_CHARGED);
}

#include "global.h"
#include "enemy_stab.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "pokemon.h"
#include "run_randomizer.h"
#include "species_randomizer.h"
#include "trainer_randomizer.h"
#include "trainer_difficulty.h"
#include "trainer_util.h"
#include "constants/flags.h"

const struct Trainer *GetRunTrainer(u16 trainerId)
{
#if TESTING
    extern const struct Trainer gRunTrainerTestData[DIFFICULTY_COUNT][TRAINERS_COUNT];
    return &gRunTrainerTestData[DIFFICULTY_NORMAL][trainerId];
#else
    return &gTrainers[DIFFICULTY_NORMAL][trainerId];
#endif
}

#define TRAINER_SPECIES_DOMAIN 0x500
#define RIVAL_EVOLUTION_DOMAIN 0x501

static EWRAM_DATA u8 sMegaSlots[MAX_BATTLE_TRAINERS] = {0};
static EWRAM_DATA u16 sGeneratedTrainerIds[MAX_BATTLE_TRAINERS] = {0};

bool32 CanRunTrainerSlotMegaEvolve(u32 trainer, u32 slot)
{
    if ((trainer != B_TRAINER_OPPONENT_A && trainer != B_TRAINER_OPPONENT_B) || !FlagGet(FLAG_RUN_RULE_TRAINERS))
        return TRUE;
    u16 id = trainer == B_TRAINER_OPPONENT_A ? TRAINER_BATTLE_PARAM.opponentA : TRAINER_BATTLE_PARAM.opponentB;
    if (!IsRunTrainerBattle(id))
        return TRUE;
    return sGeneratedTrainerIds[trainer] == id && sMegaSlots[trainer] == slot;
}

bool32 IsRunTrainerBattle(u16 trainerId)
{
    return trainerId > TRAINER_NONE && trainerId < TRAINERS_COUNT && !gIsDebugBattle
        && (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
        && !(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_CATCH_TUTORIAL
            | BATTLE_TYPE_EREADER_TRAINER | BATTLE_TYPE_TRAINER_HILL | BATTLE_TYPE_SECRET_BASE
            | BATTLE_TYPE_RECORDED));
}

u16 GetRunTrainerSpecies(u16 trainerId, u32 slot, u16 original)
{
    if (!FlagGet(FLAG_RUN_RULE_TRAINERS))
        return original;
    const struct Trainer *trainer = GetRunTrainer(trainerId);
    if (slot < trainer->partySize && trainer->trainerClass == TRAINER_CLASS_RIVAL
        && (trainer->trainerPic == TRAINER_PIC_BRENDAN || trainer->trainerPic == TRAINER_PIC_MAY))
    {
        u32 starter = 0;
        u32 stage = 0;
        switch (original)
        {
        case SPECIES_TREECKO: starter = 0; stage = 0; break;
        case SPECIES_GROVYLE: starter = 0; stage = 1; break;
        case SPECIES_SCEPTILE: starter = 0; stage = 2; break;
        case SPECIES_TORCHIC: starter = 1; stage = 0; break;
        case SPECIES_COMBUSKEN: starter = 1; stage = 1; break;
        case SPECIES_BLAZIKEN: starter = 1; stage = 2; break;
        case SPECIES_MUDKIP: starter = 2; stage = 0; break;
        case SPECIES_MARSHTOMP: starter = 2; stage = 1; break;
        case SPECIES_SWAMPERT: starter = 2; stage = 2; break;
        default: starter = 3; break;
        }
        if (starter < 3)
        {
            const u16 bases[] = {SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP};
            u16 species = RandomizeStarterSpecies(bases[starter], starter);
            for (u32 step = 0; step < stage; step++)
            {
                const struct Evolution *evolutions = GetSpeciesEvolutions(species);
                u32 count = 0;
                if (evolutions == NULL)
                    break;
                for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
                    count += IsRandomizerSpeciesEligible(evolutions[i].targetSpecies);
                if (!count)
                    break;
                u32 choice = RunRandomizerHash(RIVAL_EVOLUTION_DOMAIN, starter, step) % count;
                for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
                    if (IsRandomizerSpeciesEligible(evolutions[i].targetSpecies) && choice-- == 0)
                    {
                        species = evolutions[i].targetSpecies;
                        break;
                    }
            }
            return species;
        }
    }
    return PickRandomizerSpecies(FALSE, TRAINER_SPECIES_DOMAIN, trainerId, slot, SPECIES_NONE, SPECIES_NONE);
}

bool32 CreateRunTrainerParty(struct Pokemon *party, u16 trainerId)
{
    if (!IsRunTrainerBattle(trainerId) || (!FlagGet(FLAG_RUN_RULE_TRAINERS) && GetRunTrainerDifficulty() == RUN_TRAINER_NORMAL))
        return FALSE;
    const struct Trainer *trainer = GetRunTrainer(trainerId);
    const struct Trainer *source = trainer->overrideTrainer ? GetRunTrainer(trainer->overrideTrainer) : trainer;
    u32 originalCount = trainer->partySize ? trainer->partySize : source->partySize;
    if (!originalCount)
        return FALSE;
    u32 count = GetRunTrainerPartySize(trainerId, originalCount);
    u32 lowest = 0;
    u32 starterSlot = PARTY_SIZE;
    for (u32 i = 0; i < originalCount; i++)
    {
        if (source->party[i].lvl < source->party[lowest].lvl)
            lowest = i;
        if (trainer->trainerClass == TRAINER_CLASS_RIVAL
            && (trainer->trainerPic == TRAINER_PIC_BRENDAN || trainer->trainerPic == TRAINER_PIC_MAY))
        {
            u32 species = source->party[i].species;
            if (species == SPECIES_TREECKO || species == SPECIES_GROVYLE || species == SPECIES_SCEPTILE
                || species == SPECIES_TORCHIC || species == SPECIES_COMBUSKEN || species == SPECIES_BLAZIKEN
                || species == SPECIES_MUDKIP || species == SPECIES_MARSHTOMP || species == SPECIES_SWAMPERT)
                starterSlot = i;
        }
    }
    u32 protectedSlot = starterSlot == PARTY_SIZE ? PARTY_SIZE : min(starterSlot, count - 1);
    u32 legendarySlot, megaSlot;
    GetRunTrainerSpecialSlots(trainerId, count, protectedSlot, &legendarySlot, &megaSlot);
    for (u32 i = 0; i < MAX_BATTLE_TRAINERS; i++)
        if (party == gParties[i])
        {
            sMegaSlots[i] = megaSlot;
            sGeneratedTrainerIds[i] = trainerId;
        }
    struct TrainerGenerator generator = {0};
    MakeTrainerGenerator(&generator, trainer);
    generator.randomizerSource = (u32)trainer;
    if (FlagGet(FLAG_RUN_RULE_TRAINERS))
        generator.smartTera = FALSE;
    ZeroPartyMons(party);
    for (u32 i = 0; i < count; i++)
    {
        u32 originalSlot = i < originalCount ? i : lowest;
        if (i == protectedSlot)
            originalSlot = starterSlot;
        struct TrainerMon entry = source->party[originalSlot];
        u16 megaStone = ITEM_NONE;
        entry.lvl = GetRunTrainerLevel(trainerId, entry.lvl);
        if (GetRunTrainerDifficulty() != RUN_TRAINER_NORMAL)
            entry.iv = TRAINER_PARTY_IVS(31, 31, 31, 31, 31, 31);
        if (FlagGet(FLAG_RUN_RULE_TRAINERS))
        {
            entry.species = GetRunTrainerSpecies(trainerId, i == protectedSlot ? originalSlot : i, entry.species);
            if (i == legendarySlot)
                entry.species = PickRandomizerSpecies(TRUE, 0x513, trainerId, i, SPECIES_NONE, SPECIES_NONE);
            if (i == megaSlot)
            {
                u16 species = PickRunTrainerMega(trainerId, i, &megaStone);
                if (species != SPECIES_NONE)
                    entry.species = species;
            }
            entry.ability = ABILITY_NONE;
            entry.gender = TRAINER_MON_RANDOM_GENDER;
            entry.nickname = NULL;
            entry.teraType = TYPE_MYSTERY;
            entry.shouldUseDynamax = FALSE;
            entry.gigantamaxFactor = FALSE;
            memset(entry.moves, 0, sizeof(entry.moves));
        }
        GenerateMonFromTrainerMon(&party[i], &entry, &generator);
        EnsureEnemyStabMoves(&party[i], trainerId, i);
        if (megaStone != ITEM_NONE)
            SetMonData(&party[i], MON_DATA_HELD_ITEM, &megaStone);
    }
    return TRUE;
}

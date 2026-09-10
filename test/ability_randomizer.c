#include "global.h"
#include "ability_randomizer.h"
#include "battle.h"
#include "data.h"
#include "event_data.h"
#include "pokemon.h"
#include "random.h"
#include "save.h"
#include "species_randomizer.h"
#include "trainer_util.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/form_change_types.h"

TEST("Ability randomizer preserves ordinary family relationships and native three-option slots")
{
    gTestRunnerState.timeoutSeconds = 120;
    FlagSet(FLAG_RUN_RULE_ABILITIES);
    for (u32 family = 1; family < NUM_SPECIES; family++)
    {
        if (GetRandomizerSpeciesFamily(family) != family)
            continue;
        u16 mapping[ABILITIES_COUNT] = {0};
        bool8 used[ABILITIES_COUNT] = {0};
        for (u32 species = family; species < NUM_SPECIES; species++)
        {
            if (GetRandomizerSpeciesFamily(species) != family)
                continue;
            u32 native = GetNativeFormAbilitySlots(species);
            for (u32 slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
            {
                u32 ability = GetSpeciesAbility(species, slot);
                u32 original = gSpeciesInfo[species].abilities[slot];
                if (native)
                {
                    EXPECT_NE(ability, ABILITY_NONE);
                    for (u32 other = 0; other < slot; other++)
                        EXPECT_NE(ability, GetSpeciesAbility(species, other));
                    if (native & (1 << slot))
                        EXPECT_EQ(ability, original ?: gSpeciesInfo[species].abilities[0]);
                    else
                        EXPECT(IsRandomizerAbilityAllowed(ability));
                    continue;
                }
                if (original == ABILITY_NONE)
                {
                    EXPECT_EQ(ability, ABILITY_NONE);
                    continue;
                }
                EXPECT(IsRandomizerAbilityAllowed(ability));
                if (mapping[original])
                    EXPECT_EQ(ability, mapping[original]);
                else
                {
                    EXPECT(!used[ability]);
                    mapping[original] = ability;
                    used[ability] = TRUE;
                }
            }
        }
    }
    EXPECT_EQ(GetNativeFormAbilitySlots(SPECIES_DARUMAKA), 0);
    EXPECT_EQ(GetNativeFormAbilitySlots(SPECIES_DARMANITAN_STANDARD), 4);
    EXPECT_EQ(GetNativeFormAbilitySlots(SPECIES_DARMANITAN_ZEN), 4);
    EXPECT_EQ(GetNativeFormAbilitySlots(SPECIES_PALAFIN_ZERO), 1);
    EXPECT_EQ(GetNativeFormAbilitySlots(SPECIES_PALAFIN_HERO), 1);
    EXPECT_EQ(GetNativeFormAbilitySlots(SPECIES_TERAPAGOS), 1);
    EXPECT_EQ(GetNativeFormAbilitySlots(SPECIES_TERAPAGOS_TERASTAL), 1);
    EXPECT_EQ(GetNativeFormAbilitySlots(SPECIES_TERAPAGOS_STELLAR), 1);
    EXPECT_EQ(GetNativeFormAbilitySlots(SPECIES_GRENINJA_BATTLE_BOND), B_BATTLE_BOND < GEN_9 ? 1 : 0);
    EXPECT_EQ(GetNativeFormAbilitySlots(SPECIES_VENUSAUR_MEGA), 0);
}

TEST("Ability randomizer excludes detrimental and dependent abilities while retaining traps and generic signatures")
{
    const u16 banned[] = {ABILITY_NONE, ABILITY_WONDER_GUARD, ABILITY_TRUANT, ABILITY_SLOW_START,
        ABILITY_DEFEATIST, ABILITY_BATTLE_BOND, ABILITY_COMMANDER, ABILITY_ZERO_TO_HERO,
        ABILITY_FORECAST, ABILITY_SHIELDS_DOWN, ABILITY_TERA_SHIFT, ABILITY_TERA_SHELL,
        ABILITY_TERAFORM_ZERO, ABILITY_EMBODY_ASPECT_TEAL_MASK};
    const u16 allowed[] = {ABILITY_ARENA_TRAP, ABILITY_SHADOW_TAG, ABILITY_MAGNET_PULL,
        ABILITY_COMATOSE, ABILITY_GORILLA_TACTICS, ABILITY_ILLUSION, ABILITY_IMPOSTER,
        ABILITY_AS_ONE_ICE_RIDER, ABILITY_AS_ONE_SHADOW_RIDER};
    for (u32 i = 0; i < ARRAY_COUNT(banned); i++)
        EXPECT(!IsRandomizerAbilityAllowed(banned[i]));
    for (u32 i = 0; i < ARRAY_COUNT(allowed); i++)
        EXPECT(IsRandomizerAbilityAllowed(allowed[i]));
    for (u32 ability = 1; ability < ABILITIES_COUNT; ability++)
        if (gAbilitiesInfo[ability].aiRating < 0)
            EXPECT(!IsRandomizerAbilityAllowed(ability));
}

TEST("Ability randomizer assignments survive seed changes and save load without gameplay RNG")
{
    const u16 species[] = {SPECIES_EEVEE, SPECIES_VAPOREON, SPECIES_MEOWTH_ALOLA, SPECIES_PERRSERKER, SPECIES_DARMANITAN_ZEN};
    u16 expected[ARRAY_COUNT(species)][NUM_ABILITY_SLOTS];
    FlagSet(FLAG_RUN_RULE_ABILITIES);
    for (u32 i = 0; i < ARRAY_COUNT(species); i++)
        for (u32 slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
            expected[i][slot] = GetSpeciesAbility(species[i], slot);
    SeedRng(123);
    u32 next = Random();
    SeedRng(123);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    gSaveBlock2Ptr->playerTrainerId[3] ^= 0x80;
    bool32 changed = FALSE;
    for (u32 i = 0; i < ARRAY_COUNT(species); i++)
        changed |= GetSpeciesAbility(species[i], 0) != expected[i][0];
    EXPECT(changed);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    for (s32 i = ARRAY_COUNT(species) - 1; i >= 0; i--)
        for (u32 slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
            EXPECT_EQ(GetSpeciesAbility(species[i], slot), expected[i][slot]);
    EXPECT_EQ(Random(), next);
    FlagClear(FLAG_RUN_RULE_ABILITIES);
    for (u32 i = 1; i < NUM_SPECIES; i++)
        if (gSpeciesInfo[i].baseHP)
            for (u32 slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
                EXPECT_EQ(GetSpeciesAbility(i, slot), gSpeciesInfo[i].abilities[slot]);
}

TEST("Ability randomizer Capsules and Patches reach all three native-mechanic choices")
{
    struct Pokemon mon;
    FlagSet(FLAG_RUN_RULE_ABILITIES);
    CreateMon(&mon, SPECIES_PALAFIN_ZERO, 40, 123, OTID_STRUCT_PLAYER_ID);
    u32 slot = 0;
    SetMonData(&mon, MON_DATA_ABILITY_NUM, &slot);
    EXPECT_EQ(GetAbilityItemTargetSlot(&mon, ITEM_ABILITY_CAPSULE), 1);
    EXPECT_EQ(GetAbilityItemTargetSlot(&mon, ITEM_ABILITY_PATCH), 2);
    slot = 1;
    SetMonData(&mon, MON_DATA_ABILITY_NUM, &slot);
    EXPECT_EQ(GetAbilityItemTargetSlot(&mon, ITEM_ABILITY_CAPSULE), 0);
    EXPECT_EQ(GetAbilityItemTargetSlot(&mon, ITEM_ABILITY_PATCH), 2);
    slot = 2;
    SetMonData(&mon, MON_DATA_ABILITY_NUM, &slot);
    EXPECT_EQ(GetAbilityItemTargetSlot(&mon, ITEM_ABILITY_CAPSULE), NUM_ABILITY_SLOTS);
    EXPECT_EQ(GetAbilityItemTargetSlot(&mon, ITEM_ABILITY_PATCH), 0);
    bool32 isEgg = TRUE;
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);
    EXPECT_EQ(GetAbilityItemTargetSlot(&mon, ITEM_ABILITY_PATCH), NUM_ABILITY_SLOTS);
    FlagClear(FLAG_RUN_RULE_ABILITIES);
    CreateMon(&mon, SPECIES_PALAFIN_ZERO, 40, 123, OTID_STRUCT_PLAYER_ID);
    EXPECT_EQ(GetAbilityItemTargetSlot(&mon, ITEM_ABILITY_CAPSULE), NUM_ABILITY_SLOTS);
    EXPECT_EQ(GetAbilityItemTargetSlot(&mon, ITEM_ABILITY_PATCH), NUM_ABILITY_SLOTS);
}

TEST("Ability randomizer authored trainer abilities still select their original slots")
{
    const struct TrainerMon source = {.species = SPECIES_BULBASAUR, .lvl = 20, .gender = TRAINER_MON_RANDOM_GENDER, .ability = ABILITY_CHLOROPHYLL};
    struct TrainerGenerator generator = {.otID = OTID_STRUCT_PLAYER_ID};
    struct Pokemon mon;
    generator.localRngState = LocalRandomSeed(123);
    FlagSet(FLAG_RUN_RULE_ABILITIES);
    GenerateMonFromTrainerMon(&mon, &source, &generator);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ABILITY_NUM), 2);
    EXPECT_EQ(GetMonAbility(&mon), GetSpeciesAbility(SPECIES_BULBASAUR, 2));
    u16 evolved = SPECIES_VENUSAUR;
    SetMonData(&mon, MON_DATA_SPECIES, &evolved);
    EXPECT_EQ(GetMonAbility(&mon), GetSpeciesAbility(SPECIES_VENUSAUR, 2));
    EXPECT_EQ(source.ability, ABILITY_CHLOROPHYLL);
}

TEST("Ability randomizer form upgrades require native mechanics while cleanup stays unconditional")
{
    struct FormChangeContext ctx = {.currentSpecies = SPECIES_PALAFIN_ZERO, .method = FORM_CHANGE_BATTLE_SWITCH_OUT, .ability = ABILITY_RUN_AWAY};
    EXPECT_EQ(GetFormChangeTargetSpecies_Internal(ctx), ctx.currentSpecies);
    ctx.ability = ABILITY_ZERO_TO_HERO;
    EXPECT_EQ(GetFormChangeTargetSpecies_Internal(ctx), SPECIES_PALAFIN_HERO);
    ctx.currentSpecies = SPECIES_PALAFIN_HERO;
    ctx.method = FORM_CHANGE_END_BATTLE;
    ctx.ability = ABILITY_RUN_AWAY;
    EXPECT_EQ(GetFormChangeTargetSpecies_Internal(ctx), SPECIES_PALAFIN_ZERO);
    ctx.currentSpecies = SPECIES_MINIOR_CORE_BLUE;
    ctx.method = FORM_CHANGE_BEGIN_WILD_ENCOUNTER;
    EXPECT_EQ(GetFormChangeTargetSpecies_Internal(ctx), ctx.currentSpecies);
    ctx.ability = ABILITY_SHIELDS_DOWN;
    EXPECT_EQ(GetFormChangeTargetSpecies_Internal(ctx), SPECIES_MINIOR_METEOR_BLUE);
    ctx.currentSpecies = SPECIES_TERAPAGOS_TERASTAL;
    ctx.method = FORM_CHANGE_BATTLE_TERASTALLIZATION;
    ctx.teraType = TYPE_STELLAR;
    ctx.ability = ABILITY_RUN_AWAY;
    EXPECT_EQ(GetFormChangeTargetSpecies_Internal(ctx), ctx.currentSpecies);
    ctx.ability = ABILITY_TERA_SHELL;
    EXPECT_EQ(GetFormChangeTargetSpecies_Internal(ctx), SPECIES_TERAPAGOS_STELLAR);
    ctx.currentSpecies = SPECIES_TERAPAGOS_STELLAR;
    ctx.method = FORM_CHANGE_END_BATTLE;
    ctx.ability = ABILITY_RUN_AWAY;
    EXPECT_EQ(GetFormChangeTargetSpecies_Internal(ctx), SPECIES_TERAPAGOS);
}

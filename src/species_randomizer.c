#include "global.h"
#include "event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/vars.h"
#include "pokemon.h"
#include "event_data.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "run_randomizer.h"
#include "item_randomizer.h"
#include "script.h"
#include "species_randomizer.h"
#include "wild_encounter.h"
#include "constants/flags.h"
#include "constants/form_change_types.h"

static EWRAM_DATA u16 sSpeciesFamilies[NUM_SPECIES] = {0};
static EWRAM_DATA bool8 sFamiliesReady = FALSE;
static EWRAM_DATA u16 sSpeciesPool[NUM_SPECIES] = {0};
static EWRAM_DATA u16 sOrdinaryCount = 0;
static EWRAM_DATA u16 sPoolCount = 0;

static bool32 IsEnabledSpecies(u32 species)
{
    return species > SPECIES_NONE && species < NUM_SPECIES && gSpeciesInfo[species].baseHP != 0;
}

static u32 FindFamily(u32 species)
{
    while (sSpeciesFamilies[species] != species)
    {
        sSpeciesFamilies[species] = sSpeciesFamilies[sSpeciesFamilies[species]];
        species = sSpeciesFamilies[species];
    }
    return species;
}

static void JoinFamilies(u32 first, u32 second)
{
    if (!IsEnabledSpecies(second))
        return;
    first = FindFamily(first);
    second = FindFamily(second);
    sSpeciesFamilies[max(first, second)] = min(first, second);
}

static void InitSpeciesFamilies(void)
{
    for (u32 species = 0; species < NUM_SPECIES; species++)
        sSpeciesFamilies[species] = species;

    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        if (!IsEnabledSpecies(species))
            continue;
        const struct Evolution *evolutions = GetSpeciesEvolutions(species);
        const u16 *forms = GetSpeciesFormTable(species);
        const struct FormChange *changes = GetSpeciesFormChanges(species);
        if (evolutions != NULL)
            for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
                JoinFamilies(species, evolutions[i].targetSpecies);
        if (forms != NULL)
            for (u32 i = 0; forms[i] != FORM_SPECIES_END; i++)
                JoinFamilies(species, forms[i]);
        if (changes != NULL)
            for (u32 i = 0; changes[i].method != FORM_CHANGE_TERMINATOR; i++)
                JoinFamilies(species, changes[i].targetSpecies);
    }
    sFamiliesReady = TRUE;
}

u16 GetRandomizerSpeciesFamily(u16 species)
{
    if (!IsEnabledSpecies(species))
        return SPECIES_NONE;
    if (!sFamiliesReady)
        InitSpeciesFamilies();
    return FindFamily(species);
}

bool32 IsRandomizerSpeciesLegendary(u16 species)
{
    if (!IsEnabledSpecies(species))
        return FALSE;
    return gSpeciesInfo[species].isRestrictedLegendary
        || gSpeciesInfo[species].isSubLegendary
        || gSpeciesInfo[species].isMythical;
}

bool32 IsRandomizerSpeciesEligible(u16 species)
{
    if (!IsEnabledSpecies(species))
        return FALSE;
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    if (info->isMegaEvolution || info->isPrimalReversion || info->isUltraBurst
        || info->isGigantamax || info->isTeraForm)
        return FALSE;

    const struct FormChange *changes = GetSpeciesFormChanges(species);
    if (changes != NULL)
    {
        for (u32 i = 0; changes[i].method != FORM_CHANGE_TERMINATOR; i++)
        {
            if ((changes[i].method == FORM_CHANGE_END_BATTLE || changes[i].method == FORM_CHANGE_FAINT)
                && changes[i].targetSpecies != species)
                return FALSE;
            if (changes[i].method == FORM_CHANGE_MOVE && changes[i].param2 == WHEN_FORGOTTEN
                && changes[i].targetSpecies != species)
                return FALSE;
            // Equipment forms follow the independently selected item after creation.
            if (changes[i].method == FORM_CHANGE_ITEM_HOLD && changes[i].param1 == ITEM_NONE
                && changes[i].targetSpecies != species)
                return FALSE;
        }
    }
    const struct Fusion *fusions = gFusionTablePointers[species];
    if (fusions != NULL)
        for (u32 i = 0; fusions[i].fusionStorageIndex != FUSION_TERMINATOR; i++)
            if (fusions[i].fusingIntoMon == species)
                return FALSE;
    return TRUE;
}

static u32 GetCosmeticGroup(u32 species)
{
    u32 group = gSpeciesInfo[species].cosmeticFormOf;
    return group != SPECIES_NONE ? group : species;
}

static void InitSpeciesPool(void)
{
    for (u32 legendary = 0; legendary < 2; legendary++)
    {
        for (u32 species = 1; species < NUM_SPECIES; species++)
            if (GetCosmeticGroup(species) == species && IsRandomizerSpeciesEligible(species)
                && IsRandomizerSpeciesLegendary(species) == legendary)
                sSpeciesPool[sPoolCount++] = species;
        if (!legendary)
            sOrdinaryCount = sPoolCount;
    }
}

static u16 PickAppearance(u32 species, u32 domain, u32 source, u32 slot)
{
    const u16 *forms = GetSpeciesFormTable(species);
    if (forms == NULL)
        return species;
    u32 count = 0;
    for (u32 i = 0; forms[i] != FORM_SPECIES_END; i++)
        if (GetCosmeticGroup(forms[i]) == species && IsRandomizerSpeciesEligible(forms[i]))
            count++;
    if (count == 0)
        return species;
    u32 choice = RunRandomizerHash(domain ^ 0x80000000, source, slot) % count;
    for (u32 i = 0; forms[i] != FORM_SPECIES_END; i++)
        if (GetCosmeticGroup(forms[i]) == species && IsRandomizerSpeciesEligible(forms[i]) && choice-- == 0)
            return forms[i];
    return species;
}

u16 PickRandomizerSpecies(bool32 legendary, u32 domain, u32 source, u32 slot, u16 excludedFamily1, u16 excludedFamily2)
{
    if (sPoolCount == 0)
        InitSpeciesPool();
    u32 first = legendary ? sOrdinaryCount : 0;
    u32 end = legendary ? sPoolCount : sOrdinaryCount;
    if (excludedFamily1 == SPECIES_NONE && excludedFamily2 == SPECIES_NONE)
    {
        if (first == end)
            return SPECIES_NONE;
        u32 choice = first + RunRandomizerHash(domain, source, slot) % (end - first);
        return PickAppearance(sSpeciesPool[choice], domain, source, slot);
    }
    u32 count = 0;
    for (u32 i = first; i < end; i++)
    {
        u32 family = GetRandomizerSpeciesFamily(sSpeciesPool[i]);
        if (family != excludedFamily1 && family != excludedFamily2)
            count++;
    }
    if (count == 0)
        return SPECIES_NONE;
    u32 choice = RunRandomizerHash(domain, source, slot) % count;
    for (u32 i = first; i < end; i++)
    {
        u32 family = GetRandomizerSpeciesFamily(sSpeciesPool[i]);
        if (family != excludedFamily1 && family != excludedFamily2 && choice-- == 0)
            return PickAppearance(sSpeciesPool[i], domain, source, slot);
    }
    return SPECIES_NONE;
}

u16 RandomizeEncounterSpecies(u16 original, u32 domain, u32 source, u32 slot)
{
    if (!FlagGet(FLAG_RUN_RULE_ENCOUNTERS) || !IsEnabledSpecies(original))
        return original;
    bool32 legendary = (domain == SPECIES_REWARD_STATIC || domain == SPECIES_REWARD_ROAMER)
        && IsRandomizerSpeciesLegendary(original);
    u32 species = PickRandomizerSpecies(legendary, domain, source, slot, SPECIES_NONE, SPECIES_NONE);
    return species != SPECIES_NONE ? species : original;
}

u16 RandomizeWildSlot(const struct WildPokemon *table, u32 slot)
{
    if (InBattlePike() || CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
        return table[slot].species;
    u32 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    return RandomizeEncounterSpecies(table[slot].species, SPECIES_REWARD_WILD, (u32)table, (map << 8) | slot);
}

u16 RandomizeStarterSpecies(u16 original, u32 slot)
{
    if (!FlagGet(FLAG_RUN_RULE_ENCOUNTERS))
        return original;
    u16 families[2] = {SPECIES_NONE, SPECIES_NONE};
    u16 species = original;
    for (u32 i = 0; i <= min(slot, 2); i++)
    {
        species = PickRandomizerSpecies(FALSE, SPECIES_REWARD_STARTER, 0, i, families[0], families[1]);
        if (i < 2)
            families[i] = GetRandomizerSpeciesFamily(species);
    }
    return species != SPECIES_NONE ? species : original;
}

void PrepareRandomizedEncounterMon(struct Pokemon *mon)
{
    if (!FlagGet(FLAG_RUN_RULE_ENCOUNTERS) || InBattlePike()
        || CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
        return;
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    struct FormChangeContext context =
    {
        .method = FORM_CHANGE_ITEM_HOLD,
        .currentSpecies = species,
        .heldItem = GetMonData(mon, MON_DATA_HELD_ITEM),
        .ability = GetMonAbility(mon),
    };
    u16 form = GetFormChangeTargetSpecies_Internal(context);
    context.currentSpecies = form;
    context.method = FORM_CHANGE_MOVE;
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        context.moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);
    form = GetFormChangeTargetSpecies_Internal(context);
    if (IsEnabledSpecies(form) && form != species)
    {
        SetMonData(mon, MON_DATA_SPECIES, &form);
        CalculateMonStats(mon);
    }
    TrySetDayLimitToFormChange(mon);
}

void RandomizeSpeciesFromScript(struct ScriptContext *ctx)
{
    u16 variable = ScriptReadHalfword(ctx);
    u16 original = VarGet(ScriptReadHalfword(ctx));
    u32 source = ScriptReadWord(ctx);
    u32 domain = ScriptReadHalfword(ctx);
    u32 slot = VarGet(ScriptReadHalfword(ctx));
    VarSet(variable, RandomizeEncounterSpecies(original, domain, source, slot));
}

void CreateRandomizedEnemyEventMon(struct ScriptContext *ctx)
{
    u32 source = (u32)ctx->scriptPtr;
    u16 original = VarGet(ScriptReadHalfword(ctx));
    gSpecialVar_0x8004 = RandomizeEncounterSpecies(original, SPECIES_REWARD_STATIC, source, 0);
    gSpecialVar_0x8005 = VarGet(ScriptReadHalfword(ctx));
    gSpecialVar_0x8006 = RandomizeItemReward(VarGet(ScriptReadHalfword(ctx)), ITEM_REWARD_GIFT_HELD, source, 0);
    CreateEnemyEventMon();
    PrepareRandomizedEncounterMon(&gParties[B_TRAINER_OPPONENT_A][0]);
}

u16 GetBirchRescueSpecies(void)
{
    return RandomizeEncounterSpecies(SPECIES_ZIGZAGOON, SPECIES_REWARD_FIRST_BATTLE, 0, 0);
}

void SetBirchChaseGraphics(void)
{
    u16 graphics = OBJ_EVENT_GFX_ZIGZAGOON_1;
    if (FlagGet(FLAG_RUN_RULE_ENCOUNTERS))
        graphics = GetGraphicsIdForMon(GetBirchRescueSpecies(), FALSE, FALSE);
    VarSet(VAR_OBJ_GFX_ID_0, graphics);
}

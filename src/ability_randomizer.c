#include "global.h"
#include "ability_randomizer.h"
#include "event_data.h"
#include "pokemon.h"
#include "constants/characters.h"
#include "run_randomizer.h"
#include "species_randomizer.h"
#include "constants/flags.h"
#include "constants/form_change_types.h"

#define ABILITY_RANDOMIZER_DOMAIN 0x200
#define EXTRA_SLOT_KEY(slot) (ABILITIES_COUNT + (slot))
#define ABILITY_MAPPING_KEYS (ABILITIES_COUNT + NUM_ABILITY_SLOTS)

#define FAMILY_CACHE_COUNT 4

struct FamilyAbilityCache
{
    u16 family;
    u16 mapping[ABILITY_MAPPING_KEYS];
};

static EWRAM_DATA struct FamilyAbilityCache sFamilyCaches[FAMILY_CACHE_COUNT] = {0};
static EWRAM_DATA u16 sAbilityPool[ABILITIES_COUNT] = {0};
static EWRAM_DATA u8 sNativeSlots[(NUM_SPECIES + 1) / 2] = {0};
static EWRAM_DATA u8 sNativeOnlyAbilities[(ABILITIES_COUNT + 7) / 8] = {0};
static EWRAM_DATA u16 sAbilityPoolCount = 0;
static EWRAM_DATA u32 sCachedSeed = 0;
static EWRAM_DATA u8 sNextFamilyCache = 0;
static EWRAM_DATA bool8 sCacheReady = FALSE;
static EWRAM_DATA bool8 sNativeSlotsReady = FALSE;

static u32 ReadNativeSlots(u32 species)
{
    return (sNativeSlots[species / 2] >> ((species % 2) * 4)) & 7;
}

static bool32 MarkNativeSlots(u32 species, u32 slots)
{
    u32 original = ReadNativeSlots(species);
    sNativeSlots[species / 2] |= slots << ((species % 2) * 4);
    return original != ReadNativeSlots(species);
}

static void MarkNativeAbility(u32 ability)
{
    sNativeOnlyAbilities[ability / 8] |= 1 << (ability % 8);
}

static bool32 IsValidSpecies(u32 species)
{
    return species > SPECIES_NONE && species < NUM_SPECIES && gSpeciesInfo[species].baseHP != 0;
}

static u32 GetFormRequiredAbility(const struct FormChange *change)
{
    switch (change->method)
    {
    case FORM_CHANGE_ITEM_HOLD:
    case FORM_CHANGE_BATTLE_WEATHER:
    case FORM_CHANGE_BATTLE_BEFORE_MOVE:
    case FORM_CHANGE_BATTLE_AFTER_MOVE:
    case FORM_CHANGE_BATTLE_BEFORE_MOVE_CATEGORY:
    case FORM_CHANGE_BATTLE_TERASTALLIZATION:
        return change->param2;
    case FORM_CHANGE_BEGIN_WILD_ENCOUNTER:
    case FORM_CHANGE_BATTLE_SWITCH_IN:
    case FORM_CHANGE_BATTLE_SWITCH_OUT:
    case FORM_CHANGE_BATTLE_HP_PERCENT_TURN_END:
    case FORM_CHANGE_BATTLE_HP_PERCENT_SEND_OUT:
    case FORM_CHANGE_BATTLE_HP_PERCENT_DURING_MOVE:
    case FORM_CHANGE_BATTLE_TURN_END:
    case FORM_CHANGE_BATTLE_HIT_BY_MOVE_CATEGORY:
    case FORM_CHANGE_BATTLE_HIT_BY_CONFUSION_SELF_DMG:
        return change->param1;
    case FORM_CHANGE_BATTLE_BOND:
        return B_BATTLE_BOND < GEN_9 ? change->param1 : ABILITY_NONE;
    default:
        return ABILITY_NONE;
    }
}

static u32 GetNativeAbilityAtSlot(u32 species, u32 slot)
{
    u32 ability = gSpeciesInfo[species].abilities[slot];
    if (ability == ABILITY_NONE)
        for (u32 i = 0; i < NUM_ABILITY_SLOTS && ability == ABILITY_NONE; i++)
            ability = gSpeciesInfo[species].abilities[i];
    return ability;
}

static void InitNativeSlots(void)
{
    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        if (!IsValidSpecies(species))
            continue;
        const struct FormChange *changes = GetSpeciesFormChanges(species);
        for (u32 i = 0; changes != NULL && changes[i].method != FORM_CHANGE_TERMINATOR; i++)
        {
            u32 ability = GetFormRequiredAbility(&changes[i]);
            if (ability == ABILITY_NONE || ability >= ABILITIES_COUNT)
                continue;
            MarkNativeAbility(ability);
            for (u32 slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
            {
                if (gSpeciesInfo[species].abilities[slot] != ability)
                    continue;
                MarkNativeSlots(species, 1 << slot);
                if (IsValidSpecies(changes[i].targetSpecies))
                    MarkNativeSlots(changes[i].targetSpecies, 1 << slot);
                break;
            }
        }
    }

    bool32 changed;
    do
    {
        changed = FALSE;
        for (u32 species = 1; species < NUM_SPECIES; species++)
        {
            if (!ReadNativeSlots(species))
                continue;
            const struct FormChange *changes = GetSpeciesFormChanges(species);
            for (u32 i = 0; changes != NULL && changes[i].method != FORM_CHANGE_TERMINATOR; i++)
            {
                u32 target = changes[i].targetSpecies;
                if (!IsValidSpecies(target)
                    || (changes[i].method != FORM_CHANGE_END_BATTLE && changes[i].method != FORM_CHANGE_FAINT
                        && !(changes[i].method == FORM_CHANGE_BATTLE_SWITCH_OUT && changes[i].param1 == ABILITY_NONE)))
                    continue;
                changed |= MarkNativeSlots(target, ReadNativeSlots(species));
            }
        }
    } while (changed);

    for (u32 species = 1; species < NUM_SPECIES; species++)
        for (u32 slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
            if (ReadNativeSlots(species) & (1 << slot))
                MarkNativeAbility(GetNativeAbilityAtSlot(species, slot));
    sNativeSlotsReady = TRUE;
}

u32 GetNativeFormAbilitySlots(u16 species)
{
    if (!IsValidSpecies(species))
        return 0;
    if (!sNativeSlotsReady)
        InitNativeSlots();
    return ReadNativeSlots(species);
}

bool32 IsRandomizerAbilityAllowed(u16 ability)
{
    if (!sNativeSlotsReady)
        InitNativeSlots();
    return ability > ABILITY_NONE && ability < ABILITIES_COUNT
        && gAbilitiesInfo[ability].name[0] != 0 && gAbilitiesInfo[ability].name[0] != EOS
        && !gAbilitiesInfo[ability].randomizerBanned
        && gAbilitiesInfo[ability].aiRating >= 0
        && !(sNativeOnlyAbilities[ability / 8] & (1 << (ability % 8)));
}

static u32 PickAbility(u32 family, u32 key, bool8 *used)
{
    u32 choice = RunRandomizerHash(ABILITY_RANDOMIZER_DOMAIN, family, key) % sAbilityPoolCount;
    for (u32 i = 0; i < sAbilityPoolCount; i++)
    {
        u32 ability = sAbilityPool[(choice + i) % sAbilityPoolCount];
        if (!used[ability])
        {
            used[ability] = TRUE;
            return ability;
        }
    }
    return ABILITY_NONE;
}

static void InitFamilyAbilities(struct FamilyAbilityCache *cache, u32 family)
{
    u16 *mapping = cache->mapping;
    memset(mapping, 0, sizeof(cache->mapping));
    bool8 used[ABILITIES_COUNT] = {0};
    for (u32 species = 1; species < NUM_SPECIES; species++)
        if (GetRandomizerSpeciesFamily(species) == family)
            for (u32 slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
                mapping[gSpeciesInfo[species].abilities[slot]] = TRUE;
    for (u32 ability = 1; ability < ABILITIES_COUNT; ability++)
        if (mapping[ability])
            mapping[ability] = PickAbility(family, ability, used);
    mapping[ABILITY_NONE] = ABILITY_NONE;
    for (u32 slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
        mapping[EXTRA_SLOT_KEY(slot)] = PickAbility(family, EXTRA_SLOT_KEY(slot), used);

    cache->family = family;
}

static u32 ResolveFamilySlot(const struct FamilyAbilityCache *cache, u32 species, u32 slot)
{
    u32 native = ReadNativeSlots(species);
    u32 original = gSpeciesInfo[species].abilities[slot];
    if (native & (1 << slot))
        return GetNativeAbilityAtSlot(species, slot);
    if (native)
    {
        bool32 useAddedSlot = original == ABILITY_NONE;
        for (u32 other = 0; other < NUM_ABILITY_SLOTS; other++)
            if (other != slot && original == gSpeciesInfo[species].abilities[other]
                && ((native & (1 << other)) || other < slot))
                useAddedSlot = TRUE;
        if (useAddedSlot)
            return cache->mapping[EXTRA_SLOT_KEY(slot)];
    }
    return cache->mapping[original];
}

u16 GetRandomizedSpeciesAbility(u16 species, u32 slot)
{
    if (!IsValidSpecies(species) || slot >= NUM_ABILITY_SLOTS)
        return ABILITY_NONE;
    if (!FlagGet(FLAG_RUN_RULE_ABILITIES))
        return gSpeciesInfo[species].abilities[slot];
    if (sAbilityPoolCount == 0)
        for (u32 ability = 1; ability < ABILITIES_COUNT; ability++)
            if (IsRandomizerAbilityAllowed(ability))
                sAbilityPool[sAbilityPoolCount++] = ability;
    u32 seed = RunRandomizerHash(ABILITY_RANDOMIZER_DOMAIN, 0, 0);
    if (!sCacheReady || sCachedSeed != seed)
    {
        memset(sFamilyCaches, 0, sizeof(sFamilyCaches));
        sCachedSeed = seed;
        sCacheReady = TRUE;
    }
    u32 family = GetRandomizerSpeciesFamily(species);
    for (u32 i = 0; i < FAMILY_CACHE_COUNT; i++)
        if (sFamilyCaches[i].family == family)
            return ResolveFamilySlot(&sFamilyCaches[i], species, slot);
    struct FamilyAbilityCache *cache = &sFamilyCaches[sNextFamilyCache];
    sNextFamilyCache = (sNextFamilyCache + 1) % FAMILY_CACHE_COUNT;
    InitFamilyAbilities(cache, family);
    return ResolveFamilySlot(cache, species, slot);
}

u32 GetAbilityItemTargetSlot(struct Pokemon *mon, u16 item)
{
    if (GetMonData(mon, MON_DATA_IS_EGG))
        return NUM_ABILITY_SLOTS;
    u32 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 current = GetMonData(mon, MON_DATA_ABILITY_NUM);
    u32 target;
    if (item == ITEM_ABILITY_CAPSULE && current < NUM_NORMAL_ABILITY_SLOTS)
        target = current ^ 1;
    else if (item == ITEM_ABILITY_PATCH)
        target = current == 2 ? 0 : 2;
    else
        return NUM_ABILITY_SLOTS;
    u32 ability = GetSpeciesAbility(species, target);
    if (ability == ABILITY_NONE || ability == GetAbilityBySpecies(species, current))
        return NUM_ABILITY_SLOTS;
    return target;
}

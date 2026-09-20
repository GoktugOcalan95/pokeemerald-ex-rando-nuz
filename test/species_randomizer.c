#include "global.h"
#include "event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/vars.h"
#include "item_randomizer.h"
#include "roamer.h"
#include "string_util.h"
#include "test/overworld_script.h"
#include "constants/trade.h"
#include "event_data.h"
#include "random.h"
#include "save.h"
#include "script_pokemon_util.h"
#include "starter_choose.h"
#include "wild_encounter.h"
#include "pokemon.h"
#include "species_randomizer.h"
#include "test/test.h"
#include "constants/form_change_types.h"

asm(".set VAR_0x8009, 0x8009\n.set VAR_0x800B, 0x800B\n");

TEST("Species randomizer families connect every enabled evolution and form")
{
    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        if (!gSpeciesInfo[species].baseHP)
            continue;
        u32 family = GetRandomizerSpeciesFamily(species);
        EXPECT(family != SPECIES_NONE);
        const struct Evolution *evolutions = GetSpeciesEvolutions(species);
        const u16 *forms = GetSpeciesFormTable(species);
        const struct FormChange *changes = GetSpeciesFormChanges(species);
        if (evolutions != NULL)
            for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
                if (GetRandomizerSpeciesFamily(evolutions[i].targetSpecies) != SPECIES_NONE)
                    EXPECT_EQ(GetRandomizerSpeciesFamily(evolutions[i].targetSpecies), family);
        if (forms != NULL)
            for (u32 i = 0; forms[i] != FORM_SPECIES_END; i++)
                if (GetRandomizerSpeciesFamily(forms[i]) != SPECIES_NONE)
                    EXPECT_EQ(GetRandomizerSpeciesFamily(forms[i]), family);
        if (changes != NULL)
            for (u32 i = 0; changes[i].method != FORM_CHANGE_TERMINATOR; i++)
                if (GetRandomizerSpeciesFamily(changes[i].targetSpecies) != SPECIES_NONE)
                    EXPECT_EQ(GetRandomizerSpeciesFamily(changes[i].targetSpecies), family);
    }
    EXPECT_EQ(GetRandomizerSpeciesFamily(SPECIES_EGG), SPECIES_NONE);
    EXPECT_EQ(GetRandomizerSpeciesFamily(SPECIES_VAPOREON), GetRandomizerSpeciesFamily(SPECIES_SYLVEON));
    EXPECT_EQ(GetRandomizerSpeciesFamily(SPECIES_RALTS), GetRandomizerSpeciesFamily(SPECIES_GALLADE_MEGA));
    EXPECT_EQ(GetRandomizerSpeciesFamily(SPECIES_MEOWTH), GetRandomizerSpeciesFamily(SPECIES_PERRSERKER));
    EXPECT_NE(GetRandomizerSpeciesFamily(SPECIES_KYUREM_WHITE), GetRandomizerSpeciesFamily(SPECIES_RESHIRAM));
    EXPECT_NE(GetRandomizerSpeciesFamily(SPECIES_NECROZMA_DUSK_MANE), GetRandomizerSpeciesFamily(SPECIES_SOLGALEO));
}

TEST("Species randomizer excludes temporary forms and unsupported creation state")
{
    const u16 stable[] = {SPECIES_GRENINJA, SPECIES_GRENINJA_BATTLE_BOND, SPECIES_PALAFIN_ZERO,
        SPECIES_WISHIWASHI_SOLO, SPECIES_MINIOR_CORE_BLUE, SPECIES_ZYGARDE_10_POWER_CONSTRUCT,
        SPECIES_ZYGARDE_50_POWER_CONSTRUCT, SPECIES_ROTOM_HEAT, SPECIES_HOOPA_UNBOUND,
        SPECIES_FURFROU_HEART, SPECIES_BURMY_TRASH, SPECIES_ALCREMIE_BERRY_RAINBOW_SWIRL,
        SPECIES_MEOWTH_ALOLA, SPECIES_NIHILEGO, SPECIES_IRON_VALIANT};
    const u16 excluded[] = {SPECIES_NONE, SPECIES_EGG, SPECIES_GRENINJA_ASH, SPECIES_PALAFIN_HERO,
        SPECIES_WISHIWASHI_SCHOOL, SPECIES_MINIOR_METEOR_BLUE, SPECIES_ZYGARDE_COMPLETE,
        SPECIES_CHARIZARD_MEGA_X, SPECIES_GROUDON_PRIMAL, SPECIES_NECROZMA_ULTRA,
        SPECIES_ALCREMIE_GMAX, SPECIES_TERAPAGOS_TERASTAL, SPECIES_KYUREM_WHITE,
        SPECIES_NECROZMA_DUSK_MANE, SPECIES_CALYREX_ICE, SPECIES_ARCEUS_FIRE,
        SPECIES_GIRATINA_ORIGIN, SPECIES_OGERPON_WELLSPRING};
    for (u32 i = 0; i < ARRAY_COUNT(stable); i++)
        EXPECT(IsRandomizerSpeciesEligible(stable[i]));
    for (u32 i = 0; i < ARRAY_COUNT(excluded); i++)
        EXPECT(!IsRandomizerSpeciesEligible(excluded[i]));
    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        if (!IsRandomizerSpeciesEligible(species))
            continue;
        const struct SpeciesInfo *info = &gSpeciesInfo[species];
        EXPECT(!info->isMegaEvolution && !info->isPrimalReversion && !info->isUltraBurst
            && !info->isGigantamax && !info->isTeraForm);
        const struct FormChange *changes = GetSpeciesFormChanges(species);
        if (changes != NULL)
            for (u32 i = 0; changes[i].method != FORM_CHANGE_TERMINATOR; i++)
                if (changes[i].method == FORM_CHANGE_FAINT || changes[i].method == FORM_CHANGE_END_BATTLE)
                    EXPECT_EQ(changes[i].targetSpecies, species);
    }
}

TEST("Species randomizer cosmetic metadata preserves functional and regional forms")
{
    for (u32 species = 1; species < NUM_SPECIES; species++)
    {
        u32 group = gSpeciesInfo[species].cosmeticFormOf;
        if (!group || !gSpeciesInfo[species].baseHP)
            continue;
        EXPECT(IsRandomizerSpeciesEligible(species));
        EXPECT(IsRandomizerSpeciesEligible(group));
        EXPECT_EQ(GetRandomizerSpeciesFamily(species), GetRandomizerSpeciesFamily(group));
        EXPECT_EQ((u32)gSpeciesInfo[group].cosmeticFormOf, group);
    }
    EXPECT_EQ((u32)gSpeciesInfo[SPECIES_ALCREMIE_BERRY_RAINBOW_SWIRL].cosmeticFormOf, SPECIES_ALCREMIE);
    EXPECT_EQ((u32)gSpeciesInfo[SPECIES_MINIOR_CORE_BLUE].cosmeticFormOf, SPECIES_MINIOR_CORE_RED);
    EXPECT_EQ((u32)gSpeciesInfo[SPECIES_SCATTERBUG_POKEBALL].cosmeticFormOf, SPECIES_SCATTERBUG_ICY_SNOW);
    EXPECT_EQ((u32)gSpeciesInfo[SPECIES_MEOWTH_ALOLA].cosmeticFormOf, SPECIES_NONE);
    EXPECT_EQ((u32)gSpeciesInfo[SPECIES_FLOETTE_ETERNAL].cosmeticFormOf, SPECIES_NONE);
    EXPECT_EQ((u32)gSpeciesInfo[SPECIES_TATSUGIRI_DROOPY].cosmeticFormOf, SPECIES_NONE);
}

TEST("Species randomizer assignments respect source pools and survive save load without using gameplay RNG")
{
    u16 expected[7];
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    for (u32 domain = SPECIES_REWARD_WILD; domain <= SPECIES_REWARD_STARTER; domain++)
    {
        for (u32 source = 0; source < 64; source++)
        {
            u32 species = RandomizeEncounterSpecies(SPECIES_MEW, domain, source, 0);
            EXPECT(IsRandomizerSpeciesEligible(species));
            EXPECT_EQ(IsRandomizerSpeciesLegendary(species), domain == SPECIES_REWARD_STATIC || domain == SPECIES_REWARD_ROAMER);
        }
        expected[domain - SPECIES_REWARD_WILD] = RandomizeEncounterSpecies(SPECIES_MEW, domain, 123, 0);
    }
    SeedRng(123);
    u32 next = Random();
    SeedRng(123);
    for (u32 domain = SPECIES_REWARD_WILD; domain <= SPECIES_REWARD_STARTER; domain++)
        EXPECT_EQ(RandomizeEncounterSpecies(SPECIES_MEW, domain, 123, 0), expected[domain - SPECIES_REWARD_WILD]);
    EXPECT_EQ(Random(), next);
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    gSaveBlock2Ptr->playerTrainerId[3] ^= 0x80;
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    for (u32 domain = SPECIES_REWARD_WILD; domain <= SPECIES_REWARD_STARTER; domain++)
        EXPECT_EQ(RandomizeEncounterSpecies(SPECIES_MEW, domain, 123, 0), expected[domain - SPECIES_REWARD_WILD]);
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
    EXPECT_EQ(RandomizeEncounterSpecies(SPECIES_MEW, SPECIES_REWARD_GIFT, 123, 0), SPECIES_MEW);
}

TEST("Species randomizer starter previews choose three distinct evolution families")
{
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    for (u32 seed = 0; seed < 32; seed++)
    {
        gSaveBlock2Ptr->playerTrainerId[0] = seed;
        u32 first = GetStarterPokemon(0);
        u32 second = GetStarterPokemon(1);
        u32 third = GetStarterPokemon(2);
        EXPECT(IsRandomizerSpeciesEligible(first));
        EXPECT(!IsRandomizerSpeciesLegendary(first));
        EXPECT_NE(GetRandomizerSpeciesFamily(first), GetRandomizerSpeciesFamily(second));
        EXPECT_NE(GetRandomizerSpeciesFamily(first), GetRandomizerSpeciesFamily(third));
        EXPECT_NE(GetRandomizerSpeciesFamily(second), GetRandomizerSpeciesFamily(third));
        EXPECT_EQ(GetStarterPokemon(0), first);
        ZeroPlayerPartyMons();
        ScriptGiveMon(first, 5, ITEM_NONE);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), first);
        SetBoxMonPerfectIVs(&gParties[B_TRAINER_PLAYER][0].box, 5);
        u32 perfect = 0;
        for (u32 stat = 0; stat < NUM_STATS; stat++)
            perfect += GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP_IV + stat) == 31;
        EXPECT(perfect >= 5);
    }
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
    EXPECT_EQ(GetStarterPokemon(0), SPECIES_TREECKO);
    EXPECT_EQ(GetStarterPokemon(3), SPECIES_TREECKO);
    ZeroPlayerPartyMons();
}

TEST("Species randomizer wild slots retain authored levels and selection probabilities")
{
    enum WildPokemonArea area;
    PARAMETRIZE { area = WILD_AREA_LAND; }
    PARAMETRIZE { area = WILD_AREA_WATER; }
    PARAMETRIZE { area = WILD_AREA_ROCKS; }
    struct WildPokemon mons[NUM_LAND_MONS_ENCOUNTER_SLOTS];
    struct WildPokemonInfo info = {.encounterRate = 20, .wildPokemon = mons};
    for (u32 i = 0; i < ARRAY_COUNT(mons); i++)
        mons[i] = (struct WildPokemon){.species = SPECIES_ZIGZAGOON, .minLevel = i + 7, .maxLevel = i + 7};
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    CreateRandomMon(&gParties[B_TRAINER_PLAYER][0], SPECIES_MAGIKARP, 5);
    for (u32 seed = 0; seed < 32; seed++)
    {
        SeedRng(seed);
        u32 slot = area == WILD_AREA_LAND ? ChooseWildMonIndex_Land()
            : area == WILD_AREA_WATER ? ChooseWildMonIndex_Water() : ChooseWildMonIndex_Rocks();
        SeedRng(seed);
        EXPECT(TryGenerateWildMon(&info, area, 0));
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES), RandomizeWildSlot(mons, slot));
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL), mons[slot].minLevel);
    }
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
    ZeroPlayerPartyMons();
}

TEST("Species randomizer initializes timed forms and follows independently selected equipment")
{
    struct Pokemon mon;
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    CreateRandomMon(&mon, SPECIES_FURFROU_HEART, 20);
    PrepareRandomizedEncounterMon(&mon);
    EXPECT(GetMonData(&mon, MON_DATA_DAYS_SINCE_FORM_CHANGE) > 0);
    CreateRandomMon(&mon, SPECIES_HOOPA_UNBOUND, 20);
    PrepareRandomizedEncounterMon(&mon);
    EXPECT(GetMonData(&mon, MON_DATA_DAYS_SINCE_FORM_CHANGE) > 0);
    CreateRandomMon(&mon, SPECIES_ARCEUS_NORMAL, 20);
    u16 item = ITEM_FLAME_PLATE;
    SetMonData(&mon, MON_DATA_HELD_ITEM, &item);
    PrepareRandomizedEncounterMon(&mon);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIES), SPECIES_ARCEUS_FIRE);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HELD_ITEM), ITEM_FLAME_PLATE);
    CreateRandomMon(&mon, SPECIES_ALCREMIE_BERRY_RAINBOW_SWIRL, 20);
    EXPECT(!TryFormChange(&mon, FORM_CHANGE_END_BATTLE, B_TRAINER_PLAYER));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIES), SPECIES_ALCREMIE_BERRY_RAINBOW_SWIRL);
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
}

TEST("Species randomizer scripted gift previews match receipt and independent held items")
{
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    FlagClear(FLAG_RUN_RULE_ITEMS);
    ZeroPlayerPartyMons();
    const u8 *script = OVERWORLD_SCRIPT(
        randomizespecies VAR_0x800B, SPECIES_CASTFORM_NORMAL, Test_RandomizedCastform + 5, SPECIES_REWARD_GIFT, PARTY_SIZE;
        Test_RandomizedCastform:;
        givemon SPECIES_CASTFORM_NORMAL, 25, ITEM_MYSTIC_WATER;
    );
    RunScriptImmediately(script);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), gSpecialVar_0x800B);
    EXPECT_EQ(VarGet(VAR_TEMP_TRANSFERRED_SPECIES), gSpecialVar_0x800B);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_LEVEL), 25);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM), ITEM_MYSTIC_WATER);
    u32 species = gSpecialVar_0x800B;
    FlagSet(FLAG_RUN_RULE_ITEMS);
    ZeroPlayerPartyMons();
    RunScriptImmediately(script);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), species);
    EXPECT(IsRandomizedRewardItemAllowed(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HELD_ITEM)));
    FlagClear(FLAG_RUN_RULE_ITEMS);
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
    ZeroPlayerPartyMons();
}

TEST("Species randomizer gift eggs retain egg state and stable source species")
{
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    ZeroPlayerPartyMons();
    RUN_OVERWORLD_SCRIPT(
        randomizespecies VAR_0x800B, SPECIES_WYNAUT, Test_RandomizedEgg + 1, SPECIES_REWARD_EGG;
        Test_RandomizedEgg:;
        giveegg SPECIES_WYNAUT;
    );
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), gSpecialVar_0x800B);
    EXPECT(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_IS_EGG));
    EXPECT(!IsRandomizerSpeciesLegendary(gSpecialVar_0x800B));
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
    ZeroPlayerPartyMons();
}

TEST("Species randomizer static previews match both battle slots and preserve levels")
{
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    RUN_OVERWORLD_SCRIPT(
        setvar VAR_0x8009, 7;
        randomizespecies VAR_0x800B, SPECIES_REGIROCK, Test_RandomizedStatic + 1, SPECIES_REWARD_STATIC, VAR_0x8009;
        Test_RandomizedStatic:;
        setwildbattle SPECIES_REGIROCK, 50, ITEM_NONE, SPECIES_KECLEON, 30, ITEM_NONE, sourceSlot=VAR_0x8009;
    );
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES), gSpecialVar_0x800B);
    EXPECT(IsRandomizerSpeciesLegendary(gSpecialVar_0x800B));
    EXPECT(!IsRandomizerSpeciesLegendary(GetMonData(&gParties[B_TRAINER_OPPONENT_A][1], MON_DATA_SPECIES)));
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL), 50);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][1], MON_DATA_LEVEL), 30);
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
}

TEST("Species randomizer event encounters retain fateful state and matching previews")
{
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    FlagClear(FLAG_RUN_RULE_ITEMS);
    RUN_OVERWORLD_SCRIPT(
        randomizespecies VAR_0x800B, SPECIES_LATIOS, Test_RandomizedEvent + 5, SPECIES_REWARD_STATIC;
        Test_RandomizedEvent:;
        seteventmon SPECIES_LATIOS, 50, ITEM_SOUL_DEW;
    );
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES), gSpecialVar_0x800B);
    EXPECT(IsRandomizerSpeciesLegendary(gSpecialVar_0x800B));
    EXPECT(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_MODERN_FATEFUL_ENCOUNTER));
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL), 50);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HELD_ITEM), ITEM_SOUL_DEW);
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
}

u16 GetInGameTradeSpeciesInfo(void);
void CreateInGameTradePokemon(void);

TEST("Species randomizer trades preserve the requested species and name the actual offer")
{
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    FlagClear(FLAG_RUN_RULE_ITEMS);
    gSpecialVar_0x8004 = 0;
    gSpecialVar_0x8005 = INGAME_TRADE_SEEDOT;
    CreateRandomMon(&gParties[B_TRAINER_PLAYER][0], SPECIES_RALTS, 17);
    EXPECT_EQ(GetInGameTradeSpeciesInfo(), SPECIES_RALTS);
    u16 species = RandomizeEncounterSpecies(SPECIES_SEEDOT, SPECIES_REWARD_TRADE, INGAME_TRADE_SEEDOT, 0);
    EXPECT_EQ(StringCompare(gStringVar2, GetSpeciesName(species)), 0);
    CreateInGameTradePokemon();
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES), species);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL), 17);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HELD_ITEM), ITEM_CHESTO_BERRY);
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
    ZeroPlayerPartyMons();
}

TEST("Species randomizer roamers keep their selected species after reconstruction and saving")
{
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    DeactivateAllRoamers();
    EXPECT(TryAddRoamer(SPECIES_LATIAS, 40));
    u32 species = gSaveBlock1Ptr->roamer[0].species;
    EXPECT(IsRandomizerSpeciesLegendary(species));
    EXPECT_EQ(species, RandomizeEncounterSpecies(SPECIES_LATIAS, SPECIES_REWARD_ROAMER, SPECIES_LATIAS, 0));
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    gSaveBlock1Ptr->roamer[0].species = SPECIES_NONE;
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    CreateRoamerMonInstance(0);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES), species);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_LEVEL), 40);
    DeactivateAllRoamers();
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
}

TEST("Species randomizer move dependent forms follow the actual initial moves")
{
    struct Pokemon mon;
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    EXPECT(IsRandomizerSpeciesEligible(SPECIES_KELDEO_ORDINARY));
    EXPECT(!IsRandomizerSpeciesEligible(SPECIES_KELDEO_RESOLUTE));
    CreateRandomMon(&mon, SPECIES_KELDEO_ORDINARY, 50);
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(&mon, MOVE_TACKLE, i);
    PrepareRandomizedEncounterMon(&mon);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIES), SPECIES_KELDEO_ORDINARY);
    SetMonMoveSlot(&mon, MOVE_SECRET_SWORD, 0);
    PrepareRandomizedEncounterMon(&mon);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIES), SPECIES_KELDEO_RESOLUTE);
    SetMonMoveSlot(&mon, MOVE_TACKLE, 0);
    EXPECT(TryFormChange(&mon, FORM_CHANGE_MOVE, B_TRAINER_PLAYER));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_SPECIES), SPECIES_KELDEO_ORDINARY);
    FlagClear(FLAG_RUN_RULE_ENCOUNTERS);
}

TEST("Birch chase graphics match the seeded battle species and preserve the authored default")
{
    InitEventData();
    SetBirchChaseGraphics();
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0), OBJ_EVENT_GFX_ZIGZAGOON_1);
    EXPECT_EQ(GetBirchRescueSpecies(), SPECIES_ZIGZAGOON);
    FlagSet(FLAG_RUN_RULE_ENCOUNTERS);
    for (u32 seed = 0; seed < 64; seed++)
    {
        memcpy(gSaveBlock2Ptr->playerTrainerId, &seed, sizeof(seed));
        u16 species = GetBirchRescueSpecies();
        SetBirchChaseGraphics();
        EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0), GetGraphicsIdForMon(species, FALSE, FALSE));
        EXPECT_EQ(GetBirchRescueSpecies(), species);
    }
    InitEventData();
}

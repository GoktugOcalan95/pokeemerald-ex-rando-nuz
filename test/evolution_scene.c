#include "global.h"
#include "evolution_scene.h"
#include "main.h"
#include "pokemon.h"
#include "text.h"
#include "test/test.h"

static void CB2_EvolutionFinished(void)
{
}

TEST("Evolution scene runs to completion and evolves the mon")
{
    u32 frame;
    u32 savedSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    MainCallback savedCallback = gMain.callback2;
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];

    ZeroPlayerPartyMons();
    CreateMon(mon, SPECIES_TOGEPI, 20, 0, OTID_STRUCT_PLAYER_ID);
    CalculatePlayerPartyCount();
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_FAST;
    gMain.newKeys = 0;
    gMain.heldKeys = 0;
    gCB2_AfterEvolution = CB2_EvolutionFinished;

    // Real frames: the scene waits on palette fades, sprite animations and the cry, which advance in v-blank.
    EvolutionScene(mon, SPECIES_TOGETIC, TRUE, 0);
    for (frame = 0; frame < 3000 && gMain.callback2 != CB2_EvolutionFinished; frame++)
    {
        gMain.callback2();
        VBlankIntrWait();
    }

    SetVBlankCallback(NULL);
    gMain.callback2 = savedCallback;
    gSaveBlock2Ptr->optionsTextSpeed = savedSpeed;
    EXPECT_LT(frame, 3000);
    EXPECT_EQ(GetMonData(mon, MON_DATA_SPECIES), SPECIES_TOGETIC);
}

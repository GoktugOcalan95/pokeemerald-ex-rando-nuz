#include "global.h"
#include "post_juan.h"
#include "event_data.h"
#include "constants/flags.h"
#include "constants/vars.h"

bool32 TryUnlockPostJuanLegendaries(void)
{
    if (!FlagGet(FLAG_BADGE08_GET) || FlagGet(FLAG_POST_JUAN_LEGENDARIES_UNLOCKED))
        return FALSE;

    FlagSet(FLAG_REGI_DOORS_OPENED);
    FlagSet(FLAG_SYS_BRAILLE_DIG);
    FlagSet(FLAG_SYS_REGIROCK_PUZZLE_COMPLETED);
    FlagSet(FLAG_SYS_BRAILLE_REGICE_COMPLETED);
    FlagSet(FLAG_SYS_REGISTEEL_PUZZLE_COMPLETED);
    FlagSet(FLAG_WALLACE_GOES_TO_SKY_PILLAR);
    FlagSet(FLAG_HIDE_SKY_PILLAR_WALLACE);
    FlagSet(FLAG_HIDE_SKY_PILLAR_TOP_RAYQUAZA);
    VarSet(VAR_SKY_PILLAR_STATE, 2);
    VarSet(VAR_SKY_PILLAR_RAYQUAZA_CRY_DONE, 1);
    FlagSet(FLAG_POST_JUAN_LEGENDARIES_UNLOCKED);
    return TRUE;
}

void UnlockPostJuanLegendaries(void)
{
    gSpecialVar_Result = TryUnlockPostJuanLegendaries();
}

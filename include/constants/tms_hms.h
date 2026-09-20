#ifndef GUARD_CONSTANTS_TMS_HMS_H
#define GUARD_CONSTANTS_TMS_HMS_H

#define FOREACH_ORIGINAL_TM(F) \
    F(FOCUS_PUNCH) \
    F(DRAGON_CLAW) \
    F(WATER_PULSE) \
    F(CALM_MIND) \
    F(ROAR) \
    F(TOXIC) \
    F(HAIL) \
    F(BULK_UP) \
    F(BULLET_SEED) \
    F(HIDDEN_POWER) \
    F(SUNNY_DAY) \
    F(TAUNT) \
    F(ICE_BEAM) \
    F(BLIZZARD) \
    F(HYPER_BEAM) \
    F(LIGHT_SCREEN) \
    F(PROTECT) \
    F(RAIN_DANCE) \
    F(GIGA_DRAIN) \
    F(SAFEGUARD) \
    F(FRUSTRATION) \
    F(SOLAR_BEAM) \
    F(IRON_TAIL) \
    F(THUNDERBOLT) \
    F(THUNDER) \
    F(EARTHQUAKE) \
    F(RETURN) \
    F(DIG) \
    F(PSYCHIC) \
    F(SHADOW_BALL) \
    F(BRICK_BREAK) \
    F(DOUBLE_TEAM) \
    F(REFLECT) \
    F(SHOCK_WAVE) \
    F(FLAMETHROWER) \
    F(SLUDGE_BOMB) \
    F(SANDSTORM) \
    F(FIRE_BLAST) \
    F(ROCK_TOMB) \
    F(AERIAL_ACE) \
    F(TORMENT) \
    F(FACADE) \
    F(SECRET_POWER) \
    F(REST) \
    F(ATTRACT) \
    F(THIEF) \
    F(STEEL_WING) \
    F(SKILL_SWAP) \
    F(SNATCH) \
    F(OVERHEAT)

#define FOREACH_EXPANDED_TM(F) \
    F(FOCUS_BLAST) \
    F(PLAY_ROUGH) \
    F(DRAGON_PULSE) \
    F(TRICK_ROOM) \
    F(ACROBATICS) \
    F(WILL_O_WISP) \
    F(ICE_SPINNER) \
    F(ROCK_POLISH) \
    F(FALSE_SWIPE) \
    F(TRAILBLAZE) \
    F(ENERGY_BALL) \
    F(ENCORE) \
    F(SEED_BOMB) \
    F(STONE_EDGE) \
    F(HYPER_VOICE) \
    F(DAZZLING_GLEAM) \
    F(ROOST) \
    F(SCALD) \
    F(X_SCISSOR) \
    F(TAILWIND) \
    F(KNOCK_OFF) \
    F(BUG_BUZZ) \
    F(PSYCHIC_FANGS) \
    F(WILD_CHARGE) \
    F(POWER_GEM) \
    F(EARTH_POWER) \
    F(U_TURN) \
    F(POISON_JAB) \
    F(SHADOW_CLAW) \
    F(DARK_PULSE) \
    F(AURA_SPHERE) \
    F(AVALANCHE) \
    F(BODY_PRESS) \
    F(VOLT_SWITCH) \
    F(AIR_SLASH) \
    F(FLASH_CANNON) \
    F(STEALTH_ROCK) \
    F(HEAT_WAVE) \
    F(BULLDOZE) \
    F(LIQUIDATION) \
    F(SNARL) \
    F(NASTY_PLOT) \
    F(GRASS_KNOT) \
    F(PSYSHOCK) \
    F(DRAINING_KISS) \
    F(PAYBACK) \
    F(GYRO_BALL) \
    F(IRON_HEAD) \
    F(DRAGON_DANCE) \
    F(DRAIN_PUNCH)

#define FOREACH_TM(F) \
    FOREACH_ORIGINAL_TM(F) \
    FOREACH_EXPANDED_TM(F)

#define FOREACH_HM(F) \
    F(CUT) \
    F(FLY) \
    F(SURF) \
    F(STRENGTH) \
    F(FLASH) \
    F(ROCK_SMASH) \
    F(WATERFALL) \
    F(DIVE)

#define FOREACH_TMHM(F) \
    FOREACH_TM(F) \
    FOREACH_HM(F)

#endif

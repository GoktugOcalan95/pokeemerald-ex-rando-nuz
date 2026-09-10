EventScript_UseSurf::
	checkfieldmove FIELD_MOVE_SURF, TRUE
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_EndUseSurf
	setfieldeffectargument 0, PARTY_SIZE
	lockall
	dofieldeffect FLDEFF_USE_SURF
EventScript_ReleaseUseSurf::
	releaseall
EventScript_EndUseSurf::
	end

EventScript_CurrentTooFast::
	lockall
	checkfieldmove FIELD_MOVE_SURF
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_CurrentTooFastEndMsg
	playse SE_SELECT
	msgbox Text_CurrentTooFast
EventScript_CurrentTooFastEndMsg::
	releaseall
	end

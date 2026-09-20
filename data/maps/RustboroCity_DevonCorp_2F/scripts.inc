RustboroCity_DevonCorp_2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, RustboroCity_DevonCorp_2F_OnTransition
	.byte 0

RustboroCity_DevonCorp_2F_OnTransition:
	call_if_eq VAR_FOSSIL_RESURRECTION_STATE, 1, RustboroCity_DevonCorp_2F_EventScript_SetFossilReady
	end

RustboroCity_DevonCorp_2F_EventScript_SetFossilReady::
	setvar VAR_FOSSIL_RESURRECTION_STATE, 2
	return

RustboroCity_DevonCorp_2F_EventScript_TalkToPokemonScientist::
	lock
	faceplayer
	call_if_eq VAR_FOSSIL_RESURRECTION_STATE, 1, RustboroCity_DevonCorp_2F_EventScript_SetFossilReady
	msgbox RustboroCity_DevonCorp_2F_Text_DeviceForTalkingToPokemon, MSGBOX_DEFAULT
	release
	end

RustboroCity_DevonCorp_2F_EventScript_BallScientist::
	lock
	faceplayer
	call_if_eq VAR_FOSSIL_RESURRECTION_STATE, 1, RustboroCity_DevonCorp_2F_EventScript_SetFossilReady
	goto_if_set FLAG_MET_DEVON_EMPLOYEE, RustboroCity_DevonCorp_2F_EventScript_DevelopedBalls
	msgbox RustboroCity_DevonCorp_2F_Text_DevelopingNewBalls, MSGBOX_DEFAULT
	release
	end

RustboroCity_DevonCorp_2F_EventScript_DevelopedBalls::
	msgbox RustboroCity_DevonCorp_2F_Text_WeFinallyMadeNewBalls, MSGBOX_DEFAULT
	release
	end

RustboroCity_DevonCorp_2F_EventScript_PokenavScientist::
	lock
	faceplayer
	call_if_eq VAR_FOSSIL_RESURRECTION_STATE, 1, RustboroCity_DevonCorp_2F_EventScript_SetFossilReady
	goto_if_set FLAG_RECEIVED_POKENAV, RustboroCity_DevonCorp_2F_EventScript_HasPokenav
	msgbox RustboroCity_DevonCorp_2F_Text_IMadePokenav, MSGBOX_DEFAULT
	release
	end

RustboroCity_DevonCorp_2F_EventScript_HasPokenav::
	msgbox RustboroCity_DevonCorp_2F_Text_WowThatsAPokenav, MSGBOX_DEFAULT
	release
	end

RustboroCity_DevonCorp_2F_EventScript_PokemonDreamsScientist::
	lock
	faceplayer
	call_if_eq VAR_FOSSIL_RESURRECTION_STATE, 1, RustboroCity_DevonCorp_2F_EventScript_SetFossilReady
	msgbox RustboroCity_DevonCorp_2F_Text_DeviceToVisualizePokemonDreams, MSGBOX_DEFAULT
	release
	end

RustboroCity_DevonCorp_2F_EventScript_FossilScientist::
	lock
	faceplayer
	goto_if_eq VAR_FOSSIL_RESURRECTION_STATE, 2, RustboroCity_DevonCorp_2F_EventScript_FossilMonReady
	goto_if_eq VAR_FOSSIL_RESURRECTION_STATE, 1, RustboroCity_DevonCorp_2F_EventScript_StillRegenerating
	msgbox RustboroCity_DevonCorp_2F_Text_DevelopDeviceToResurrectFossils, MSGBOX_DEFAULT
	checkitem ITEM_HELIX_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_DOME_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_OLD_AMBER
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_ROOT_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_CLAW_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_ARMOR_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_SKULL_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_COVER_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_PLUME_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_JAW_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_SAIL_FOSSIL
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_FOSSILIZED_BIRD
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_FOSSILIZED_FISH
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_FOSSILIZED_DRAKE
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	checkitem ITEM_FOSSILIZED_DINO
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_DevonCorp_2F_EventScript_NoticeFossil
	release
	end

RustboroCity_DevonCorp_2F_EventScript_NoticeFossil::
	msgbox RustboroCity_DevonCorp_2F_Text_WantToBringFossilBackToLife, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_DevonCorp_2F_EventScript_DeclineGiveFossil
	closemessage
	call RustboroCity_DevonCorp_2F_EventScript_OfferHELIX_FOSSIL
	call RustboroCity_DevonCorp_2F_EventScript_OfferDOME_FOSSIL
	call RustboroCity_DevonCorp_2F_EventScript_OfferOLD_AMBER
	call RustboroCity_DevonCorp_2F_EventScript_OfferROOT_FOSSIL
	call RustboroCity_DevonCorp_2F_EventScript_OfferCLAW_FOSSIL
	call RustboroCity_DevonCorp_2F_EventScript_OfferARMOR_FOSSIL
	call RustboroCity_DevonCorp_2F_EventScript_OfferSKULL_FOSSIL
	call RustboroCity_DevonCorp_2F_EventScript_OfferCOVER_FOSSIL
	call RustboroCity_DevonCorp_2F_EventScript_OfferPLUME_FOSSIL
	call RustboroCity_DevonCorp_2F_EventScript_OfferJAW_FOSSIL
	call RustboroCity_DevonCorp_2F_EventScript_OfferSAIL_FOSSIL
	call RustboroCity_DevonCorp_2F_EventScript_OfferFOSSILIZED_BIRD
	call RustboroCity_DevonCorp_2F_EventScript_OfferFOSSILIZED_FISH
	call RustboroCity_DevonCorp_2F_EventScript_OfferFOSSILIZED_DRAKE
	call RustboroCity_DevonCorp_2F_EventScript_OfferFOSSILIZED_DINO
	dynmultistack 1, 1, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, RustboroCity_DevonCorp_2F_EventScript_CancelFossilSelect
	copyvar VAR_0x8008, VAR_RESULT
	call RustboroCity_DevonCorp_2F_EventScript_ConsumeFossil
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_CancelFossilSelect
	bufferitemname STR_VAR_1, VAR_0x8008
	msgbox RustboroCity_DevonCorp_2F_Text_HandedFossilToResearcher, MSGBOX_DEFAULT
	release
	end

RustboroCity_DevonCorp_2F_EventScript_ConsumeFossil::
	removeitem VAR_0x8008
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	copyvar VAR_WHICH_FOSSIL_REVIVED, VAR_0x8008
	setvar VAR_FOSSIL_RESURRECTION_STATE, 1
RustboroCity_DevonCorp_2F_EventScript_FossilReturn::
	return

RustboroCity_DevonCorp_2F_EventScript_OfferHELIX_FOSSIL::
	checkitem ITEM_HELIX_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_HELIX_FOSSIL
	dynmultipush gStringVar1, ITEM_HELIX_FOSSIL
	return

RustboroCity_DevonCorp_2F_EventScript_OfferDOME_FOSSIL::
	checkitem ITEM_DOME_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_DOME_FOSSIL
	dynmultipush gStringVar1, ITEM_DOME_FOSSIL
	return

RustboroCity_DevonCorp_2F_EventScript_OfferOLD_AMBER::
	checkitem ITEM_OLD_AMBER
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_OLD_AMBER
	dynmultipush gStringVar1, ITEM_OLD_AMBER
	return

RustboroCity_DevonCorp_2F_EventScript_OfferROOT_FOSSIL::
	checkitem ITEM_ROOT_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_ROOT_FOSSIL
	dynmultipush gStringVar1, ITEM_ROOT_FOSSIL
	return

RustboroCity_DevonCorp_2F_EventScript_OfferCLAW_FOSSIL::
	checkitem ITEM_CLAW_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_CLAW_FOSSIL
	dynmultipush gStringVar1, ITEM_CLAW_FOSSIL
	return

RustboroCity_DevonCorp_2F_EventScript_OfferARMOR_FOSSIL::
	checkitem ITEM_ARMOR_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_ARMOR_FOSSIL
	dynmultipush gStringVar1, ITEM_ARMOR_FOSSIL
	return

RustboroCity_DevonCorp_2F_EventScript_OfferSKULL_FOSSIL::
	checkitem ITEM_SKULL_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_SKULL_FOSSIL
	dynmultipush gStringVar1, ITEM_SKULL_FOSSIL
	return

RustboroCity_DevonCorp_2F_EventScript_OfferCOVER_FOSSIL::
	checkitem ITEM_COVER_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_COVER_FOSSIL
	dynmultipush gStringVar1, ITEM_COVER_FOSSIL
	return

RustboroCity_DevonCorp_2F_EventScript_OfferPLUME_FOSSIL::
	checkitem ITEM_PLUME_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_PLUME_FOSSIL
	dynmultipush gStringVar1, ITEM_PLUME_FOSSIL
	return

RustboroCity_DevonCorp_2F_EventScript_OfferJAW_FOSSIL::
	checkitem ITEM_JAW_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_JAW_FOSSIL
	dynmultipush gStringVar1, ITEM_JAW_FOSSIL
	return

RustboroCity_DevonCorp_2F_EventScript_OfferSAIL_FOSSIL::
	checkitem ITEM_SAIL_FOSSIL
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_SAIL_FOSSIL
	dynmultipush gStringVar1, ITEM_SAIL_FOSSIL
	return

RustboroCity_DevonCorp_2F_EventScript_OfferFOSSILIZED_BIRD::
	checkitem ITEM_FOSSILIZED_BIRD
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_FOSSILIZED_BIRD
	dynmultipush gStringVar1, ITEM_FOSSILIZED_BIRD
	return

RustboroCity_DevonCorp_2F_EventScript_OfferFOSSILIZED_FISH::
	checkitem ITEM_FOSSILIZED_FISH
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_FOSSILIZED_FISH
	dynmultipush gStringVar1, ITEM_FOSSILIZED_FISH
	return

RustboroCity_DevonCorp_2F_EventScript_OfferFOSSILIZED_DRAKE::
	checkitem ITEM_FOSSILIZED_DRAKE
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_FOSSILIZED_DRAKE
	dynmultipush gStringVar1, ITEM_FOSSILIZED_DRAKE
	return

RustboroCity_DevonCorp_2F_EventScript_OfferFOSSILIZED_DINO::
	checkitem ITEM_FOSSILIZED_DINO
	goto_if_eq VAR_RESULT, FALSE, RustboroCity_DevonCorp_2F_EventScript_FossilReturn
	bufferitemname STR_VAR_1, ITEM_FOSSILIZED_DINO
	dynmultipush gStringVar1, ITEM_FOSSILIZED_DINO
	return

RustboroCity_DevonCorp_2F_EventScript_DeclineGiveFossil::
	msgbox RustboroCity_DevonCorp_2F_Text_OhIsThatSo, MSGBOX_DEFAULT
	release
	end

RustboroCity_DevonCorp_2F_EventScript_StillRegenerating::
	msgbox RustboroCity_DevonCorp_2F_Text_FossilRegeneratorTakesTime, MSGBOX_DEFAULT
	release
	end

RustboroCity_DevonCorp_2F_EventScript_FossilMonReady::
	call RustboroCity_DevonCorp_2F_EventScript_PreviewFossil
	bufferspeciesname STR_VAR_2, VAR_TEMP_TRANSFERRED_SPECIES
	msgbox RustboroCity_DevonCorp_2F_Text_FossilizedMonBroughtBackToLife, MSGBOX_DEFAULT
	call RustboroCity_DevonCorp_2F_EventScript_GiveFossil
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PARTY, RustboroCity_DevonCorp_2F_EventScript_ReceiveFossilParty
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, RustboroCity_DevonCorp_2F_EventScript_ReceiveFossilPC
	goto Common_EventScript_NoMoreRoomForPokemon
	end

RustboroCity_DevonCorp_2F_EventScript_PreviewFossil::
	switch VAR_WHICH_FOSSIL_REVIVED
	case ITEM_HELIX_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_PreviewHELIX_FOSSIL
	case ITEM_DOME_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_PreviewDOME_FOSSIL
	case ITEM_OLD_AMBER, RustboroCity_DevonCorp_2F_EventScript_PreviewOLD_AMBER
	case ITEM_ROOT_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_PreviewROOT_FOSSIL
	case ITEM_CLAW_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_PreviewCLAW_FOSSIL
	case ITEM_ARMOR_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_PreviewARMOR_FOSSIL
	case ITEM_SKULL_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_PreviewSKULL_FOSSIL
	case ITEM_COVER_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_PreviewCOVER_FOSSIL
	case ITEM_PLUME_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_PreviewPLUME_FOSSIL
	case ITEM_JAW_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_PreviewJAW_FOSSIL
	case ITEM_SAIL_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_PreviewSAIL_FOSSIL
	case ITEM_FOSSILIZED_BIRD, RustboroCity_DevonCorp_2F_EventScript_PreviewFOSSILIZED_BIRD
	case ITEM_FOSSILIZED_FISH, RustboroCity_DevonCorp_2F_EventScript_PreviewFOSSILIZED_FISH
	case ITEM_FOSSILIZED_DRAKE, RustboroCity_DevonCorp_2F_EventScript_PreviewFOSSILIZED_DRAKE
	case ITEM_FOSSILIZED_DINO, RustboroCity_DevonCorp_2F_EventScript_PreviewFOSSILIZED_DINO
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewHELIX_FOSSIL::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_OMANYTE, RustboroCity_DevonCorp_2F_EventScript_GiveHELIX_FOSSIL + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewDOME_FOSSIL::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_KABUTO, RustboroCity_DevonCorp_2F_EventScript_GiveDOME_FOSSIL + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewOLD_AMBER::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_AERODACTYL, RustboroCity_DevonCorp_2F_EventScript_GiveOLD_AMBER + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewROOT_FOSSIL::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_LILEEP, RustboroCity_DevonCorp_2F_EventScript_GiveROOT_FOSSIL + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewCLAW_FOSSIL::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_ANORITH, RustboroCity_DevonCorp_2F_EventScript_GiveCLAW_FOSSIL + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewARMOR_FOSSIL::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_SHIELDON, RustboroCity_DevonCorp_2F_EventScript_GiveARMOR_FOSSIL + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewSKULL_FOSSIL::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_CRANIDOS, RustboroCity_DevonCorp_2F_EventScript_GiveSKULL_FOSSIL + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewCOVER_FOSSIL::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_TIRTOUGA, RustboroCity_DevonCorp_2F_EventScript_GiveCOVER_FOSSIL + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewPLUME_FOSSIL::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_ARCHEN, RustboroCity_DevonCorp_2F_EventScript_GivePLUME_FOSSIL + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewJAW_FOSSIL::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_TYRUNT, RustboroCity_DevonCorp_2F_EventScript_GiveJAW_FOSSIL + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewSAIL_FOSSIL::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_AMAURA, RustboroCity_DevonCorp_2F_EventScript_GiveSAIL_FOSSIL + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewFOSSILIZED_BIRD::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_ARCTOZOLT, RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_BIRD + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewFOSSILIZED_FISH::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_ARCTOVISH, RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_FISH + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewFOSSILIZED_DRAKE::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_DRACOVISH, RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_DRAKE + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_PreviewFOSSILIZED_DINO::
	randomizespecies VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_DRACOZOLT, RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_DINO + 5, SPECIES_REWARD_GIFT, PARTY_SIZE
	return

RustboroCity_DevonCorp_2F_EventScript_GiveFossil::
	switch VAR_WHICH_FOSSIL_REVIVED
	case ITEM_HELIX_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_GiveHELIX_FOSSIL
	case ITEM_DOME_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_GiveDOME_FOSSIL
	case ITEM_OLD_AMBER, RustboroCity_DevonCorp_2F_EventScript_GiveOLD_AMBER
	case ITEM_ROOT_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_GiveROOT_FOSSIL
	case ITEM_CLAW_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_GiveCLAW_FOSSIL
	case ITEM_ARMOR_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_GiveARMOR_FOSSIL
	case ITEM_SKULL_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_GiveSKULL_FOSSIL
	case ITEM_COVER_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_GiveCOVER_FOSSIL
	case ITEM_PLUME_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_GivePLUME_FOSSIL
	case ITEM_JAW_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_GiveJAW_FOSSIL
	case ITEM_SAIL_FOSSIL, RustboroCity_DevonCorp_2F_EventScript_GiveSAIL_FOSSIL
	case ITEM_FOSSILIZED_BIRD, RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_BIRD
	case ITEM_FOSSILIZED_FISH, RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_FISH
	case ITEM_FOSSILIZED_DRAKE, RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_DRAKE
	case ITEM_FOSSILIZED_DINO, RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_DINO
	return

RustboroCity_DevonCorp_2F_EventScript_GiveHELIX_FOSSIL::
	givemon SPECIES_OMANYTE, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveDOME_FOSSIL::
	givemon SPECIES_KABUTO, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveOLD_AMBER::
	givemon SPECIES_AERODACTYL, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveROOT_FOSSIL::
	givemon SPECIES_LILEEP, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveCLAW_FOSSIL::
	givemon SPECIES_ANORITH, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveARMOR_FOSSIL::
	givemon SPECIES_SHIELDON, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveSKULL_FOSSIL::
	givemon SPECIES_CRANIDOS, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveCOVER_FOSSIL::
	givemon SPECIES_TIRTOUGA, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GivePLUME_FOSSIL::
	givemon SPECIES_ARCHEN, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveJAW_FOSSIL::
	givemon SPECIES_TYRUNT, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveSAIL_FOSSIL::
	givemon SPECIES_AMAURA, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_BIRD::
	givemon SPECIES_ARCTOZOLT, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_FISH::
	givemon SPECIES_ARCTOVISH, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_DRAKE::
	givemon SPECIES_DRACOVISH, 20
	return

RustboroCity_DevonCorp_2F_EventScript_GiveFOSSILIZED_DINO::
	givemon SPECIES_DRACOZOLT, 20
	return

RustboroCity_DevonCorp_2F_EventScript_ReceiveFossilParty::
	call RustboroCity_DevonCorp_2F_EventScript_ReceivedFossilFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_DevonCorp_2F_EventScript_FinishReceivingFossil
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto RustboroCity_DevonCorp_2F_EventScript_FinishReceivingFossil
	end

RustboroCity_DevonCorp_2F_EventScript_ReceiveFossilPC::
	call RustboroCity_DevonCorp_2F_EventScript_ReceivedFossilFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	call_if_eq VAR_RESULT, YES, Common_EventScript_NameReceivedBoxMon
	call Common_EventScript_TransferredToPC
	goto RustboroCity_DevonCorp_2F_EventScript_FinishReceivingFossil
	end

RustboroCity_DevonCorp_2F_EventScript_ReceivedFossilFanfare::
	bufferspeciesname STR_VAR_2, VAR_TEMP_TRANSFERRED_SPECIES
	playfanfare MUS_OBTAIN_ITEM
	message RustboroCity_DevonCorp_2F_Text_ReceivedMonFromResearcher
	waitmessage
	waitfanfare
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	return

RustboroCity_DevonCorp_2F_EventScript_FinishReceivingFossil::
	setvar VAR_FOSSIL_RESURRECTION_STATE, 0
	setflag FLAG_RECEIVED_REVIVED_FOSSIL_MON
	release
	end

RustboroCity_DevonCorp_2F_EventScript_CancelFossilSelect::
	release
	end

RustboroCity_DevonCorp_2F_EventScript_MatchCallScientist::
	lock
	faceplayer
	call_if_eq VAR_FOSSIL_RESURRECTION_STATE, 1, RustboroCity_DevonCorp_2F_EventScript_SetFossilReady
	goto_if_ge VAR_RUSTBORO_CITY_STATE, 6, RustboroCity_DevonCorp_2F_EventScript_WorkOnNext
	msgbox RustboroCity_DevonCorp_2F_Text_DevelopNewPokenavFeature, MSGBOX_DEFAULT
	release
	end

RustboroCity_DevonCorp_2F_EventScript_WorkOnNext::
	msgbox RustboroCity_DevonCorp_2F_Text_WhatToWorkOnNext, MSGBOX_DEFAULT
	release
	end

RustboroCity_DevonCorp_2F_Text_DeviceForTalkingToPokemon:
	.string "We're developing a device for talking\n"
	.string "with POKéMON.\p"
	.string "But we haven't had much success…$"

RustboroCity_DevonCorp_2F_Text_DevelopingNewBalls:
	.string "I'm developing new kinds of\n"
	.string "POKé BALLS…\p"
	.string "But I haven't made much headway…$"

RustboroCity_DevonCorp_2F_Text_WeFinallyMadeNewBalls:
	.string "We finally made new kinds of\n"
	.string "POKé BALLS!\p"
	.string "The REPEAT BALL makes it easier to\n"
	.string "catch POKéMON you've caught before.\p"
	.string "The TIMER BALL gets better at catching\n"
	.string "POKéMON the longer a battle runs.\p"
	.string "Both are proudly developed by\n"
	.string "the DEVON CORPORATION.\p"
	.string "Please give them a try!$"

RustboroCity_DevonCorp_2F_Text_IMadePokenav:
	.string "I made the POKéNAV!\p"
	.string "As an engineer, I feel blessed to have\n"
	.string "made something so great!$"

RustboroCity_DevonCorp_2F_Text_WowThatsAPokenav:
	.string "Oh, wow!\n"
	.string "That's a POKéNAV!\p"
	.string "It came about as a result of our\n"
	.string "PRESIDENT's desire to learn about\l"
	.string "the feelings of POKéMON.\p"
	.string "Would you like me to describe its\n"
	.string "features in detail?\p"
	.string "No, no. I think you'll find out just by\n"
	.string "trying the POKéNAV out.$"

RustboroCity_DevonCorp_2F_Text_DeviceToVisualizePokemonDreams:
	.string "I'm trying to develop a device that\n"
	.string "visually reproduces the dreams of\l"
	.string "POKéMON…\p"
	.string "But it's not going well.$"

RustboroCity_DevonCorp_2F_Text_DevelopDeviceToResurrectFossils:
	.string "I've been trying to develop a device\n"
	.string "that resurrects POKéMON from fossils…\p"
	.string "And, it's working!$"

RustboroCity_DevonCorp_2F_Text_WantToBringFossilBackToLife:
	.string "Wait! That thing you have there…\n"
	.string "Is that a POKéMON fossil?\p"
	.string "Would you like to bring that POKéMON\n"
	.string "back to life?\p"
	.string "I can with my newly developed\n"
	.string "FOSSIL REGENERATOR.$"

RustboroCity_DevonCorp_2F_Text_OhIsThatSo:
	.string "Oh, is that so?\p"
	.string "DEVON's technological expertise\n"
	.string "is outstanding, I tell you.$"

RustboroCity_DevonCorp_2F_Text_HandedFossilToResearcher:
	.string "Excellent!\n"
	.string "Let's do this right away.\p"
	.string "{PLAYER} handed the {STR_VAR_1} to\n"
	.string "the DEVON RESEARCHER.$"

RustboroCity_DevonCorp_2F_Text_FossilRegeneratorTakesTime:
	.string "The FOSSIL REGENERATOR, which I made,\n"
	.string "is incredible.\p"
	.string "But it has one drawback--it takes\n"
	.string "a long time to work.\p"
	.string "So, uh… How about you go for a stroll\n"
	.string "and look around for a while?$"

RustboroCity_DevonCorp_2F_Text_FossilizedMonBroughtBackToLife:
	.string "Thanks for waiting!\p"
	.string "Your fossilized POKéMON has been\n"
	.string "brought back to life!\p"
	.string "The fossil was an ancient POKéMON.\n"
	.string "{STR_VAR_2}, it was!$"

RustboroCity_DevonCorp_2F_Text_ReceivedMonFromResearcher:
	.string "{PLAYER} received {STR_VAR_2} from\n"
	.string "the DEVON RESEARCHER.$"

@ Unused
RustboroCity_DevonCorp_2F_Text_TooManyPokemon:
	.string "Uh-oh, you've got too many POKéMON.\n"
	.string "You have no room for this one.$"

RustboroCity_DevonCorp_2F_Text_DevelopNewPokenavFeature:
	.string "I'm trying to develop a new feature\n"
	.string "for the POKéNAV…\p"
	.string "But it's not going well.$"

RustboroCity_DevonCorp_2F_Text_WhatToWorkOnNext:
	.string "Well, now what shall I work on\n"
	.string "developing next?\p"
	.string "Our company allows us to make our\n"
	.string "inspirations into reality.\p"
	.string "One couldn't ask for a better\n"
	.string "environment as an engineer.$"


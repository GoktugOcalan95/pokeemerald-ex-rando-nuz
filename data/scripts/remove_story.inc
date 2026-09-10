RemoveStory_Return::
	return

RemoveStory_End::
	end

RemoveStory_Release::
	releaseall
	end

RemoveStory_AfterRoxanne::
	callnative RemoveStoryAfterRoxanne
	call RemoveStory_GiveDevonTools
	return

RemoveStory_GiveDevonTools::
	goto_if_unset FLAG_BADGE01_GET, RemoveStory_Return
	call_if_unset FLAG_RECEIVED_POKENAV, RemoveStory_GivePokenav
	goto_if_set FLAG_REMOVE_STORY_RECEIVED_LETTER, RemoveStory_Return
	msgbox RemoveStory_Text_Letter, MSGBOX_DEFAULT
	giveitem ITEM_LETTER
	goto_if_eq VAR_RESULT, FALSE, RemoveStory_GiftBagFullReturn
	setflag FLAG_REMOVE_STORY_RECEIVED_LETTER
	return

RemoveStory_GivePokenav::
	msgbox RemoveStory_Text_Pokenav, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message RustboroCity_DevonCorp_3F_Text_ReceivedPokenav
	waitfanfare
	setflag FLAG_SYS_POKENAV_GET
	setflag FLAG_RECEIVED_POKENAV
	setflag FLAG_HAS_MATCH_CALL
	setflag FLAG_ADDED_MATCH_CALL_TO_POKENAV
	setflag FLAG_HIDE_RUSTBORO_CITY_POKEMON_SCHOOL_SCOTT
	return

RemoveStory_GiftBagFullReturn::
	msgbox gText_TooBadBagIsFull, MSGBOX_DEFAULT
	return

RemoveStory_WoodsResearcher::
	goto_if_unset FLAG_RUN_RULE_REMOVE_STORY, RemoveStory_End
	lock
	faceplayer
	msgbox Common_Text_RandomizedGift, MSGBOX_DEFAULT
	giveitem_random ITEM_GREAT_BALL, FLAG_HIDE_PETALBURG_WOODS_DEVON_EMPLOYEE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_HIDE_PETALBURG_WOODS_DEVON_EMPLOYEE
	removeobject VAR_LAST_TALKED
	release
	end

RemoveStory_MrStone::
	goto_if_unset FLAG_BADGE01_GET, RemoveStory_MrStoneWelcome
	call RemoveStory_GiveDevonTools
	goto_if_set FLAG_REMOVE_STORY_RUSTBORO_GIFT, RemoveStory_MrStoneLetterReward
	msgbox Common_Text_RandomizedGift, MSGBOX_DEFAULT
	giveitem_random ITEM_GREAT_BALL, FLAG_RETURNED_DEVON_GOODS
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_REMOVE_STORY_RUSTBORO_GIFT
RemoveStory_MrStoneLetterReward:
	goto_if_set FLAG_RECEIVED_EXP_SHARE, RustboroCity_DevonCorp_3F_EventScript_MrStoneAfterFavor
	goto_if_set FLAG_DELIVERED_STEVEN_LETTER, RustboroCity_DevonCorp_3F_EventScript_GiveExpShare
RemoveStory_MrStoneWelcome:
	msgbox RemoveStory_Text_MrStone, MSGBOX_DEFAULT
	release
	end

RemoveStory_StevenLetter::
	goto_if_set FLAG_DELIVERED_STEVEN_LETTER, RemoveStory_SteelWing
	checkitem ITEM_LETTER
	goto_if_eq VAR_RESULT, FALSE, RemoveStory_StevenNoLetter
	msgbox GraniteCave_StevensRoom_Text_ImStevenLetterForMe, MSGBOX_DEFAULT
	setvar VAR_0x8004, ITEM_LETTER
	call Common_EventScript_PlayerHandedOverTheItem
	setflag FLAG_DELIVERED_STEVEN_LETTER
RemoveStory_SteelWing:
	goto_if_set FLAG_REMOVE_STORY_STEEL_WING, RemoveStory_Release
	msgbox_tm GraniteCave_StevensRoom_Text_ThankYouTakeThis, ITEM_TM_STEEL_WING, MSGBOX_DEFAULT
	giveitem ITEM_TM_STEEL_WING
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_REMOVE_STORY_STEEL_WING
	goto GraniteCave_StevensRoom_EventScript_RegisterSteven

RemoveStory_StevenNoLetter::
	msgbox RemoveStory_Text_Steven, MSGBOX_DEFAULT
	release
	end

RemoveStory_MuseumThief::
	finditem ITEM_TM_THIEF
	goto_if_unset FLAG_HIDE_REMOVE_STORY_MUSEUM_ITEM, RemoveStory_End
	setflag FLAG_RECEIVED_TM_THIEF
	end

RemoveStory_GiveMagmaEmblem::
	goto_if_set FLAG_RECEIVED_RED_OR_BLUE_ORB, RemoveStory_OldLadyPeaceful
	msgbox RemoveStory_Text_Emblem, MSGBOX_DEFAULT
	giveitem ITEM_MAGMA_EMBLEM
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_RED_OR_BLUE_ORB
	release
	end

RemoveStory_Wallace::
	goto_if_set FLAG_RECEIVED_HM_WATERFALL, SootopolisCity_EventScript_GoToGym
	msgbox RemoveStory_Text_Waterfall, MSGBOX_DEFAULT
	giveitem ITEM_HM_WATERFALL
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_HM_WATERFALL
	msgbox SootopolisCity_Text_ExplainWaterfallGoToGym, MSGBOX_DEFAULT
	release
	end

RemoveStory_StevenDive::
	goto_if_set FLAG_RECEIVED_HM_DIVE, MossdeepCity_StevensHouse_EventScript_StevenAfterDive
	msgbox RemoveStory_Text_Dive, MSGBOX_DEFAULT
	giveitem ITEM_HM_DIVE
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_RECEIVED_HM_DIVE
	setflag FLAG_OMIT_DIVE_FROM_STEVEN_LETTER
	msgbox MossdeepCity_StevensHouse_Text_ExplainDive, MSGBOX_DEFAULT
	release
	end

RemoveStory_CableCarGate::
	goto_if_set FLAG_BADGE03_GET, Route112_CableCarStation_EventScript_OfferRide
	msgbox RemoveStory_Text_CableCar, MSGBOX_DEFAULT
	release
	end

RemoveStory_BrineyGate::
	goto_if_set FLAG_BADGE01_GET, Route104_MrBrineysHouse_EventScript_WhereAreWeBound
	msgbox RemoveStory_Text_Briney, MSGBOX_DEFAULT
	release
	end

RemoveStory_Text_Pokenav:
	.string "DEVON sent this POKéNAV to help\n"
	.string "you on your journey!$"
RemoveStory_Text_Letter:
	.string "MR. STONE has a LETTER for STEVEN\n"
	.string "in DEWFORD's GRANITE CAVE.\p"
	.string "You can deliver it whenever you like.$"
RemoveStory_Text_MrStone:
	.string "Welcome to DEVON!\n"
	.string "I hope your journey is going well.$"
RemoveStory_Text_Steven:
	.string "I'm STEVEN. I explore caves in\n"
	.string "search of rare stones.$"
RemoveStory_Text_Emblem:
	.string "I found this emblem on the mountain.\n"
	.string "You may have it.$"
RemoveStory_Text_Waterfall:
	.string "I'm WALLACE. This HM will help you\n"
	.string "continue your journey after this GYM.$"
RemoveStory_Text_Dive:
	.string "STEVEN: Take this HM to explore\n"
	.string "underwater after earning the MIND BADGE.$"
RemoveStory_Text_CableCar:
	.string "The cable car opens to TRAINERS\n"
	.string "who have earned WATTSON's BADGE.$"
RemoveStory_Text_Briney:
	.string "Come see me after earning\n"
	.string "ROXANNE's BADGE. We can set sail!$"

RemoveStory_PositionWallace::
	call_if_unset FLAG_RECEIVED_HM_WATERFALL, RemoveStory_ShowWallace
	setobjectxyperm LOCALID_SOOTOPOLIS_WALLACE, 32, 33
	setobjectxyperm LOCALID_SOOTOPOLIS_EXPERT, 30, 18
	setobjectmovementtype LOCALID_SOOTOPOLIS_WALLACE, MOVEMENT_TYPE_FACE_DOWN
	return

RemoveStory_PrepareDive::
	goto_if_set FLAG_RECEIVED_HM_DIVE, RemoveStory_Return
	setvar VAR_STEVENS_HOUSE_STATE, 1
	return

RemoveStory_StevenDiveArrival::
	lockall
	setvar VAR_STEVENS_HOUSE_STATE, 2
	goto RemoveStory_StevenDive

RemoveStory_LockSteven::
	lock
	faceplayer
	return

RemoveStory_SternBeforeBadge::
	msgbox SlateportCity_Harbor_Text_WontBeLongBeforeWeFinishFerry, MSGBOX_DEFAULT
	release
	end

RemoveStory_OldLadyPeaceful::
	msgbox RemoveStory_Text_PeacefulMountain, MSGBOX_DEFAULT
	release
	end

RemoveStory_Text_PeacefulMountain:
	.string "We watch over the orbs here.\n"
	.string "Please treat this mountain with care.$"

RemoveStory_ShowWallace::
	clearflag FLAG_HIDE_SOOTOPOLIS_CITY_WALLACE
	return

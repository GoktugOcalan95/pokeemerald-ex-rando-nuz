MandatoryRivals_EventScript_Blocked::
	lockall
	msgbox MandatoryRivals_Text_BattleFirst, MSGBOX_DEFAULT
	releaseall
	end

MandatoryRivals_Text_BattleFirst:
	.string "My rival is waiting at\n"
	.string "{STR_VAR_1}.\p"
	.string "I should battle them before\n"
	.string "going any farther.$"

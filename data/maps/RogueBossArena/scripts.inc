RogueBossArena_MapScripts::
	.byte 0

RogueBossArena_EventScript_Boss::
	lock
	faceplayer
	specialvar VAR_RESULT, RogueScript_GetChapter
	goto_if_eq VAR_RESULT, 0, RogueBossArena_EventScript_Roxanne
	goto_if_eq VAR_RESULT, 1, RogueBossArena_EventScript_Brawly
	goto_if_eq VAR_RESULT, 2, RogueBossArena_EventScript_Wattson
	goto_if_eq VAR_RESULT, 3, RogueBossArena_EventScript_Flannery
	msgbox RogueBossArena_Text_NoBoss, MSGBOX_DEFAULT
	release
	end

RogueBossArena_EventScript_Roxanne::
	cleartrainerflag TRAINER_ROXANNE_1
	trainerbattle_single TRAINER_ROXANNE_1, RogueBossArena_Text_RoxanneIntro, RogueBossArena_Text_BossDefeat, RogueBossArena_EventScript_BossDefeated, NO_MUSIC
	release
	end

RogueBossArena_EventScript_Brawly::
	cleartrainerflag TRAINER_BRAWLY_1
	trainerbattle_single TRAINER_BRAWLY_1, RogueBossArena_Text_BrawlyIntro, RogueBossArena_Text_BossDefeat, RogueBossArena_EventScript_BossDefeated, NO_MUSIC
	release
	end

RogueBossArena_EventScript_Wattson::
	cleartrainerflag TRAINER_WATTSON_1
	trainerbattle_single TRAINER_WATTSON_1, RogueBossArena_Text_WattsonIntro, RogueBossArena_Text_BossDefeat, RogueBossArena_EventScript_BossDefeated, NO_MUSIC
	release
	end

RogueBossArena_EventScript_Flannery::
	cleartrainerflag TRAINER_FLANNERY_1
	trainerbattle_single TRAINER_FLANNERY_1, RogueBossArena_Text_FlanneryIntro, RogueBossArena_Text_BossDefeat, RogueBossArena_EventScript_BossDefeated, NO_MUSIC
	release
	end

RogueBossArena_EventScript_BossDefeated::
	goto_if_ne VAR_RESULT, B_OUTCOME_WON, RogueBossArena_EventScript_BossNotDefeated
	specialvar VAR_RESULT, RogueScript_CompleteRoom
	goto_if_eq VAR_RESULT, FALSE, RogueBossArena_EventScript_CompleteFailed
	specialvar VAR_RESULT, RogueScript_IsRunComplete
	goto_if_eq VAR_RESULT, TRUE, RogueBossArena_EventScript_RunWon
	msgbox RogueBossArena_Text_ChapterComplete, MSGBOX_DEFAULT
	goto RogueBossArena_EventScript_ReturnHub

RogueBossArena_EventScript_RunWon::
	msgbox RogueBossArena_Text_RunWon, MSGBOX_DEFAULT

RogueBossArena_EventScript_ReturnHub::
	closemessage
	warp MAP_ROGUE_HUB, 12, 8
	waitstate
	release
	end

RogueBossArena_EventScript_BossNotDefeated::
	msgbox RogueBossArena_Text_BossStillWaiting, MSGBOX_DEFAULT
	release
	end

RogueBossArena_EventScript_CompleteFailed::
	msgbox RogueBossArena_Text_CompleteFailed, MSGBOX_DEFAULT
	release
	end

RogueBossArena_Text_RoxanneIntro:
	.string "CHAPTER 1 ends here!\nShow me your run has a foundation.$"

RogueBossArena_Text_BrawlyIntro:
	.string "CHAPTER 2 ends here!\nLet's see how strong your team became.$"

RogueBossArena_Text_WattsonIntro:
	.string "CHAPTER 3 ends here!\nThis battle will test your momentum.$"

RogueBossArena_Text_FlanneryIntro:
	.string "The final BOSS GATE is open!\nGive this run everything you have.$"

RogueBossArena_Text_BossDefeat:
	.string "You cleared the BOSS GATE!$"

RogueBossArena_Text_BossStillWaiting:
	.string "The BOSS GATE remains open.\nReturn when you are ready.$"

RogueBossArena_Text_ChapterComplete:
	.string "Chapter complete!\nThe next two routes are now unlocked.$"

RogueBossArena_Text_RunWon:
	.string "All four BOSS GATES are cleared!\nThe ROGUE run is complete.$"

RogueBossArena_Text_CompleteFailed:
	.string "The boss result could not be recorded.$"

RogueBossArena_Text_NoBoss:
	.string "No chapter boss is available.$"

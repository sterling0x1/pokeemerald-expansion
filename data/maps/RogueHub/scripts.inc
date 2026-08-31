RogueHub_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, RogueHub_OnFrame
	.byte 0

RogueHub_OnFrame:
	map_script_2 VAR_TEMP_0, 0, RogueHub_EventScript_ShowPayout
	.2byte 0

RogueHub_EventScript_ShowPayout::
	setvar VAR_TEMP_0, 1
	specialvar VAR_RESULT, RogueScript_CollectPayout
	goto_if_eq VAR_RESULT, 0, RogueHub_EventScript_NoPayout
	lockall
	msgbox RogueHub_Text_Payout, MSGBOX_DEFAULT
	releaseall

RogueHub_EventScript_NoPayout::
	end

RogueHub_EventScript_RouteGuide::
	lock
	faceplayer
	specialvar VAR_RESULT, RogueScript_IsRunComplete
	goto_if_eq VAR_RESULT, TRUE, RogueHub_EventScript_RunEnded
	specialvar VAR_RESULT, RogueScript_IsRunFailed
	goto_if_eq VAR_RESULT, TRUE, RogueHub_EventScript_RunEnded
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	goto_if_eq VAR_RESULT, 0, RogueHub_EventScript_NeedPartner
	specialvar VAR_RESULT, RogueScript_IsBossReady
	goto_if_eq VAR_RESULT, TRUE, RogueHub_EventScript_BossPrompt
	special RogueScript_BufferNextRoomNumber
	msgbox RogueHub_Text_DepartPrompt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RogueHub_EventScript_CancelDeparture
	goto RogueHub_EventScript_ConfirmDeparture

RogueHub_EventScript_BossPrompt::
	special RogueScript_BufferChapterNumber
	msgbox RogueHub_Text_BossPrompt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RogueHub_EventScript_CancelDeparture

RogueHub_EventScript_ConfirmDeparture::
	specialvar VAR_RESULT, RogueScript_SelectFirstRoute
	goto_if_eq VAR_RESULT, FALSE, RogueHub_EventScript_NoRouteReady
	specialvar VAR_RESULT, RogueScript_IsBossRoom
	goto_if_eq VAR_RESULT, TRUE, RogueHub_EventScript_WarpBoss
	msgbox RogueHub_Text_Departing, MSGBOX_DEFAULT
	closemessage
	specialvar VAR_RESULT, RogueScript_GetCurrentRouteVariant
	goto_if_eq VAR_RESULT, 1, RogueHub_EventScript_WarpRoute2
	goto_if_eq VAR_RESULT, 2, RogueHub_EventScript_WarpRoute3
	goto_if_eq VAR_RESULT, 3, RogueHub_EventScript_WarpRoute4
	goto_if_eq VAR_RESULT, 4, RogueHub_EventScript_WarpRoute5
	goto_if_eq VAR_RESULT, 5, RogueHub_EventScript_WarpRoute6
	goto_if_eq VAR_RESULT, 6, RogueHub_EventScript_WarpRoute7
	goto_if_eq VAR_RESULT, 7, RogueHub_EventScript_WarpRoute8
	warp MAP_ROGUE_ROUTE, 10, 18
	waitstate
	release
	end

RogueHub_EventScript_WarpRoute2::
	warp MAP_ROGUE_ROUTE_2, 18, 11
	waitstate
	release
	end

RogueHub_EventScript_WarpRoute3::
	warp MAP_ROGUE_ROUTE_3, 49, 12
	waitstate
	release
	end

RogueHub_EventScript_WarpRoute4::
	warp MAP_ROGUE_ROUTE_4, 12, 7
	waitstate
	release
	end

RogueHub_EventScript_WarpRoute5::
	warp MAP_ROGUE_ROUTE_5, 16, 37
	waitstate
	release
	end

RogueHub_EventScript_WarpRoute6::
	warp MAP_ROGUE_ROUTE_6, 37, 11
	waitstate
	release
	end

RogueHub_EventScript_WarpRoute7::
	warp MAP_ROGUE_ROUTE_7, 26, 35
	waitstate
	release
	end

RogueHub_EventScript_WarpRoute8::
	warp MAP_ROGUE_ROUTE_8, 4, 9
	waitstate
	release
	end

RogueHub_EventScript_WarpBoss::
	msgbox RogueHub_Text_BossDeparting, MSGBOX_DEFAULT
	closemessage
	warp MAP_ROGUE_BOSS_ARENA, 12, 8
	waitstate
	release
	end

RogueHub_EventScript_NeedPartner::
	msgbox RogueHub_Text_NeedPartner, MSGBOX_DEFAULT
	release
	end

RogueHub_EventScript_CancelDeparture::
	msgbox RogueHub_Text_CancelDeparture, MSGBOX_DEFAULT
	release
	end

RogueHub_EventScript_NoRouteReady::
	msgbox RogueHub_Text_NoRouteReady, MSGBOX_DEFAULT
	release
	end

RogueHub_EventScript_RunEnded::
	msgbox RogueHub_Text_RunEnded, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RogueHub_EventScript_CancelDeparture
	special RogueScript_StartNewRun
	msgbox RogueHub_Text_NewRunReady, MSGBOX_DEFAULT
	release
	end

RogueHub_EventScript_RunGuide::
	lock
	faceplayer
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	goto_if_ne VAR_RESULT, 0, RogueHub_EventScript_PartnerReady
	msgbox RogueHub_Text_ChoosePartnerIntro, MSGBOX_DEFAULT

RogueHub_EventScript_ChooseTreecko::
	msgbox RogueHub_Text_ChooseTreecko, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, RogueHub_EventScript_GiveTreecko
	msgbox RogueHub_Text_ChooseTorchic, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, RogueHub_EventScript_GiveTorchic
	msgbox RogueHub_Text_ChooseMudkip, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, RogueHub_EventScript_GiveMudkip
	specialvar VAR_RESULT, RogueScript_HasBonusStarterUnlock
	goto_if_eq VAR_RESULT, TRUE, RogueHub_EventScript_ChooseEevee
	goto RogueHub_EventScript_CheckMysteryPartner

RogueHub_EventScript_ChooseEevee::
	msgbox RogueHub_Text_ChooseEevee, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, RogueHub_EventScript_GiveEevee

RogueHub_EventScript_CheckMysteryPartner::
	specialvar VAR_RESULT, RogueScript_HasMysteryPartnerUnlock
	goto_if_eq VAR_RESULT, TRUE, RogueHub_EventScript_ChooseMysteryPartner
	goto RogueHub_EventScript_ChooseTreecko

RogueHub_EventScript_GiveTreecko::
	givemon SPECIES_TREECKO, 5, ITEM_NONE
	goto RogueHub_EventScript_FinishPartnerChoice

RogueHub_EventScript_GiveTorchic::
	givemon SPECIES_TORCHIC, 5, ITEM_NONE
	goto RogueHub_EventScript_FinishPartnerChoice

RogueHub_EventScript_GiveMudkip::
	givemon SPECIES_MUDKIP, 5, ITEM_NONE
	goto RogueHub_EventScript_FinishPartnerChoice

RogueHub_EventScript_GiveEevee::
	givemon SPECIES_EEVEE, 5, ITEM_NONE
	goto RogueHub_EventScript_FinishPartnerChoice

RogueHub_EventScript_ChooseMysteryPartner::
	msgbox RogueHub_Text_ChooseMysteryPartner, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RogueHub_EventScript_ChooseTreecko
	specialvar VAR_RESULT, RogueScript_GiveMysteryPartner
	goto_if_eq VAR_RESULT, FALSE, RogueHub_EventScript_ChooseTreecko
	msgbox RogueHub_Text_MysteryPartnerGiven, MSGBOX_DEFAULT

RogueHub_EventScript_FinishPartnerChoice::
	setflag FLAG_SYS_POKEMON_GET
	setflag FLAG_SYS_POKEDEX_GET
	setflag FLAG_SYS_NATIONAL_DEX
	setflag FLAG_SYS_B_DASH
	setflag FLAG_ADVENTURE_STARTED
	special SetUnlockedPokedexFlags
	special EnableNationalPokedex
	giveitem ITEM_POKE_BALL, 5
	msgbox RogueHub_Text_PartnerChosen, MSGBOX_DEFAULT
	specialvar VAR_RESULT, RogueScript_GrantStartingBundle
	goto_if_eq VAR_RESULT, FALSE, RogueHub_EventScript_PartnerChoiceDone
	msgbox RogueHub_Text_StartingBundleGiven, MSGBOX_DEFAULT

RogueHub_EventScript_PartnerChoiceDone::
	release
	end

RogueHub_EventScript_PartnerReady::
	msgbox RogueHub_Text_PartnerReady, MSGBOX_DEFAULT
	release
	end

RogueHub_EventScript_RestStation::
	lock
	faceplayer
	msgbox RogueHub_Text_RestStation, MSGBOX_DEFAULT
	special HealPlayerParty
	playfanfare MUS_HEAL
	waitfanfare
	release
	end

RogueHub_EventScript_CasinoHost::
	lock
	faceplayer

RogueHub_EventScript_CasinoMenu::
	special RogueScript_BufferChipBalance
	msgbox RogueHub_Text_CasinoWelcome, MSGBOX_DEFAULT
	multichoice 7, 1, MULTI_ROGUE_CASINO, FALSE
	goto_if_eq VAR_RESULT, 0, RogueHub_EventScript_BuyStarterPass
	goto_if_eq VAR_RESULT, 1, RogueHub_EventScript_BuyStartingBundle
	goto_if_eq VAR_RESULT, 2, RogueHub_EventScript_BuyMysteryPartner
	msgbox RogueHub_Text_CasinoLeave, MSGBOX_DEFAULT
	release
	end

RogueHub_EventScript_BuyStarterPass::
	setvar VAR_0x8004, 0
	goto RogueHub_EventScript_TryCasinoPurchase

RogueHub_EventScript_BuyStartingBundle::
	setvar VAR_0x8004, 1
	goto RogueHub_EventScript_TryCasinoPurchase

RogueHub_EventScript_BuyMysteryPartner::
	setvar VAR_0x8004, 2

RogueHub_EventScript_TryCasinoPurchase::
	specialvar VAR_RESULT, RogueScript_TryPurchaseUnlock
	goto_if_eq VAR_RESULT, 0, RogueHub_EventScript_CasinoPurchaseSuccess
	goto_if_eq VAR_RESULT, 1, RogueHub_EventScript_CasinoAlreadyOwned
	goto_if_eq VAR_RESULT, 2, RogueHub_EventScript_CasinoNotEnough
	msgbox RogueHub_Text_CasinoError, MSGBOX_DEFAULT
	release
	end

RogueHub_EventScript_CasinoPurchaseSuccess::
	special RogueScript_BufferChipBalance
	msgbox RogueHub_Text_CasinoPurchaseSuccess, MSGBOX_DEFAULT
	goto RogueHub_EventScript_CasinoMenu

RogueHub_EventScript_CasinoAlreadyOwned::
	msgbox RogueHub_Text_CasinoAlreadyOwned, MSGBOX_DEFAULT
	goto RogueHub_EventScript_CasinoMenu

RogueHub_EventScript_CasinoNotEnough::
	msgbox RogueHub_Text_CasinoNotEnough, MSGBOX_DEFAULT
	goto RogueHub_EventScript_CasinoMenu

RogueHub_Text_DepartPrompt:
	.string "ROUTE {STR_VAR_1} is ready.\nBegin the room now?$"

RogueHub_Text_Departing:
	.string "ROUTE {STR_VAR_1} locked in!\nReach the guide at the far end.$"

RogueHub_Text_BossPrompt:
	.string "CHAPTER {STR_VAR_1} BOSS is ready.\nEnter the boss gate now?$"

RogueHub_Text_BossDeparting:
	.string "The BOSS GATE is open!\nWin to reach the next chapter.$"

RogueHub_Text_NeedPartner:
	.string "You need a partner first.\nSpeak with SCOTT in the center.$"

RogueHub_Text_CancelDeparture:
	.string "Take your time.\nThe route will remain ready.$"

RogueHub_Text_NoRouteReady:
	.string "No route is available right now.$"

RogueHub_Text_RunEnded:
	.string "That run has ended. A fresh run\nresets your party and BAG. Begin?$"

RogueHub_Text_NewRunReady:
	.string "A fresh run is ready.\nChoose a route when prepared.$"

RogueHub_Text_ChoosePartnerIntro:
	.string "Choose your first ROGUE partner.\nThis choice begins your adventure.$"

RogueHub_Text_ChooseTreecko:
	.string "Choose TREECKO?$"

RogueHub_Text_ChooseTorchic:
	.string "Choose TORCHIC?$"

RogueHub_Text_ChooseMudkip:
	.string "Choose MUDKIP?$"

RogueHub_Text_ChooseEevee:
	.string "Use your STARTER PASS for EEVEE?$"

RogueHub_Text_ChooseMysteryPartner:
	.string "Choose a mystery partner?$"

RogueHub_Text_MysteryPartnerGiven:
	.string "Your mystery partner is {STR_VAR_1}!$"

RogueHub_Text_PartnerChosen:
	.string "Your partner is ready!\nYou also received 5 POKE BALLS.$"

RogueHub_Text_PartnerReady:
	.string "Your team is ready.\nThe route guide can begin the run.$"

RogueHub_Text_RestStation:
	.string "Your team has been restored.\nGood luck on the next route!$"

RogueHub_Text_StartingBundleGiven:
	.string "START KIT: 3 POTIONS,\n2 ANTIDOTES and 1 REVIVE.$"

RogueHub_Text_Payout:
	.string "ROGUE PAYOUT: {STR_VAR_1} CHIPS!\nYour balance is now {STR_VAR_2}.$"

RogueHub_Text_CasinoWelcome:
	.string "ROGUE CHIPS: {STR_VAR_1}\nChoose a permanent reward.$"

RogueHub_Text_CasinoPurchaseSuccess:
	.string "Reward unlocked!\nYou have {STR_VAR_1} CHIPS left.$"

RogueHub_Text_CasinoAlreadyOwned:
	.string "You already own that reward.$"

RogueHub_Text_CasinoNotEnough:
	.string "You do not have enough ROGUE CHIPS.$"

RogueHub_Text_CasinoError:
	.string "That reward is unavailable.$"

RogueHub_Text_CasinoLeave:
	.string "Come back after your next run!$"

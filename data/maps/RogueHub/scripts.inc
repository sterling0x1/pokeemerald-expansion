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
	goto_if_ne VAR_RESULT, 0, RogueHub_EventScript_CheckBonusStarter
	msgbox RogueHub_Text_ChoosePartnerIntro, MSGBOX_DEFAULT
	special ChooseRogueStarter
	goto_if_eq VAR_RESULT, 0, RogueHub_EventScript_GiveTreecko
	goto_if_eq VAR_RESULT, 1, RogueHub_EventScript_GiveTorchic
	goto_if_eq VAR_RESULT, 2, RogueHub_EventScript_GiveMudkip
	goto RogueHub_EventScript_GiveTreecko

RogueHub_EventScript_GiveTreecko::
	givemon SPECIES_TREECKO, 5, ITEM_NONE
	goto RogueHub_EventScript_FinishPartnerChoice

RogueHub_EventScript_GiveTorchic::
	givemon SPECIES_TORCHIC, 5, ITEM_NONE
	goto RogueHub_EventScript_FinishPartnerChoice

RogueHub_EventScript_GiveMudkip::
	givemon SPECIES_MUDKIP, 5, ITEM_NONE
	goto RogueHub_EventScript_FinishPartnerChoice

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

RogueHub_EventScript_CheckBonusStarter::
	specialvar VAR_RESULT, RogueScript_CanChooseBonusStarter
	goto_if_eq VAR_RESULT, FALSE, RogueHub_EventScript_CheckMysteryPartner
	msgbox RogueHub_Text_BonusStarterAvailable, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RogueHub_EventScript_CheckMysteryPartner
	special ChooseRogueBonusStarter
	goto_if_eq VAR_RESULT, 0, RogueHub_EventScript_GiveBonusTreecko
	goto_if_eq VAR_RESULT, 1, RogueHub_EventScript_GiveBonusTorchic
	goto_if_eq VAR_RESULT, 2, RogueHub_EventScript_GiveBonusMudkip
	goto_if_eq VAR_RESULT, 3, RogueHub_EventScript_GiveBonusEevee
	goto RogueHub_EventScript_CheckMysteryPartner

RogueHub_EventScript_GiveBonusTreecko::
	givemon SPECIES_TREECKO, 5, ITEM_NONE
	goto RogueHub_EventScript_FinishBonusStarter

RogueHub_EventScript_GiveBonusTorchic::
	givemon SPECIES_TORCHIC, 5, ITEM_NONE
	goto RogueHub_EventScript_FinishBonusStarter

RogueHub_EventScript_GiveBonusMudkip::
	givemon SPECIES_MUDKIP, 5, ITEM_NONE
	goto RogueHub_EventScript_FinishBonusStarter

RogueHub_EventScript_GiveBonusEevee::
	givemon SPECIES_EEVEE, 5, ITEM_NONE

RogueHub_EventScript_FinishBonusStarter::
	special RogueScript_MarkBonusStarterClaimed
	msgbox RogueHub_Text_BonusStarterGiven, MSGBOX_DEFAULT

RogueHub_EventScript_CheckMysteryPartner::
	specialvar VAR_RESULT, RogueScript_GiveMysteryPartner
	goto_if_eq VAR_RESULT, FALSE, RogueHub_EventScript_CheckStartingBundle
	msgbox RogueHub_Text_MysteryPartnerGiven, MSGBOX_DEFAULT

RogueHub_EventScript_CheckStartingBundle::
	specialvar VAR_RESULT, RogueScript_GrantStartingBundle
	goto_if_eq VAR_RESULT, FALSE, RogueHub_EventScript_PartnerChoiceDone
	msgbox RogueHub_Text_StartingBundleGiven, MSGBOX_DEFAULT

RogueHub_EventScript_PartnerChoiceDone::
	msgbox RogueHub_Text_PartnerReady, MSGBOX_DEFAULT
	release
	end

RogueHub_EventScript_PartnerReady::
	goto RogueHub_EventScript_CheckBonusStarter

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
	goto_if_eq VAR_0x8004, 0, RogueHub_EventScript_CasinoStarterPassReady
	goto_if_eq VAR_0x8004, 1, RogueHub_EventScript_CasinoDeliverBundle
	goto_if_eq VAR_0x8004, 2, RogueHub_EventScript_CasinoDeliverMystery
	goto RogueHub_EventScript_CasinoMenu

RogueHub_EventScript_CasinoStarterPassReady::
	msgbox RogueHub_Text_CasinoStarterPassReady, MSGBOX_DEFAULT
	goto RogueHub_EventScript_CasinoMenu

RogueHub_EventScript_CasinoDeliverBundle::
	specialvar VAR_RESULT, RogueScript_GrantStartingBundle
	goto_if_eq VAR_RESULT, FALSE, RogueHub_EventScript_CasinoClaimLater
	msgbox RogueHub_Text_StartingBundleGiven, MSGBOX_DEFAULT
	goto RogueHub_EventScript_CasinoMenu

RogueHub_EventScript_CasinoDeliverMystery::
	specialvar VAR_RESULT, RogueScript_GiveMysteryPartner
	goto_if_eq VAR_RESULT, FALSE, RogueHub_EventScript_CasinoClaimLater
	msgbox RogueHub_Text_MysteryPartnerGiven, MSGBOX_DEFAULT
	goto RogueHub_EventScript_CasinoMenu

RogueHub_EventScript_CasinoClaimLater::
	msgbox RogueHub_Text_CasinoClaimLater, MSGBOX_DEFAULT
	goto RogueHub_EventScript_CasinoMenu

RogueHub_EventScript_CasinoAlreadyOwned::
	goto_if_eq VAR_0x8004, 0, RogueHub_EventScript_CasinoStarterAlreadyOwned
	goto_if_eq VAR_0x8004, 1, RogueHub_EventScript_CasinoDeliverOwnedBundle
	goto_if_eq VAR_0x8004, 2, RogueHub_EventScript_CasinoDeliverOwnedMystery

RogueHub_EventScript_CasinoStarterAlreadyOwned::
	specialvar VAR_RESULT, RogueScript_CanChooseBonusStarter
	goto_if_eq VAR_RESULT, TRUE, RogueHub_EventScript_CasinoStarterPassReady
	goto RogueHub_EventScript_CasinoAlreadyOwnedMessage

RogueHub_EventScript_CasinoDeliverOwnedBundle::
	specialvar VAR_RESULT, RogueScript_GrantStartingBundle
	goto_if_eq VAR_RESULT, FALSE, RogueHub_EventScript_CasinoAlreadyOwnedMessage
	msgbox RogueHub_Text_StartingBundleGiven, MSGBOX_DEFAULT
	goto RogueHub_EventScript_CasinoMenu

RogueHub_EventScript_CasinoDeliverOwnedMystery::
	specialvar VAR_RESULT, RogueScript_GiveMysteryPartner
	goto_if_eq VAR_RESULT, FALSE, RogueHub_EventScript_CasinoAlreadyOwnedMessage
	msgbox RogueHub_Text_MysteryPartnerGiven, MSGBOX_DEFAULT
	goto RogueHub_EventScript_CasinoMenu

RogueHub_EventScript_CasinoAlreadyOwnedMessage::
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
	.string "Choose EEVEE?$"

RogueHub_Text_BonusStarterAvailable:
	.string "Your STARTER PASS is ready.\nChoose a bonus partner for this run?$"

RogueHub_Text_BonusStarterGiven:
	.string "Your bonus partner has joined!$"

RogueHub_Text_MysteryPartnerGiven:
	.string "MYSTERY BONUS: {STR_VAR_1}\nhas joined this run!$"

RogueHub_Text_PartnerChosen:
	.string "Your partner is ready!\nYou also received 5 POKE BALLS.$"

RogueHub_Text_PartnerReady:
	.string "Your team is ready.\nThe route guide can begin the run.$"

RogueHub_Text_RestStation:
	.string "Your team has been restored.\nGood luck on the next route!$"

RogueHub_Text_StartingBundleGiven:
	.string "START KIT: 3 POTIONS,\n2 ANTIDOTES, 1 REVIVE\pand 5 POKE BALLS.$"

RogueHub_Text_Payout:
	.string "ROGUE PAYOUT: {STR_VAR_1} CHIPS!\nYour balance is now {STR_VAR_2}.$"

RogueHub_Text_CasinoWelcome:
	.string "ROGUE CHIPS: {STR_VAR_1}\nChoose a permanent reward.$"

RogueHub_Text_CasinoPurchaseSuccess:
	.string "Reward unlocked!\nYou have {STR_VAR_1} CHIPS left.$"

RogueHub_Text_CasinoStarterPassReady:
	.string "Speak with SCOTT in the hub to\nchoose your bonus starter.$"

RogueHub_Text_CasinoClaimLater:
	.string "Choose your first partner and leave\nparty space, then speak with SCOTT.$"

RogueHub_Text_CasinoAlreadyOwned:
	.string "You already own that reward.$"

RogueHub_Text_CasinoNotEnough:
	.string "You do not have enough ROGUE CHIPS.$"

RogueHub_Text_CasinoError:
	.string "That reward is unavailable.$"

RogueHub_Text_CasinoLeave:
	.string "Come back after your next run!$"

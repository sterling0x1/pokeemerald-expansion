RogueCasino_MapScripts::
	.byte 0

RogueCasino_EventScript_PrizeCounter::
	goto RogueHub_EventScript_CasinoHost
	end

RogueCasino_EventScript_Host::
	lock
	faceplayer
	special RogueScript_BufferChipBalance
	msgbox RogueCasino_Text_Host, MSGBOX_DEFAULT
	release
	end

RogueCasino_EventScript_SuppliesVendor::
	lock
	faceplayer
	message RogueCasino_Text_SuppliesWelcome
	waitmessage
	special RogueScript_BeginCasinoSession
	variablecoinmart RogueCasino_Supplies
	special RogueScript_EndCasinoSession
	msgbox RogueCasino_Text_SuppliesGoodbye, MSGBOX_DEFAULT
	release
	end

	.align 2
RogueCasino_Supplies:
	.2byte ITEM_POKE_BALL, 5
	.2byte ITEM_GREAT_BALL, 10
	.2byte ITEM_MASTER_BALL, 5000
	.2byte ITEM_POTION, 4
	.2byte ITEM_SUPER_POTION, 10
	.2byte ITEM_ANTIDOTE, 3
	.2byte ITEM_PARALYZE_HEAL, 3
	.2byte ITEM_AWAKENING, 3
	.2byte ITEM_BURN_HEAL, 3
	.2byte ITEM_ICE_HEAL, 3
	.2byte ITEM_REPEL, 5
	.2byte ITEM_ESCAPE_ROPE, 8
	pokemartlistend

RogueCasino_EventScript_SlotMachine0::
	setvar VAR_0x8004, 0
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_SlotMachine1::
	setvar VAR_0x8004, 1
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_SlotMachine2::
	setvar VAR_0x8004, 2
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_SlotMachine3::
	setvar VAR_0x8004, 3
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_SlotMachine4::
	setvar VAR_0x8004, 4
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_SlotMachine5::
	setvar VAR_0x8004, 5
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_SlotMachine6::
	setvar VAR_0x8004, 6
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_SlotMachine7::
	setvar VAR_0x8004, 7
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_SlotMachine8::
	setvar VAR_0x8004, 8
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_SlotMachine9::
	setvar VAR_0x8004, 9
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_SlotMachine10::
	setvar VAR_0x8004, 10
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_SlotMachine11::
	setvar VAR_0x8004, 11
	goto RogueCasino_EventScript_PlaySlots
	end

RogueCasino_EventScript_PlaySlots::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_eq VAR_TEMP_2, 0, RogueCasino_EventScript_NoChips
	specialvar VAR_RESULT, GetSlotMachineId
	playbgm MUS_GAME_CORNER, FALSE
	playslotmachine VAR_RESULT
	goto RogueCasino_EventScript_FinishGame
	end

RogueCasino_EventScript_RouletteTable1::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_eq VAR_TEMP_2, 0, RogueCasino_EventScript_NoChips
	setvar VAR_0x8004, 0
	special PlayRoulette
	end

RogueCasino_EventScript_RouletteTable2::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_eq VAR_TEMP_2, 0, RogueCasino_EventScript_NoChips
	setvar VAR_0x8004, 1
	special PlayRoulette
	end

RogueCasino_EventScript_Blackjack::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_eq VAR_TEMP_2, 0, RogueCasino_EventScript_NoChips
	msgbox RogueCasino_Text_Blackjack, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_CancelGame
	playbgm MUS_GAME_CORNER, FALSE
	special StartBlackJack
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	goto RogueCasino_EventScript_FinishGame
	end

RogueCasino_EventScript_Derby::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, 10, RogueCasino_EventScript_NotEnoughForGame
	msgbox MauvilleCity_GameCorner_Text_Derby, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_CancelGame
	setvar GAME_CORNER_VAR_WINNINGS, 0
	playbgm MUS_RG_PEWTER, FALSE
	special StartDerby
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	goto RogueCasino_EventScript_AwardWinnings
	end

RogueCasino_EventScript_Snake::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, 25, RogueCasino_EventScript_NotEnoughForGame
	msgbox MauvilleCity_GameCorner_Text_Snake, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_CancelGame
	removecoins 25
	setvar GAME_CORNER_VAR_WINNINGS, 0
	playbgm MUS_NONE, FALSE
	special StartSnake
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	goto RogueCasino_EventScript_AwardWinnings
	end

RogueCasino_EventScript_FlappyBird::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, 25, RogueCasino_EventScript_NotEnoughForGame
	msgbox MauvilleCity_GameCorner_Text_Bird, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_CancelGame
	removecoins 25
	setvar GAME_CORNER_VAR_WINNINGS, 0
	playbgm MUS_NONE, FALSE
	special StartFlappyBird
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	goto RogueCasino_EventScript_AwardWinnings
	end

RogueCasino_EventScript_BlockStacker::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, 25, RogueCasino_EventScript_NotEnoughForGame
	msgbox MauvilleCity_GameCorner_Text_BS, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_CancelGame
	removecoins 25
	setvar GAME_CORNER_VAR_WINNINGS, 0
	playbgm MUS_CASINO_PLUS_1, FALSE
	special StartBlockStacker
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	goto RogueCasino_EventScript_AwardWinnings
	end

RogueCasino_EventScript_Pachinko::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, 3, RogueCasino_EventScript_NotEnoughForGame
	msgbox MauvilleCity_GameCorner_Text_Pachinko, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_CancelGame
	playbgm MUS_RG_ROUTE1, FALSE
	callnative PlayPachinko
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	goto RogueCasino_EventScript_FinishGame
	end

RogueCasino_EventScript_PinballMeowth::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, 25, RogueCasino_EventScript_NotEnoughForGame
	msgbox MauvilleCity_GameCorner_Text_Pinball_Meowth, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_CancelGame
	removecoins 25
	setvar GAME_CORNER_VAR_WINNINGS, 0
	playbgm MUS_RG_POKE_CENTER, FALSE
	callnative PlayMeowthPinballGame
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	goto RogueCasino_EventScript_AwardWinnings
	end

RogueCasino_EventScript_PinballDiglett::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, 50, RogueCasino_EventScript_NotEnoughForGame
	msgbox MauvilleCity_GameCorner_Text_Pinball_Diglett, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_CancelGame
	removecoins 50
	setvar GAME_CORNER_VAR_WINNINGS, 0
	playbgm MUS_RG_POKE_JUMP, FALSE
	callnative PlayDiglettPinballGame
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	goto RogueCasino_EventScript_AwardWinnings
	end

RogueCasino_EventScript_PinballSeel::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, 25, RogueCasino_EventScript_NotEnoughForGame
	msgbox MauvilleCity_GameCorner_Text_Pinball_Seel, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_CancelGame
	removecoins 25
	setvar GAME_CORNER_VAR_WINNINGS, 0
	playbgm MUS_RG_SS_ANNE, FALSE
	callnative PlaySeelPinballGame
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	goto RogueCasino_EventScript_AwardWinnings
	end

RogueCasino_EventScript_PinballGengar::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, 100, RogueCasino_EventScript_NotEnoughForGame
	msgbox MauvilleCity_GameCorner_Text_Pinball_Gengar, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_CancelGame
	removecoins 100
	setvar GAME_CORNER_VAR_WINNINGS, 0
	playbgm MUS_RG_POKE_TOWER, FALSE
	callnative PlayGengarPinballGame
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	goto RogueCasino_EventScript_AwardWinnings
	end

RogueCasino_EventScript_VoltorbFlip::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_lt VAR_TEMP_2, 50, RogueCasino_EventScript_NotEnoughForGame
	msgbox MauvilleCity_GameCorner_Text_VF, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_CancelGame
	removecoins 50
	setvar GAME_CORNER_VAR_WINNINGS, 0
	playbgm MUS_RG_OAK, FALSE
	special Special_ViewVoltorbFlip
	waitstate
	goto_if_eq VAR_RESULT, 0, RogueCasino_EventScript_VoltorbLose
	goto_if_eq VAR_RESULT, 1, RogueCasino_EventScript_VoltorbContinue
	goto RogueCasino_EventScript_FinishGame
	end

RogueCasino_EventScript_VoltorbLose::
	savebgm MUS_DUMMY
	fadedefaultbgm
	msgbox VF_Come_Again_Text, MSGBOX_DEFAULT
	setvar GAME_CORNER_VAR_WINNINGS, 0
	goto RogueCasino_EventScript_FinishGame
	end

RogueCasino_EventScript_VoltorbContinue::
	buffernumberstring STR_VAR_1, GAME_CORNER_VAR_WINNINGS
	msgbox MauvilleCity_GameCorner_Text_VF_Keep_Going, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_VoltorbCashOut
	special Special_ViewVoltorbFlip
	waitstate
	goto_if_eq VAR_RESULT, 0, RogueCasino_EventScript_VoltorbLose
	goto_if_eq VAR_RESULT, 1, RogueCasino_EventScript_VoltorbContinue
	goto RogueCasino_EventScript_FinishGame
	end

RogueCasino_EventScript_VoltorbCashOut::
	addcoins GAME_CORNER_VAR_WINNINGS
	setvar GAME_CORNER_VAR_WINNINGS, 0
.if FLIP_VAR_LEVEL != 0
	setvar FLIP_VAR_LEVEL, 0
.endif
	playse SE_SHOP
	savebgm MUS_DUMMY
	fadedefaultbgm
	delay 40
	goto RogueCasino_EventScript_FinishGame
	end

RogueCasino_EventScript_GachaBasic::
	setvar VAR_0x8004, 1
	goto RogueCasino_EventScript_GachaCommon
	end

RogueCasino_EventScript_GachaGreat::
	setvar VAR_0x8004, 2
	goto RogueCasino_EventScript_GachaCommon
	end

RogueCasino_EventScript_GachaUltra::
	setvar VAR_0x8004, 3
	goto RogueCasino_EventScript_GachaCommon
	end

RogueCasino_EventScript_GachaMaster::
	setvar VAR_0x8004, 4
	goto RogueCasino_EventScript_GachaCommon
	end

RogueCasino_EventScript_GachaCommon::
	lockall
	special RogueScript_BeginCasinoSession
	checkcoins VAR_TEMP_2
	goto_if_eq VAR_TEMP_2, 0, RogueCasino_EventScript_NoChips
	playbgm MUS_RG_CINNABAR, FALSE
	special StartGacha
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PARTY, RogueCasino_EventScript_GachaMonParty
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, RogueCasino_EventScript_GachaMonPC
	goto RogueCasino_EventScript_FinishGame
	end

RogueCasino_EventScript_GachaMonParty::
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_FinishGame
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto RogueCasino_EventScript_FinishGame
	end

RogueCasino_EventScript_GachaMonPC::
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RogueCasino_EventScript_GachaTransferPC
	call Common_EventScript_NameReceivedBoxMon
RogueCasino_EventScript_GachaTransferPC::
	call Common_EventScript_TransferredToPC
	goto RogueCasino_EventScript_FinishGame
	end

RogueCasino_EventScript_AwardWinnings::
	goto_if_eq GAME_CORNER_VAR_WINNINGS, 0, RogueCasino_EventScript_FinishGame
	buffernumberstring STR_VAR_1, GAME_CORNER_VAR_WINNINGS
	msgbox RogueCasino_Text_Winnings, MSGBOX_DEFAULT
	addcoins GAME_CORNER_VAR_WINNINGS
	setvar GAME_CORNER_VAR_WINNINGS, 0
	playse SE_SHOP
	delay 40
	goto RogueCasino_EventScript_FinishGame
	end

RogueCasino_EventScript_NoChips::
	special RogueScript_EndCasinoSession
	msgbox RogueCasino_Text_NoChips, MSGBOX_DEFAULT
	releaseall
	end

RogueCasino_EventScript_NotEnoughForGame::
	special RogueScript_EndCasinoSession
	msgbox RogueCasino_Text_NotEnoughForGame, MSGBOX_DEFAULT
	releaseall
	end

RogueCasino_EventScript_CancelGame::
	special RogueScript_EndCasinoSession
	releaseall
	end

RogueCasino_EventScript_FinishGame::
	special RogueScript_EndCasinoSession
	special RogueScript_BufferChipBalance
	msgbox RogueCasino_Text_Balance, MSGBOX_DEFAULT
	releaseall
	end

RogueCasino_Text_Host:
	.string "Welcome to the ROGUE CASINO!\nYour balance is {STR_VAR_1} ROGUE CHIPS.\pEvery machine uses the same chip balance.\lVisit the counter for permanent rewards.$"

RogueCasino_Text_SuppliesWelcome:
	.string "Need supplies for your next run?\nEverything here costs ROGUE CHIPS.$"

RogueCasino_Text_SuppliesGoodbye:
	.string "Good luck on your next route!$"

RogueCasino_Text_Blackjack:
	.string "Play BLACKJACK using ROGUE CHIPS?$"

RogueCasino_Text_NoChips:
	.string "You need ROGUE CHIPS to play.\pClear routes and bosses to earn them.$"

RogueCasino_Text_NotEnoughForGame:
	.string "You don't have enough ROGUE CHIPS\nto play this game.$"

RogueCasino_Text_Winnings:
	.string "You won\n{STR_VAR_1} ROGUE CHIPS!$"

RogueCasino_Text_Balance:
	.string "You now have {STR_VAR_1} ROGUE CHIPS.$"

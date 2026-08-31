RogueRoute_MapScripts::
	.byte 0

RogueRoute_EventScript_ExitGuide::
	lock
	faceplayer
	msgbox RogueRoute_Text_RoomCompletePrompt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RogueRoute_EventScript_Stay
	specialvar VAR_RESULT, RogueScript_CompleteRoom
	goto_if_eq VAR_RESULT, FALSE, RogueRoute_EventScript_NotComplete
	specialvar VAR_RESULT, RogueScript_SelectFirstRoute
	goto_if_eq VAR_RESULT, FALSE, RogueRoute_EventScript_NotComplete
	specialvar VAR_RESULT, RogueScript_IsBossRoom
	goto_if_eq VAR_RESULT, TRUE, RogueRoute_EventScript_WarpBoss
	msgbox RogueRoute_Text_RoomComplete, MSGBOX_DEFAULT
	closemessage
	specialvar VAR_RESULT, RogueScript_GetCurrentRouteVariant
	goto_if_eq VAR_RESULT, 1, RogueRoute_EventScript_WarpRoute2
	goto_if_eq VAR_RESULT, 2, RogueRoute_EventScript_WarpRoute3
	goto_if_eq VAR_RESULT, 3, RogueRoute_EventScript_WarpRoute4
	goto_if_eq VAR_RESULT, 4, RogueRoute_EventScript_WarpRoute5
	goto_if_eq VAR_RESULT, 5, RogueRoute_EventScript_WarpRoute6
	goto_if_eq VAR_RESULT, 6, RogueRoute_EventScript_WarpRoute7
	goto_if_eq VAR_RESULT, 7, RogueRoute_EventScript_WarpRoute8
	warp MAP_ROGUE_ROUTE, 10, 18
	waitstate
	release
	end

RogueRoute_EventScript_WarpRoute2::
	warp MAP_ROGUE_ROUTE_2, 18, 11
	waitstate
	release
	end

RogueRoute_EventScript_WarpRoute3::
	warp MAP_ROGUE_ROUTE_3, 49, 12
	waitstate
	release
	end

RogueRoute_EventScript_WarpRoute4::
	warp MAP_ROGUE_ROUTE_4, 12, 7
	waitstate
	release
	end

RogueRoute_EventScript_WarpRoute5::
	warp MAP_ROGUE_ROUTE_5, 16, 37
	waitstate
	release
	end

RogueRoute_EventScript_WarpRoute6::
	warp MAP_ROGUE_ROUTE_6, 37, 11
	waitstate
	release
	end

RogueRoute_EventScript_WarpRoute7::
	warp MAP_ROGUE_ROUTE_7, 26, 35
	waitstate
	release
	end

RogueRoute_EventScript_WarpRoute8::
	warp MAP_ROGUE_ROUTE_8, 4, 9
	waitstate
	release
	end

RogueRoute_EventScript_WarpBoss::
	msgbox RogueRoute_Text_BossAhead, MSGBOX_DEFAULT
	closemessage
	warp MAP_ROGUE_BOSS_ARENA, 12, 8
	waitstate
	release
	end

RogueRoute_EventScript_Stay::
	msgbox RogueRoute_Text_Stay, MSGBOX_DEFAULT
	release
	end

RogueRoute_EventScript_NotComplete::
	msgbox RogueRoute_Text_NotComplete, MSGBOX_DEFAULT
	release
	end

RogueRoute_Text_RoomCompletePrompt:
	.string "You reached the route exit!\nContinue deeper into the run?$"

RogueRoute_Text_RoomComplete:
	.string "Route clear!\nMoving deeper into the run.$"

RogueRoute_Text_BossAhead:
	.string "Route clear!\nThe chapter BOSS awaits.$"

RogueRoute_Text_Stay:
	.string "Explore as long as you need.$"

RogueRoute_Text_NotComplete:
	.string "The room cannot be completed yet.$"

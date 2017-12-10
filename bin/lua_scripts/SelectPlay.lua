--
if gSwitchNum["state"] == "normal" then
	if not player.realNumExist(gSwitchNum["normal"]) and player.realNumExist(gSwitchNum["backup"]) then
		print("change to backup")
		gSwitchNum["state"] = "backup"
	end
elseif gSwitchNum["state"] == "backup" then
	if not player.realNumExist(gSwitchNum["backup"]) and player.realNumExist(gSwitchNum["normal"]) then
		print("change to normal")
		gSwitchNum["state"] = "normal"
	end
end
gNormalPlay = gOppoConfig.NorPlay
-- 先注掉，在需要时可以开启使用
if (not IS_TEST_MODE) and USE_SWITCH and gSwitchNum["state"] == "backup" then
	dofile("./lua_scripts/opponent/Backup.lua")
else
	dofile("./lua_scripts/opponent/"..OPPONENT_NAME..".lua")	
end

function RunRefScript(name)
	local filename = "./lua_scripts/play/Ref/"..name..".lua"
	dofile(filename)
end

function SelectRefPlay()
	local curRefMsg = vision:GetCurrentRefereeMsg()
	if curRefMsg == "" then
		return false
	end
	--ourTimeout
	if curRefMsg == "ourIndirectKick" and gLastRefMsg ~= curRefMsg then
		gIsOurIndirectKickExit = false
	end
	if curRefMsg == "ourIndirectKick" and gIsOurIndirectKickExit then
		return false
	end
	if gLastRefMsg ~= curRefMsg or curRefMsg == "gameStop" then
		ball.updateRefMsg()
	end
	if curRefMsg=="ourTimeout" then
		gCurrentPlay = "OurTimeoutV3"
	end
	RunRefScript(curRefMsg)
	gLastRefMsg = curRefMsg
	return true
end

function SelectBayesPlay()
	--gCurrentPlay = "Nor_PassAndShoot"
	-- gCurrentPlay = "Nor_MorePass"

	--add by twj 14/5/12
	--------------------
	if gNormalPlay == "NormalPlayNew" or gNormalPlay == "NormalPlayDefend" or gNormalPlay == "NormalPlayOneState" 
	or gNormalPlay == "NormalPlayMark" or gNormalPlay == "NormalPlayPass" then
		world:clearBallStateCouter()
	end
	---------------------
	gCurrentPlay = gNormalPlay
	ResetPlay(gCurrentPlay)
end

if SelectRefPlay() then
	-- or NeedExit(gCurrentPlay)不添加会出问题！！
	--print("gCurrentPlay",gCurrentPlay)
	--print("gLastPlay",gLastPlay)
	if gCurrentPlay ~= gLastPlay or NeedExit(gCurrentPlay) then
		ResetPlay(gCurrentPlay)
		if vision:GetCurrentRefereeMsg() == "ourIndirectKick" then
			gIsOurIndirectKickExit = true
		end
		print("New Play: "..gCurrentPlay)
	else
--~ 		print("TimerCounter: ", gTimeCounter)
	end
else
	if IS_TEST_MODE then
		local externPlay = httpServer:getCurPlay()
		if  externPlay ~= "" then
			if gExternStopCycle < 120 then
				for role,value in pairs(gRoleNum) do
					if value ~= nil and role ~= "Fronter" and role ~="Tier" and role ~= "Breaker" then
						if role == "Kicker" then
							httpServer:setToWebData("Assister",value)
						else
							httpServer:setToWebData(role,value)
						end
					end
				end
				print(gCurrentPlay, gCurrentState)
				gExternStopCycle = gExternStopCycle + 1
				dofile("./lua_scripts/play/Ref/GameStop.lua")
				if gCurrentPlay ~= gLastPlay then
					print("Rest Received Play: ", gCurrentPlay)
					ResetPlay(gCurrentPlay)
					print("1",vision:Cycle(),gCurrentState)
				end
			else
				for role,value in pairs(gRoleNum) do
					if value ~= nil and role ~= "Fronter" and role ~="Tier" and role ~= "Breaker" and role ~= "Kicker" then
						httpServer:setToWebData(role,value)
					end
				end
				dofile("./lua_scripts/generated/"..externPlay..".lua")
				gCurrentPlay = externPlay
				if gCurrentPlay ~= gLastPlay then
					print("Rest Received Play: ", gCurrentPlay)
					ResetPlay(gCurrentPlay)
				elseif NeedExit(gCurrentPlay) then
					if gExternExitCycle < 60 then
						gExternExitCycle = gExternExitCycle + 1
					else
						httpServer:clearCurPlay()
						gExternStopCycle = 0
						gExternExitCycle = 0
					end
				end
			end
		else
			if gLastPlay == "" or NeedExit(gCurrentPlay) then
				gCurrentPlay = gTestPlay
			end
			if gCurrentPlay ~= gLastPlay or
				NeedExit(gCurrentPlay) then
				ResetPlay(gCurrentPlay)
			end
		end
	else
		--add by twj 14/5/12
		------------------------------------------
		if gNormalPlay == "NormalPlayNew" or gNormalPlay == "NormalPlayDefend" or gNormalPlay == "NormalPlayOneState" 
		or gNormalPlay == "NormalPlayMark" or gNormalPlay == "NormalPlayPass" then
			world:setBallHandler(gRoleNum["Leader"])
			gLastBallStatus=gCurrentBallStatus
			gCurrentBallStatus=world:getBallStatus(vision:Cycle(),gRoleNum["Leader"])
			gLastFieldArea=gCurrentFieldArea
			gCurrentFieldArea=cond.judgeFieldArea()
		end
		--------------------------------------------

		if gLastPlay == "" or NeedExit(gCurrentPlay) then
			print("Play: "..gCurrentPlay.." Exit!!")
			SelectBayesPlay()
		end
	end
end

gLastPlay = gCurrentPlay

RunPlay(gCurrentPlay)

local stateFile=io.open(".\\LOG\\"..gStateFileNameString..".txt","a")
stateFile:write(vision:Cycle().." "..gCurrentState.." "..gCurrentPlay.." me:"..bestPlayer:getOurBestPlayer()
	.." he:"..bestPlayer:getTheirBestPlayer().."\n")
stateFile:close()

--print( world:getSuitSider())
--print("vel"..enemy.vel(bestPlayer:getTheirGoalie()):y()*2)
--print(bestPlayer:getTheirGoalie())
--print("hello",bestPlayer:getOurBestPlayer())
--print(gCurrentState,vision:Cycle())
--print("ball",ball.posX(),ball.valid())
--print("raw",vision:RawBall():X(),vision:RawBall():Valid())
--print(vision:getBallVelStable(),vision:ballVelValid())
debugEngine:gui_debug_msg(CGeoPoint:new_local(455, 60),gCurrentState)
--world:drawReflect(gRoleNum["Tier"])
--defenceInfo:setNoChangeFlag()
--print(vision:Cycle()..vision:GetCurrentRefereeMsg(),vision:gameState():gameOn())

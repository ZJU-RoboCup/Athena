local Change_Pos = function ()
  local tmpX
  local tmpY

  local UpdatePos = function ()
    tmpX = player.posX("Leader")
    tmpY = player.posY("Leader")
  end

  local pos_ChangeLeader = function ()
    UpdatePos()
    if tmpX < 175 then
    	return CGeoPoint:new_local(80, 0)
    elseif tmpY < 0 then
    	return CGeoPoint:new_local(320, -200)
    else
    	return CGeoPoint:new_local(320, 200)
    end
  end

  local pos_ChangeMiddle = function ()
    UpdatePos()
    if tmpX < 175 then
    	return CGeoPoint:new_local(320, -200)
    elseif tmpY < 0 then
    	return CGeoPoint:new_local(320, 200)
    else
    	return CGeoPoint:new_local(320, -200)
    end
  end

  local pos_ChangeDefender = function ()
    UpdatePos()
    if tmpX < 175 then
    	return CGeoPoint:new_local(320, 200)
    else
    	return CGeoPoint:new_local(80, 0)
    end
  end

  return pos_ChangeLeader, pos_ChangeMiddle, pos_ChangeDefender
end
local Leader_POS, Middle_POS, Defender_POS = Change_Pos()

gPlayTable.CreatePlay{

firstState = "getball",
["getball"] = {
	switch = function()
		if ball.posX() > 390 or ball.posX() < 50 then
			return "stop"
		elseif bufcnt(player.toTargetDist("Middle") < 10, 60) then
			return "firstpass"
		end
	end,
	Leader   = task.staticGetBall("Middle"),
	Middle   = task.goCmuRush(Middle_POS, player.toBallDir, 600, flag.allow_dss),
	Defender = task.goCmuRush(Defender_POS, player.toBallDir, 600, flag.allow_dss),
	match  = "{LMD}"
},

["firstpass"] = {
	switch = function()
		if ball.posX() > 390 or ball.posX() < 50 then
			return "stop"
		elseif player.kickBall("Leader") then
			return "pass2"
		end
	end,
	Leader   = task.pass("Middle",450),
	Middle   = task.goCmuRush(Middle_POS, player.toBallDir, 600, flag.allow_dss),
	Defender = task.goCmuRush(Defender_POS, player.toBallDir, 600, flag.allow_dss),
	match  = "{LMD}"
},

["pass1"] = {
	switch = function()
		if ball.posX() > 390 or ball.posX() < 50 then
			return "stop"
		elseif player.kickBall("Leader") then
			return "pass2"
		end
	end,
	Leader   = task.receiveChip("Middle",180),
	Middle   = task.goCmuRush(Middle_POS, player.toBallDir, 600, flag.allow_dss),
	Defender = task.goCmuRush(Defender_POS, player.toBallDir, 600, flag.allow_dss),
	match  = "{LMD}"
},

["pass2"] = {
	switch = function()
		if ball.posX() > 390 or ball.posX() < 50 then
			return "stop"
		elseif player.kickBall("Middle") then
			return "pass3"
		end
	end,
	Leader   = task.goCmuRush(Leader_POS, player.toBallDir, 600, flag.allow_dss),
	Middle   = task.receiveChip("Defender",180),
	Defender = task.goCmuRush(Defender_POS, player.toBallDir, 600, flag.allow_dss),
	match  = "{LMD}"
},

["pass3"] = {
	switch = function()
		if ball.posX() > 390 or ball.posX() < 50 then
			return "stop"
		elseif player.kickBall("Defender") then
			return "pass1"
		end
	end,
	Leader   = task.goCmuRush(Leader_POS, player.toBallDir, 600, flag.allow_dss),
	Middle   = task.goCmuRush(Middle_POS, player.toBallDir, 600, flag.allow_dss),
	Defender = task.receiveChip("Leader",180),
	match  = "{LMD}"
},

["stop"] = {
	switch = function()
		return "stop"
	end,
	Leader   = task.stop(),
	Middle   = task.stop(),
	Defender = task.stop(),
	match  = "{LMD}"
},

name = "TestTrianglePass",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
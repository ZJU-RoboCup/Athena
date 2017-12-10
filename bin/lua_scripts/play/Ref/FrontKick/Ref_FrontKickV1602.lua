--by hzy 6/28/2016
--根据FrontkickV2修改,针对MRL不防开球车的脚本

local function GeneratePos(x,y,anti)
	local ifanti
	if anti == nil or anti == false then
		ifanti = 1
	else
		ifanti = -1
	end
	return function()
		return CGeoPoint:new_local(ball.posX()+x,ifanti*(ball.posY()+ball.syntY()*y))
	end
end
local FACE_POS = ball.refSyntYPos(CGeoPoint:new_local(0,-300))
local WAIT_POS = CGeoPoint:new_local(-250, 150)
local POS = {
	WAIT_POS,
	GeneratePos(-50,0,true),
	GeneratePos(0,-100,true),
	GeneratePos(20,-170,false)
}

local REACH_POS = ball.refSyntYPos(CGeoPoint:new_local(param.pitchLength/2.0,-10))

local SHOOT_POS = GeneratePos(-150,-50)
gPlayTable.CreatePlay{

firstState = "startball",

["startball"] = {
  switch = function ()
    if  bufcnt(true, 80) then
      return "goOneJam"
    end
  end,
  Assister = task.staticGetBall(FACE_POS,false),
  Leader   = task.goCmuRush(POS[2], dir.compensate(ball.pos()), 600, flag.allow_dss),
  Special  = task.goCmuRush(POS[1], dir.compensate(ball.pos()), 600, flag.allow_dss),
  Middle   = task.rightBack(),
  Defender = task.leftBack(),
  Goalie   = task.goalie(),
  match    = "{ALS}[MD]"
},

["goOneJam"] = {
  switch = function ()
    if  bufcnt(true, 80) then
      return "goTwoJam"
    end
  end,
  Assister = task.staticGetBall(FACE_POS,false),
  Leader   = task.goCmuRush(POS[4], dir.compensate(ball.pos()), 600, flag.allow_dss),
  Special  = task.goCmuRush(POS[2], dir.compensate(ball.pos()), 600, flag.allow_dss),
  Middle   = task.goCmuRush(POS[1], dir.compensate(ball.pos()), 600, flag.allow_dss),
  Defender = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{ASL}[MD]"
},

["goTwoJam"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Leader") < 20,2,50) then
      return "goPass"
    end
  end,
  Assister = task.staticGetBall(FACE_POS,false),
  Leader   = task.goCmuRush(SHOOT_POS,_, 600, flag.allow_dss),
  Special  = task.goCmuRush(POS[3], dir.compensate(ball.pos()), 600, flag.allow_dss),
  Middle   = task.goCmuRush(POS[2], dir.compensate(ball.pos()), 600, flag.allow_dss),
  Defender = task.goCmuRush(POS[1], dir.compensate(ball.pos()), 600, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{ASLMD}"
},
["goPass"] = {
	switch = function()
		if player.kickBall("Assister") then
			return "shoot"
		elseif bufcnt(true,60) then
			return "exit"
		end
	end,
	Assister = task.goAndTurnKick(SHOOT_POS,300),
	Leader   = task.goCmuRush(SHOOT_POS,_, 600, flag.allow_dss),
	Special  = task.goCmuRush(POS[4], dir.compensate(ball.pos()), 600, flag.allow_dss),
  	Middle   = task.goCmuRush(POS[3], dir.compensate(ball.pos()), 600, flag.allow_dss),
  	Defender = task.goCmuRush(POS[2], dir.compensate(ball.pos()), 600, flag.allow_dss),
  	Goalie   = task.goalie(),
  	match    = "{ASLMD}"
},
["shoot"] = {
	switch = function()
		if bufcnt(player.kickBall("Leader"),1,100) then
			return "finish"
		end
	end,
	Assister = task.defendMiddle("Leader"),
	Leader   = task.InterTouch(_,REACH_POS,750),
	Special  = task.goCmuRush(POS[4], dir.compensate(ball.pos()), 600, flag.allow_dss),
  	Middle   = task.leftBack(),
  	Defender = task.rightBack(),--task.goCmuRush(POS[2], dir.compensate(ball.pos()), 600, flag.allow_dss),
  	Goalie   = task.goalie(),
  	match    = "{ASLMD}"
},
name = "Ref_FrontKickV1602",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

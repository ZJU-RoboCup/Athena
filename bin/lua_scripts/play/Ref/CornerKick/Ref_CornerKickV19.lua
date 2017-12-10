--测试对方防守的角球脚本 by Fantasy in Hefei



--[1]
local SPECIAL_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(70, 200))
local DEFENDER_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(70, 100))
local LEADER_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(70, -100))
local MIDDLE_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(70, -200))


--[2]
local SPECIAL_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(200, 150))
local DEFENDER_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(200, 75))
local LEADER_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(200, -75))
local MIDDLE_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(200, -150))

--[3]
local SPECIAL_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(390, 140))
local DEFENDER_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(300, 80))
local LEADER_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(300, -80))
local MIDDLE_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(390, -140))


--[4]
local SPECIAL_POS_4 = ball.refAntiYPos(CGeoPoint:new_local(0, 0))
local DEFENDER_POS_4 = ball.refAntiYPos(CGeoPoint:new_local(100, 0))
local LEADER_POS_4 = ball.refAntiYPos(CGeoPoint:new_local(200, 0))
local MIDDLE_POS_4 = ball.refAntiYPos(CGeoPoint:new_local(300, 0))

--[5]
local SPECIAL_POS_5 = ball.refAntiYPos(CGeoPoint:new_local(180, 270))
local DEFENDER_POS_5 = ball.refAntiYPos(CGeoPoint:new_local(180, -270))
local LEADER_POS_5 = ball.refAntiYPos(CGeoPoint:new_local(330, 60))
local MIDDLE_POS_5 = ball.refAntiYPos(CGeoPoint:new_local(330, -60))

gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20 and
                player.toTargetDist("Goalie") < 20, 30, 180) then
        return "run2"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(LEADER_POS_1, _, 600, flag.allow_dss),
    Special  = task.goCmuRush(SPECIAL_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(MIDDLE_POS_1, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(DEFENDER_POS_1, _, 600, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{A}{LDMS}"
  },

  ["run2"] = {
    switch = function ()
       if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20 and
                player.toTargetDist("Goalie") < 20, 30, 180) then
        return "run3"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(LEADER_POS_2, _, 600, flag.allow_dss),
    Special  = task.goCmuRush(SPECIAL_POS_2, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(MIDDLE_POS_2, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(DEFENDER_POS_2, _, 600, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  ["run3"] = {
    switch = function ()
        if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20 and
                player.toTargetDist("Goalie") < 20, 30, 180) then
        return "run4"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(LEADER_POS_3, _, 600, flag.allow_dss),
    Special  = task.goCmuRush(SPECIAL_POS_3, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(MIDDLE_POS_2, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(DEFENDER_POS_2, _, 600, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  ["run4"] = {
    switch = function ()
       if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20 and
                player.toTargetDist("Goalie") < 20, 60, 180) then
        return "run5"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(LEADER_POS_2, _, 600, flag.allow_dss),
    Special  = task.goCmuRush(SPECIAL_POS_2, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(MIDDLE_POS_3, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(DEFENDER_POS_3, _, 600, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  ["run5"] = {
    switch = function ()
     if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20 and
                player.toTargetDist("Goalie") < 20, 60, 180) then
        return "run6"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(LEADER_POS_4, _, 600, flag.allow_dss),
    Special  = task.goCmuRush(SPECIAL_POS_4, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(MIDDLE_POS_4, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(DEFENDER_POS_4, _, 600, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

   ["run6"] = {
    switch = function ()
      if bufcnt(player.kickBall("Assister"), "fast", 100)  then
        return "finish"
      end
    end,
    Assister = task.goAndTurnChip(CGeoPoint:new_local(420,0), 300),
    Leader   = task.goCmuRush(LEADER_POS_5, _, 600, flag.allow_dss),
    Special  = task.goCmuRush(SPECIAL_POS_5, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(MIDDLE_POS_5, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(DEFENDER_POS_5, _, 600, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  name = "Ref_CornerKickV19",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}
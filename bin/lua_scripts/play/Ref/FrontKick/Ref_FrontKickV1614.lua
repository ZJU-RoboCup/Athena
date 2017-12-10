-- 模拟2013年CMU进MRL的定位球，注意只能针对角球区域（>280）
-- by zhyaic 2014-07-14
-- 2014-07-20 yys 改
-- cornerkickv11 挑球版
local MP1 = {
  CGeoPoint:new_local(335,115),
  CGeoPoint:new_local(325,90)
}
local MP2 = {
  CGeoPoint:new_local(355,95),
  CGeoPoint:new_local(345,70)
}
local MP3 = {
  CGeoPoint:new_local(370,105),
  CGeoPoint:new_local(340,30)
}
local FINAL_SHOOT_POS = CGeoPoint:new_local(70,270)--(130, 240)
local SHOOT_POS = ball.refSyntYPos(FINAL_SHOOT_POS)
-- pos for leader
local LEADER_POS1 = ball.refSyntYPos(CGeoPoint:new_local(-120, 270)) -- (-150,190)
local LEADER_POS2 = ball.refSyntYPos(FINAL_SHOOT_POS)

-- pos for middle
local MIDDLE_POS1 = ball.refAntiYPos(CGeoPoint:new_local(350, 260))

-- pos for special
local SPECIAL_POS1 = ball.refSyntYPos(MP1[1])--ball.refAntiYPos(CGeoPoint:new_local(200, 210))
local SPECIAL_POS2 = ball.refSyntYPos(MP2[1])--ball.refAntiYPos(CGeoPoint:new_local(57, 200))
local SPECIAL_POS3 = ball.refSyntYPos(MP3[1])
local SPECIAL_POS4 = ball.refSyntYPos(CGeoPoint:new_local(300, 0))
local ASSISTER_POS = ball.refSyntYPos(CGeoPoint:new_local(340,160))
-- pos for defender
-- change for cmu
-- local DEFENDER_POS1 = ball.refSyntYPos(CGeoPoint:new_local(225,217))
local DEFENDER_POS1 = ball.refSyntYPos(MP1[2])--ball.refAntiYPos(CGeoPoint:new_local(300, 50))
local DEFENDER_POS2 = ball.refSyntYPos(MP2[2])
local DEFENDER_POS3 = ball.refSyntYPos(MP3[2])
local USE_FAKE_DIR = true

-- 这里比较特殊
local PASS_POS = CGeoPoint:new_local(130, -260)--(130,-205)
local TMP_FACE_POS = USE_FAKE_DIR and CGeoPoint:new_local(450, -300) or CGeoPoint:new_local(0, 200)
local FACE_POS = USE_FAKE_DIR and CGeoPoint:new_local(450, -300) or PASS_POS--pos.passForTouch(CGeoPoint:new_local(150,-170))
local REACH_POS = ball.refSyntYPos(CGeoPoint:new_local(param.pitchLength/2.0,15))
local USE_CHIP = true

local HALF = gOppoConfig.IfHalfField

-- 让挑球的速度更快一点
gPlayTable.CreatePlay{

  firstState = "start",

  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 30 and 
                player.toTargetDist("Special") < 30, 10) then
        return "tmp"
      end
    end,
    Assister = task.staticGetBall(TMP_FACE_POS),
    Leader   = task.goCmuRush(LEADER_POS1, _, 500, flag.allow_dss),
    Special  = task.goCmuRush(SPECIAL_POS1, _, 500, flag.allow_dss),
    Middle   = task.goCmuRush(MIDDLE_POS1, _, 500, flag.allow_dss),
    Defender = task.goCmuRush(DEFENDER_POS1, _, 500, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{A}[LSMD]"
  },

  ["tmp"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Assister") < 10, 40, 180) then
        return "rush"
      end
    end,
    Assister = task.staticGetBall(FACE_POS),
    Leader   = task.goCmuRush(LEADER_POS1, _, 500, flag.allow_dss),
    Special  = task.goCmuRush(SPECIAL_POS2, _, 500, flag.allow_dss),
    Middle   = HALF and task.stop() or task.singleBack(),
    Defender = task.goCmuRush(DEFENDER_POS2, _, 500, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALSMD}"
  },

  ["rush"] = {
    switch = function ()
      local tmpDist = USE_FAKE_DIR and 95 or 75 -- 75
      if bufcnt(true, 80) then
        return "pass"
      end
    end,
    Assister = task.slowGetBall(PASS_POS),
    Leader   = USE_CHIP and task.goCmuRush(LEADER_POS1, _, 500, flag.allow_dss) or task.goCmuRush(SHOOT_POS, _, 400),
    Special  = task.goCmuRush(SPECIAL_POS2, _, 500, flag.allow_dss),
    Middle   = HALF and task.stop() or task.singleBack(),
    Defender = task.goCmuRush(DEFENDER_POS2, _, 500, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALSMD}"
  },

  ["pass"] = {
    switch = function ()
      if bufcnt(player.kickBall("Assister") or ball.velMod() > 50, 1, 180) then
        return "fix"
      end
    end,
    Assister = USE_CHIP and task.chipPass(PASS_POS, 120) or task.goAndTurnKickByGetBall(PASS_POS, 500),
    Leader   = USE_CHIP and task.goCmuRush(LEADER_POS1, _, 500, flag.allow_dss) or task.goCmuRush(SHOOT_POS, _, 300),
    Special  = task.goCmuRush(SPECIAL_POS2, _, 500, flag.allow_dss),
    Middle   = HALF and task.stop() or task.singleBack(),
    Defender = task.goCmuRush(DEFENDER_POS2, _, 500, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALDSM}"
  },
  ["fix"] = {
    switch = function ()
      if bufcnt(true, 30, 180) then
        return "shoot"
      end
    end,
    Assister = task.goCmuRush(ASSISTER_POS, _, 500, flag.allow_dss),
    Leader   = USE_CHIP and task.goCmuRush(LEADER_POS2, _, 500, flag.allow_dss) or task.goCmuRush(SHOOT_POS, _, 300),
    Special  = task.goCmuRush(SPECIAL_POS2, _, 500, flag.allow_dss),
    Middle   = HALF and task.stop() or task.singleBack(),
    Defender = task.goCmuRush(DEFENDER_POS2, _, 500, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALDSM}"
  },
  ["shoot"] = {
    switch = function ()
      if bufcnt(player.kickBall("Leader"), 1, 120) then
        return "exit"
      end
    end,
    Assister = task.stop(),
    Leader   = task.InterTouch(LEADER_POS2,REACH_POS),--task.InterTouch(LEADER_POS2,REACH_POS),--task.waitTouchNew(LEADER_POS2),
    Special  = task.goCmuRush(SPECIAL_POS3, _, 500, flag.allow_dss),
    Middle   = HALF and task.stop() or task.singleBack(),
    Defender = task.goCmuRush(DEFENDER_POS3, _, 500, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALDSM}"
  },

  name = "Ref_FrontKickV1614",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}
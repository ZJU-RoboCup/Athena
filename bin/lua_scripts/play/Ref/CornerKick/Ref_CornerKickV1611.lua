-- 模拟2013年CMU进MRL的定位球，注意只能针对角球区域（>280）
-- by zhyaic 2014-07-14
-- 2014-07-20 yys 改
-- cornerkickv11 挑球版
local MP1 = {
  CGeoPoint:new_local(335,115),
  CGeoPoint:new_local(325,90)
}
local MP2 = {
  CGeoPoint:new_local(340,110),
  CGeoPoint:new_local(330,85)
}
local MP3 = {
  CGeoPoint:new_local(355,120),
  CGeoPoint:new_local(325,45)
}
local FINAL_SHOOT_POS = CGeoPoint:new_local(100,-50)--(130, 240)
local SHOOT_POS = ball.refSyntYPos(FINAL_SHOOT_POS)
-- pos for leader
local LEADER_POS1 = ball.refSyntYPos(CGeoPoint:new_local(0, -100)) -- (-150,190)
local LEADER_POS2 = ball.refSyntYPos(FINAL_SHOOT_POS)

-- pos for middle
local MIDDLE_POS1 = ball.refAntiYPos(CGeoPoint:new_local(300, 0))

-- pos for special
local SPECIAL_POS1 = ball.refAntiYPos(CGeoPoint:new_local(200, 210))
local SPECIAL_POS2 = ball.refAntiYPos(CGeoPoint:new_local(57, 200))
local SPECIAL_POS3 = ball.refAntiYPos(CGeoPoint:new_local(300, 0))
local SPECIAL_POS4 = ball.refSyntYPos(CGeoPoint:new_local(300, 0))
local ASSISTER_POS = ball.refSyntYPos(CGeoPoint:new_local(340,160))
-- pos for defender
-- change for cmu
-- local DEFENDER_POS1 = ball.refSyntYPos(CGeoPoint:new_local(225,217))
local DEFENDER_POS1 = ball.refAntiYPos(CGeoPoint:new_local(300, -100))
local USE_FAKE_DIR = false

-- 这里比较特殊
local PASS_POS = CGeoPoint:new_local(130, 0)--(130,-205)
local TMP_FACE_POS = USE_FAKE_DIR and CGeoPoint:new_local(450, -300) or CGeoPoint:new_local(0, 200)
local FACE_POS = USE_FAKE_DIR and CGeoPoint:new_local(450, -300) or PASS_POS--pos.passForTouch(CGeoPoint:new_local(150,-170))
local REACH_POS = ball.refSyntYPos(CGeoPoint:new_local(param.pitchLength/2.0,-15))
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
    Middle   = HALF and task.stop() or task.singleBack("Leader"),
    Defender = task.goCmuRush(DEFENDER_POS1, _, 500, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALSMD}"
  },
  ["rush"] = {
    switch = function ()
      local tmpDist = USE_FAKE_DIR and 95 or 75 -- 75
      if bufcnt(player.toTargetDist("Leader") < tmpDist, "fast", 180) then
        return "pass"
      end
    end,
    Assister = task.slowGetBall(PASS_POS),-- or task.staticGetBall(PASS_POS),
    Leader   = USE_CHIP and task.goCmuRush(LEADER_POS1, _, 500, flag.allow_dss) or task.goCmuRush(SHOOT_POS, _, 400),
    Special  = task.goCmuRush(SPECIAL_POS3, _, 500, flag.allow_dss),
    Middle   = task.goCmuRush(MIDDLE_POS1, _, 500, flag.allow_dss),
    Defender = task.goCmuRush(DEFENDER_POS1, _, 500, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALSMD}"
  },

  ["pass"] = {
    switch = function ()
      if bufcnt(player.kickBall("Assister") or ball.velMod() > 50, 1, 180) then
        return "fix"
      end
    end,
    Assister = USE_CHIP and task.chipPass(PASS_POS, 220) or task.goAndTurnKick(PASS_POS, 500),
    Leader   = USE_CHIP and task.goCmuRush(LEADER_POS1, _, 500, flag.allow_dss) or task.goCmuRush(SHOOT_POS, _, 300),
    Special  = HALF and task.stop() or task.leftBack(),
    Middle   = HALF and task.stop() or task.rightBack(),
    Defender = task.goCmuRush(DEFENDER_POS1, _, 500, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALD}[SM]"
  },
  ["fix"] = {
    switch = function ()
      if bufcnt(true, 20, 120) then
        return "shoot"
      end
    end,
    Assister = task.stop(),
    Leader   = USE_CHIP and task.goCmuRush(LEADER_POS1, _, 500, flag.allow_dss) or task.goCmuRush(SHOOT_POS, _, 300),
    Special  = HALF and task.stop() or task.leftBack(),
    Middle   = HALF and task.stop() or task.rightBack(),
    Defender = task.defendMiddle(),
    Goalie   = task.goalie(),
    match    = "{ALD}[SM]"
  },

  ["shoot"] = {
    switch = function ()
      if bufcnt(player.kickBall("Leader"), 1, 120) then
        return "exit"
      end
    end,
    Assister = task.stop(),
    Leader   = USE_CHIP and task.InterTouch(SHOOT_POS) or task.waitTouchNew(SHOOT_POS,REACH_POS),--task.touch(),
    Special  = HALF and task.stop() or task.leftBack(),
    Middle   = HALF and task.stop() or task.rightBack(),
    Defender = task.defendMiddle(),
    Goalie   = task.goalie(),
    match    = "{ALD}[SM]"
  },
  name = "Ref_CornerKickV1611",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}
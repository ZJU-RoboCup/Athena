-- 模拟2013年CMU进MRL的定位球，注意只能针对角球区域（>280）
-- by zhyaic 2014-07-14
-- 2014-07-20 yys 改
local x = -50
local y = 270
local FINAL_SHOOT_POS = CGeoPoint:new_local(x,y)--(130, 240)
local SHOOT_POS = ball.refSyntYPos(FINAL_SHOOT_POS)
-- pos for leader
local LEADER_POS1 = ball.refSyntYPos(CGeoPoint:new_local(x, y)) -- (-150,190)
local LEADER_POS2 = ball.refSyntYPos(FINAL_SHOOT_POS)

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

-- pos for middle
local MIDDLE_POS1 = ball.refAntiYPos(CGeoPoint:new_local(350, 260))

-- pos for special
local SPECIAL_POS1 = ball.refAntiYPos(CGeoPoint:new_local(200, 210))
local SPECIAL_POS2 = ball.refAntiYPos(CGeoPoint:new_local(57, 200))
local SPECIAL_POS3 = ball.refAntiYPos(CGeoPoint:new_local(300, 0))

-- pos for defender
-- change for cmu
-- local DEFENDER_POS1 = ball.refSyntYPos(CGeoPoint:new_local(225,217))
local DEFENDER_POS1 = ball.refAntiYPos(CGeoPoint:new_local(300, 50))
local USE_FAKE_DIR = false

-- 这里比较特殊
local PASS_POS = CGeoPoint:new_local(x, -y)--(130,-205)
local TMP_FACE_POS = USE_FAKE_DIR and CGeoPoint:new_local(450, -300) or CGeoPoint:new_local(0, 200)
local FACE_POS = USE_FAKE_DIR and CGeoPoint:new_local(450, -300) or PASS_POS--pos.passForTouch(CGeoPoint:new_local(150,-170))
local REACH_POS = ball.refSyntYPos(CGeoPoint:new_local(param.pitchLength/2.0,-15))
local USE_CHIP = true

local HALF = false

-- 让挑球的速度更快一点
gPlayTable.CreatePlay{

  firstState = "start",

  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 30 and 
                player.toTargetDist("Special") < 30, 30) then
        return "tmp"
      end
    end,
    Assister = task.staticGetBall(TMP_FACE_POS),
    Leader   = task.goCmuRush(LEADER_POS1, _, 500, DSS_FLAG),
    Special  = task.goCmuRush(SPECIAL_POS1, _, 500,DSS_FLAG),
    Middle   = task.goCmuRush(MIDDLE_POS1, _, 500, DSS_FLAG),
    Defender = task.goCmuRush(DEFENDER_POS1, _, 500,DSS_FLAG),
    Goalie   = task.goalie(),
    match    = "{A}[LSMD]"
  },

  ["tmp"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Assister") < 10, 40, 180) then
        return "rush"
      end
    end,
    Assister = task.staticGetBall(TMP_FACE_POS),
    Leader   = task.goCmuRush(LEADER_POS1, _, 500,   DSS_FLAG),
    Special  = task.goCmuRush(SPECIAL_POS2, _, 500,  DSS_FLAG),
    Middle   = task.goCmuRush(MIDDLE_POS1, _, 500,   DSS_FLAG),
    Defender = task.goCmuRush(DEFENDER_POS1, _, 500, DSS_FLAG),
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
    Assister = task.staticGetBall(TMP_FACE_POS),--USE_CHIP and task.slowGetBall(PASS_POS) or task.staticGetBall(TMP_FACE_POS),-- or task.staticGetBall(PASS_POS),
    Leader   = USE_CHIP and task.goCmuRush(LEADER_POS1, _, _, DSS_FLAG) or task.goCmuRush(LEADER_POS1, _, 400),
    Special  = task.goCmuRush(SPECIAL_POS3, _, 500,  DSS_FLAG),
    Middle   = task.goCmuRush(MIDDLE_POS1, _, 500,   DSS_FLAG),
    Defender = task.goCmuRush(DEFENDER_POS1, _, 500, DSS_FLAG),
    Goalie   = task.goalie(),
    match    = "{ALSMD}"
  },

  ["pass"] = {
    switch = function ()
      if bufcnt(player.kickBall("Assister") or ball.velMod() > 50, 1, 180) then
        if USE_CHIP then
            return "fix"
          else
            return "shoot"
          end
      end
    end,
    Assister = USE_CHIP and task.chipPass(PASS_POS, 140) or task.goAndTurnKick(PASS_POS, 400),
    Leader   = task.goCmuRush(LEADER_POS1, _, _, DSS_FLAG),--task.InterTouch(SHOOT_POS),
    Special  = HALF and task.stop() or task.leftBack(),
    Middle   = HALF and task.stop() or task.rightBack(),
    Defender = task.goCmuRush(DEFENDER_POS1, _, 500, DSS_FLAG),
    Goalie   = task.goalie(),
    match    = "{ALD}[SM]"
  },
  ["fix"] = {
    switch = function ()
      if bufcnt(true, 20) then
        return "shoot"
      end
    end,
    Assister = task.stop(),
    Leader   = task.goCmuRush(LEADER_POS1, _, _, DSS_FLAG),
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
    Leader   = task.waitTouchNew(SHOOT_POS),--USE_CHIP and task.InterTouch(SHOOT_POS) or task.waitTouchNew(SHOOT_POS),--task.touch(),
    Special  = HALF and task.stop() or task.leftBack(),
    Middle   = HALF and task.stop() or task.rightBack(),
    Defender = task.defendMiddle(),
    Goalie   = task.goalie(),
    match    = "{ALD}[SM]"
  },

  name = "TestFuckBackDef",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}
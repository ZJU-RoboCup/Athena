-- Parsian 打 MRL 的定位球，挡拆

local WAIT_POS1 = CGeoPoint:new_local(120, 0)
local WAIT_POS2 = CGeoPoint:new_local(70, 0)    -- 挡拆车 Defender
local WAIT_POS3 = CGeoPoint:new_local(0, 0)     -- 射门车 Leader
local PULL_POS1 = ball.refAntiYPos(CGeoPoint:new_local(350, 160))

local FAKE_POS1   = ball.refAntiYPos(CGeoPoint:new_local(265, 200))
local FAKE_POS2   = ball.refAntiYPos(CGeoPoint:new_local(105, 130)) -- 挡拆车
local FAKE_POS3   = ball.refAntiYPos(CGeoPoint:new_local(60, 100)) -- 射门车-如果对方跟车更得很紧，把23点距离调近一点
local PULL_POS2   = ball.refAntiYPos(CGeoPoint:new_local(400, -180))

local SHOOT_POS         = CGeoPoint:new_local(320, 194) -- 射门车
local ANTI_SHOOT_POS    = ball.refAntiYPos(SHOOT_POS)
local SHOOT_ASSIST_POS1 = ball.refAntiYPos(CGeoPoint:new_local(330, -200))
local SHOOT_ASSIST_POS2 = ball.refAntiYPos(CGeoPoint:new_local(250, 120))   -- 挡拆车
local PULL_POS3         = ball.refAntiYPos(CGeoPoint:new_local(350, -160))

gPlayTable.CreatePlay{

firstState = "startBall",

["startBall"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Defender") < 30 and
          player.toTargetDist("Middle")       < 30 and 
          player.toTargetDist("Leader")       < 30, "normal") then
        return "fakeGoto"
    end
  end,
  Assister = task.staticGetBall( CGeoPoint:new_local(0, 0) ),
  Middle   = task.goCmuRush( WAIT_POS1 ),
  Defender = task.goCmuRush( WAIT_POS2 ),
  Leader   = task.goCmuRush( WAIT_POS3 ),
  Special  = task.goCmuRush( PULL_POS1 ),
  Goalie   = task.goalie(),
  match    = "{A}{DL}(MS)" -- 固定住？
},


["fakeGoto"] = {
  switch = function ()
      if bufcnt(player.toTargetDist("Defender")   < 30 and
                player.toTargetDist("Leader")     < 30, 5) then
          return "pickAndRoll"
      end
    end,
    Assister = task.staticGetBall( CGeoPoint:new_local(450, 0) ),
    Middle   = task.goCmuRush( FAKE_POS1 ),
    Defender = task.goCmuRush( FAKE_POS2 ),
    Leader   = task.goCmuRush( FAKE_POS3 ),
    Special  = task.goCmuRush( PULL_POS2 ),
    Goalie   = task.goalie(),
    match    = "{A}{DL}(MS)"
},


["pickAndRoll"] = {
  switch = function ()
      if bufcnt(player.kickBall("Assister") , 1) then
          return "shoot"
      end
    end,  
  Assister = task.chipPass(SHOOT_POS, 200),
  Middle   = task.goCmuRush( SHOOT_ASSIST_POS1 ),
  Defender = task.goCmuRush( SHOOT_ASSIST_POS2 ),
  Leader   = task.goCmuRush( ANTI_SHOOT_POS ),
  Special  = task.goCmuRush( PULL_POS3 ),
  Goalie   = task.goalie(),
  match = "{A}{DL}(MS)"
},

-- ["fix"] = {
--   switch = function ()
--       if bufcnt(true, 10) then
--           return "shoot"
--       end
--     end,  
--   Assister = task.defendMiddle(),
--   Middle   = task.goCmuRush( SHOOT_ASSIST_POS1 ),
--   Defender = task.goCmuRush( SHOOT_ASSIST_POS2 ),
--   Leader   = task.goCmuRush( ANTI_SHOOT_POS ),
--   Special  = task.goCmuRush( PULL_POS3 ),
--   Goalie   = task.goalie(),
--   match = "{A}{DL}(MS)"
-- },


["shoot"] = {
  switch = function ()
      if bufcnt(player.kickBall("Middle"), 1) then
          return "exit"
      end
    end,  
  Assister = task.defendMiddle(),
  Middle   = task.leftBack(),
  Defender = task.waitTouchNew(),
  Leader   = task.leftBack(),
  Special  = task.rightBack(),
  Goalie   = task.goalie(),
  match = "{A}{DL}(MS)"
},

name = "TestPickAndRollMRL",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999

}
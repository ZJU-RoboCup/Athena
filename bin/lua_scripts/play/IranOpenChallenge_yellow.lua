-- 伊朗赛挑战赛黄车脚本,需将lua在线测试的代码注释掉,否则闭包有问题
-- by yys 2014-03-25

local FOUR_POS_1 = {
  CGeoPoint:new(101, -152),
  CGeoPoint:new(101, -105),
  CGeoPoint:new(101, -152),
  CGeoPoint:new(101, -200)
}

local FOUR_POS_2 = {
  CGeoPoint:new(0, -200),
  CGeoPoint:new(0, -152),
  CGeoPoint:new(0, -105),
  CGeoPoint:new(0, -152)
}

local FOUR_POS_3 = {
  CGeoPoint:new(-101, -105),
  CGeoPoint:new(-101, -152),
  CGeoPoint:new(-101, -200),
  CGeoPoint:new(-101, -152)
}

local FOUR_POS_4 = {
  CGeoPoint:new(101, 105),
  CGeoPoint:new(101, 152),
  CGeoPoint:new(101, 200),
  CGeoPoint:new(101, 152)
}

local FOUR_POS_5 = {
  CGeoPoint:new(0, 200),
  CGeoPoint:new(0, 152),
  CGeoPoint:new(0, 105),
  CGeoPoint:new(0, 152)
}

local FOUR_POS_6 = {
  CGeoPoint:new(-101, 152),
  CGeoPoint:new(-101, 105),
  CGeoPoint:new(-101, 152),
  CGeoPoint:new(-101, 200)
}

gPlayTable.CreatePlay{

firstState = "state1",

["state1"] = {
  switch = function()
  end,
  Assister = {RunMultiPos{pos = FOUR_POS_1, dist = 5}},
  Special  = {RunMultiPos{pos = FOUR_POS_2, dist = 5}},
  Leader   = {RunMultiPos{pos = FOUR_POS_3, dist = 5}},
  Middle   = {RunMultiPos{pos = FOUR_POS_4, dist = 5}},
  Defender = {RunMultiPos{pos = FOUR_POS_5, dist = 5}},
  Goalie   = {RunMultiPos{pos = FOUR_POS_6, dist = 5}},
  match    = "{ASLMD}"
},

name = "IranOpenChallenge_yellow",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}
-- 伊朗赛挑战赛蓝车脚本(我方)
-- 在C++的 GotoPosition.cpp 和 RRTPathPlaner.cpp 中需注释掉躲避我方禁区的代码,需将lua在线测试的代码注释掉,否则闭包有问题
-- by yys 2014-03-25
local FLAG = bit:_or(flag.slowly, flag.dodge_ball)
local DSS = bit:_or(FLAG, flag.allow_dss)

local MACC = 250 -- 最大加速度, 需实物看效果

local MULTI_POS_1 = {
CGeoPoint:new(270, 0),
CGeoPoint:new(210, 130),
CGeoPoint:new(-210, 160),
CGeoPoint:new(-270, 0),
CGeoPoint:new(-210, -130),
CGeoPoint:new(210, -160)
}

local MULTI_POS_2 = {
CGeoPoint:new(-270, 0),
CGeoPoint:new(-210, 130),
CGeoPoint:new(210, 160),
CGeoPoint:new(270, 0),
CGeoPoint:new(210, -130),
CGeoPoint:new(-210, -160)
}

local MULTI_FLAG_1 = { DSS, DSS, DSS, DSS, DSS, DSS }
local MULTI_FLAG_2 = { DSS, DSS, DSS, DSS, DSS, DSS }

local MULTI_ACC_1 = { 0, 0, MACC, 0, 0, MACC }
local MULTI_ACC_2 = { 0, 0, MACC, 0, 0, MACC }

gPlayTable.CreatePlay{

firstState = "state1",

["state1"] = {
  switch = function()
  end,
  Leader  = task.goSimplePos(CGeoPoint:new_local(235, 0), math.pi),                       -- 固定车1
  Goalie  = task.goSimplePos(CGeoPoint:new_local(-235, 0), math.pi),                      -- 固定车2
  Kicker  = {RunMultiPosV2{pos = MULTI_POS_1, flag = MULTI_FLAG_1, acc = MULTI_ACC_1}},   -- 任务车1
  Tier    = {RunMultiPosV2{pos = MULTI_POS_2, flag = MULTI_FLAG_2, acc = MULTI_ACC_2}},   -- 任务车2
  match   = "{L}"
},

name = "IranOpenChallenge",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}
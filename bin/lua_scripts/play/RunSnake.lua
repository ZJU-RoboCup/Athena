local sttPos = function(n,TOTAL)
    assert(n<TOTAL)
    return CGeoPoint:new_local(240,30*(n-TOTAL/2.0))
end
local STEP = 30
local X = {  8, 8, 8, 8, 8, 8, 9, 9, 9,10,
            10,10, 9, 9, 8, 8, 7, 6, 6, 5,
             5, 4, 4, 3, 3, 2, 2, 2, 3, 4,
             4, 5, 5, 6, 6, 7, 8, 8, 8, 7,
             7, 6, 6, 5, 5, 4, 3, 3, 3, 4,
             5, 5, 6, 6, 7, 7, 8, 8, 8, 7,
             6, 6, 5, 5, 4, 4, 3, 2, 2, 2,
             3, 3, 4, 4, 5, 5, 6, 6, 7, 8,
             8, 9, 9,10,10,10, 9, 9, 9, 8}
local Y = { -2,-1, 0, 1, 2, 3, 3, 4, 5, 5,
             6, 7, 7, 8, 8, 9, 9, 9, 8, 8,
             7, 7, 6, 6, 5, 5, 4, 3, 3, 3,
             4, 4, 5, 5, 6, 6, 6, 5, 4, 4,
             3, 3, 2, 2, 1, 1, 1, 0,-1,-1,
            -1,-2,-2,-3,-3,-4,-4,-5,-6,-6,
            -6,-5,-5,-4,-4,-3,-3,-3,-4,-5,
            -5,-6,-6,-7,-7,-8,-8,-9,-9,-9,
            -8,-8,-7,-7,-6,-5,-5,-4,-3,-3}
local count = 0
local update = function()
    count = (count + 1) % 90
end
local p = function(n)
    return function()
        local index = (count + n) % 90 + 1
        return CGeoPoint:new_local(X[index]*STEP,Y[index]*STEP)
    end
end
local d = function(n)
    return function()
        local index = (count + n) % 90 + 1
        local next_index = (count + n + 1) % 90 + 1
        return (CGeoPoint:new_local(X[next_index]*STEP,Y[next_index]*STEP) - CGeoPoint:new_local(X[index]*STEP,Y[index]*STEP)):dir()
    end
end
gPlayTable.CreatePlay{

firstState = "start",
["start"] = {
    switch = function()
        if bufcnt(true,180) then
            update()
            return "run"
        end
    end,
    Assister = task.goSpeciPos(sttPos(0,5),math.pi/2),
    Leader   = task.goSpeciPos(sttPos(1,5),math.pi/2),
    Special  = task.goSpeciPos(sttPos(2,5),math.pi/2),
    Middle   = task.goSpeciPos(sttPos(3,5),math.pi/2),
    Defender = task.goSpeciPos(sttPos(4,5),math.pi/2),
    match = "(ALSMD)"
},
["run"] = {
    switch = function()
        if bufcnt(true,20) then
            update()
            return "run"
        end
    end,
    Assister = task.goSimplePos(p(0),d(0)),
    Leader   = task.goSimplePos(p(1),d(1)),
    Special  = task.goSimplePos(p(2),d(2)),
    Middle   = task.goSimplePos(p(3),d(3)),
    Defender = task.goSimplePos(p(4),d(4)),
    match = "{ALSMD}"
},
name = "RunSnake",
applicable ={
    exp = "a",
    a = true
},
attribute = "attack",
timeout = 99999
}
--[[
0
1
2
3
4
--]]
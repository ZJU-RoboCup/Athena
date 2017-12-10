gPlayTable.CreatePlay{

firstState = "halt",
switch = function()
	return "halt"
end,

["halt"] = {
	["Leader"]   = task.stop(),
	["Special"]  = task.stop(),
	["Assister"] = task.stop(),
	["Defender"] = task.stop(),
	["Middle"]   = task.stop(),
	["Goalie"]   = task.stop(),
	match = "[LSADM]"
},

name = "Ref_HaltV1",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

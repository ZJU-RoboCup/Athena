gPlayTable.CreatePlay{

firstState = "mark",

["mark"] = {
	switch = function()
		return "mark"
	end,
	Kicker = task.marking("First"),
	match = ""
},


name = "TestMarking",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}


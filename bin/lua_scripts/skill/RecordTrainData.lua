local rec_file = file.open("RecordTrainData.txt", "w")

function RecordTrainData(task)
	execute = function(runner)
		file.write(rec_file, "%f %f %f\n", player.posX(runner), player.posY(runner), player.dir(runner))
		return StopRobot(runner)
	end

	matchPos = function()
		return CGeoPoint:new_local(0,0)
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "RecordTrainData",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

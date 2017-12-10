function Goalie2015(task)
	local mflag = task.flag or 0

	execute = function(runner)
		return CGoalie2015(runner, mflag)
	end

	matchPos = function()
		return pos.goaliePos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "Goalie2015",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

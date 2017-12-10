function Goalie2013(task)
	local mflag = task.flag or 0

	execute = function(runner)
		return CGoalie2013(runner, mflag)
	end

	matchPos = function()
		return pos.goaliePos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "Goalie2013",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

function PenaltyKick2013(task)
	execute = function(runner)
		return CPenaltyKick2013(runner)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "PenaltyKick2013",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

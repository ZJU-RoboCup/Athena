function PenaltyKick2017V1(task)
	execute = function(runner)
		return CPenaltyKick2017V1(runner)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "PenaltyKick2017V1",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

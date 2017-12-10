function PenaltyGoalie(task)
	local mflag = task.flag or 0
	
	execute = function(runner)	
		return CPenaltyGoalie(runner, mflag)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "PenaltyGoalie"
}

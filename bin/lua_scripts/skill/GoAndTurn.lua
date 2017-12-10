function GoAndTurn(task)
	local mflag = task.flag or 0
	
	execute = function(runner)	
		return CGoAndTurn(runner)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "GoAndTurn",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
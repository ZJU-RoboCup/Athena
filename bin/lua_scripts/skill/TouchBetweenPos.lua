function TouchBetweenPos(task)
	local mdir
	local mposA
	local mposB
	
	execute = function(runner)
		if type(task.posA) == "function" then
			mposA = task.posA()
		else
			mposA = task.posA
		end
		if type(task.posB) == "function" then
			mposB = task.posB()
		else
			mposB = task.posB
		end
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		return CTouchBetweenPos(runner,mposA:x(), mposA:y(), mposB:x(),mposB:y(),mdir)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "TouchBetweenPos",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

function GoAvoidShootLine(task)
	local mpos
	local mdir
	local mflag = task.flag or 0
	local msender = task.sender or 0

	matchPos = function()
		if type(task.pos) == "function" then
			mpos = task.pos()
		else
			mpos = task.pos
		end
		return mpos
	end
	
	execute = function(runner)										
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end

		if type(task.pos) == "function" then
			mpos = task.pos()
		else
			mpos = task.pos
		end

		if type(task.sender) == "string" then
			msender = player.num(task.sender)
		end
		
		return CGoAvoidShootLine(runner, mpos:x(), mpos:y(), mdir, mflag, msender)
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "GoAvoidShootLine",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

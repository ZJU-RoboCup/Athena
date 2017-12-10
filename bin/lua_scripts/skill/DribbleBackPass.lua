function DribbleBackPass(task)
	local mdir
	local mflag = task.flag or 0
	local mrole = task.srole or ""

	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		
		return CDribbleBackPass(runner, mdir,mflag)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "DribbleBackPass",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

function ShootBall(task)
	local mdir
	local mpre
	local mflag = task.flag or 0
	
	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		
		mpre = task.pre(runner)		
		return CShootBall(runner, mdir, mpre,mflag)
	end

	matchPos = function()
		return pos.playerBest()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "ShootBall"
}

function WaitKick2016(task)
	local mdir
	local mangle
	local mpos
	
	execute = function(runner)
		if task.pos==nil then
			mpos = player.pos(runner)
		elseif type(task.pos) == "function" then
			mpos = task.pos()
		else
			mpos = task.pos
		end

		--dir为车身朝向
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end

		--print(mdir,mangle)
		return CWaitTouch2016(runner, mpos:x(), mpos:y(), mdir)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "WaitKick2016",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

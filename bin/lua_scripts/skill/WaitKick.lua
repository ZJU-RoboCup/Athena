function WaitKick(task)
	local mdir
	local mangle
	local mpos
	
	execute = function(runner)
		if type(task.pos) == "function" then
			mpos = task.pos()
		elseif task.pos == nil then
			mpos = player.pos(runner)
		else
			mpos = task.pos
		end
		--dir为车身朝向
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		--angle为目标点相关
		if type(task.angle) == "function" then
			mangle = task.angle()
		else
			mangle = task.angle
		end
		--print(mdir,mangle)
		return CWaitTouch(runner,mpos:x(), mpos:y(), mdir,mangle)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "WaitKick",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

function NoneZeroRush(task)
	local mpos
	local mdir
	local mflag   = task.flag or 0
	local msender = task.sender or 0
	local mrole   = task.srole or ""
	local macc    = task.acc or 0
	local mrec    = task.rec or 0 --mrec判断是否吸球  gty 2016-6-15
	local mvel    = task.vel or 0
	matchPos = function()
		if type(task.pos) == "function" then
			mpos = task.pos()
		else
			mpos = task.pos
		end
		return mpos
	end
	
	execute = function(runner)
		if runner>0 and runner <= param.maxPlayer then
			if mrole ~= "" then
				CRegisterRole(runner, mrole)
			end
		else
			print("Error runner in NoneZeroRush", runner)
		end
		if type(task.pos) == "function" then
			mpos = task.pos()
		else
			mpos = task.pos
		end

		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end

		if type(task.sender) == "string" then
			msender = player.num(task.sender)
		end

		if type(task.vel) == "function" then
			mvel = task.vel(runner)
		else
			mvel = task.vel
		end
		
		return CNoneZeroGoCmuRush(runner, mpos:x(), mpos:y(), mdir, mflag, msender, macc, mrec, mvel:x(), mvel:y())
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "NoneZeroRush",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
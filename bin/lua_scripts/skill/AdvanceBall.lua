function AdvanceBall(task)
	local mpos
	local mflag     = task.flag or 0
	local mrole     = task.srole or ""
	local tandem = function ()
		return gRoleNum[task.tandem]
	end

	execute = function(runner)
		if runner > 0 and runner <= param.maxPlayer then
			if mrole ~= "" then
				CRegisterRole(runner, mrole)
			end
		else
			print("Error runner in AdvanceBall", runner)
		end
		local tandemNum    = tandem() and tandem() or 0
		return CAdvanceBall(runner, mflag, tandemNum)  -- return CAdvanceBallOld(runner, mflag)
	end

	matchPos = function()
		if type(task.pos) == "function" then
			mpos = task.pos()
		else
			mpos = task.pos
		end

		return mpos
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "AdvanceBall",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

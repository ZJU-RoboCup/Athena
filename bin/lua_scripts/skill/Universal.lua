function Universal(task)
	local mpos
	local mflag     = task.flag or 0
	local mrole     = task.srole or ""
	local tandem = function ()
		return gRoleNum[task.tandem]
	end

	execute = function(runner)
		local decision = world:getAttackDecision(vision:Cycle(),gRoleNum["Leader"])
		-- print(decision)
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
	--[[return function()
		local execute = function()
			print(world:getAttackDecision(vision:Cycle(),gRoleNum["Leader"]))
			return task.advance()
		end

		return execute
	end--]]
	--[[local mexe, mpos
	local decision = function()
		return world:getAttackDecision(vision:Cycle(),gRoleNum["Leader"])
	end
	execute = function(runner)
		local idecision = decision()
		print(idecision)
		if idecision == "Shoot" then
			return task.advance()
		elseif idecision == "LightKick" then
			return task.flatPass(CGeoPoint:new_local(450, 0),500)
		elseif idecision == "LightChip" then

		elseif idecision == "Pass" then
			return task.flatPass(CGeoPoint:new_local(0, 0),300)
		else
			return task.advance()
		end
	end
	return execute--]]
	--[[execute = function(runner)
		print(decision())
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
	end--]]
end

gSkillTable.CreateSkill{
	name = "Universal",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

function StaticGetBall(task)
	local mpos
	local mdir
	local mflag = task.flag or 0

	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end

		return CStaticGetBall(runner, mdir, mflag)
	end

	matchPos = function()
		-- print("Kicker: ",gRoleFixNum["Kicker"][1])
		if gRoleFixNum["Kicker"][1] ~= nil then
			local strategy_num = CGetStrategyNum(gRoleFixNum["Kicker"][1])
			if strategy_num >= 1 and strategy_num <= 6 then
				mpos = player.pos(strategy_num)
			else
				if type(task.pos) == "function" then
					mpos = task.pos()
				else
					mpos = task.pos
				end
			end
		else
			if type(task.pos) == "function" then
				mpos = task.pos()
			else
				mpos = task.pos
			end
		end
		return mpos
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "StaticGetBall",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

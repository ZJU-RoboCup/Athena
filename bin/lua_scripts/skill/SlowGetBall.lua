function SlowGetBall(task)
	local mdir
	
	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		return CSlowGetBall(runner, mdir)
	end

	matchPos = function()
		if gRoleFixNum["Kicker"][1] ~= nil then
			local strategy_num = CGetStrategyNum(gRoleFixNum["Kicker"][1])
			if strategy_num >= 1 and strategy_num <= 6 then
				return player.pos(strategy_num)
			else
				return ball.pos()
			end
		end

		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "SlowGetBall",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

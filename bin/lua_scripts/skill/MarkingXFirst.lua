function MarkingXFirst(task)
	local firstNum = 1
	
	execute = function(runner)
		return CMarkingXFirstNum(runner, firstNum)
	end

	matchPos = function()
		local minX = 9999
		local bestNum = bestPlayer:getTheirBestPlayer()
		for i=1,6 do
			if enemy.valid(i) then
				if enemy.posX(i) < minX and i ~= bestNum then
					minX = enemy.posX(i)
					firstNum = i
				end
			end
		end
		local x, y = CMarkingXFirstPos(firstNum)
		return CGeoPoint:new_local(x,y)
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "MarkingXFirst",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
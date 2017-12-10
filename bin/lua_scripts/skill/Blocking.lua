local function PriToNum(str)
	local num
	if str == "Zero" then
		num = 0
	elseif str == "First" then
		num = 1
	elseif str == "Second" then
		num = 2
	elseif str == "Third" then
		num = 3
	elseif str == "Fourth" then
		num = 4
	else
		print("Error Priority in Blocking Skill!!!!!")
	end

	return num
end

function Blocking(task)
	local mflag = task.flag or 0
	local mpri
	local mpos

	execute = function(runner)
		if task.pri == nil then
			print("No Priority in Blocking Skill!!!!!")
		elseif type(task.pri) == "string" then
			mpri = PriToNum(task.pri)
		end

		for rolename, num in pairs(gRoleNum) do
			if num == runner and IsRoleActive(rolename) then
				mpos = gRolePos[rolename]
				break
			end
		end
		
		return CBlocking(runner, mpri, mpos:x(), mpos:y(), mflag)
	end

	matchPos = function()
		local x, y = CGetBlockingPos(mpri)
		return CGeoPoint:new_local(x,y)
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "Blocking",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
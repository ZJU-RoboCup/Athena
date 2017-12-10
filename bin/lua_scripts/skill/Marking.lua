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
		print("Error Priority in Marking Skill!!!!!")
	end

	return num
end

function Marking(task)
	local mflag = task.flag or 0
	local mfront = task.front or false
	local mpri
	local mdir
	local mpos

	execute = function(runner)
		if task.pri == nil then
			print("No Priority in Marking Skill!!!!!")
		elseif type(task.pri) == "string" then
			mpri = PriToNum(task.pri)
		end

		for rolename, num in pairs(gRoleNum) do
			if num == runner and IsRoleActive(rolename) then
				mpos = gRolePos[rolename]
				break
			end
		end


		if task.dir == nil then 
			mdir = 999
		elseif type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end

		return CMarking(runner, mpri, mpos:x(), mpos:y(), mflag, mfront,mdir)
	end

	matchPos = function()
		local x, y = CGetMarkingPos(mpri, mfront)
		return CGeoPoint:new_local(x,y)
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "Marking",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
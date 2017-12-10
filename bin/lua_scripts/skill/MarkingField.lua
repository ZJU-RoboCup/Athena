function MarkingField(task)
	local mposA
	local mposB
	local mstyle
	local mdir

	execute = function(runner)
		-- if type(task.posA) == "function" then
		-- 	mposA = task.posA()
		-- else
		-- 	mposA = task.posA
		-- end

		-- if type(task.posB) == "function" then
		-- 	mposB = task.posB()
		-- else
		-- 	mposB = task.posB
		-- end
		if type(task.style) == "function" then
			mstyle = task.style(runner)
		else
			mstyle = task.style
		end

		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		
		return CMarkingField(runner,mposA:x(), mposA:y(), mposB:x(),mposB:y(), mdir, mstyle)
	end

	matchPos = function()
		if type(task.posA) == "function" then
			mposA = task.posA()
		else
			mposA = task.posA
		end

		if type(task.posB) == "function" then
			mposB = task.posB()
		else
			mposB = task.posB
		end

		return CGeoPoint:new_local((mposA:x()+mposB:x())/2.0,(mposA:y()+mposB:y())/2.0)
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "MarkingField",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
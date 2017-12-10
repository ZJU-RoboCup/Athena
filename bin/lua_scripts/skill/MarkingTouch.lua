
function MarkingTouch(task)
	local mdir
	local mflag = task.flag or 0
	local mLeftUp
	local mRightDown
	local mAreaNum
	execute = function(runner)
		if type(task.leftUpPos) == "function" then
			mLeftUp = task.leftUpPos()
		else
			mLeftUp = task.leftUpPos
		end
		if type(task.rightDownPos) == "function" then
			mRightDown = task.rightDownPos()
		else
			mRightDown = task.rightDownPos
		end
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		return CMarkingTouch(runner, mdir,mLeftUp:x(),mLeftUp:y(),mRightDown:x(),mRightDown:y(),mflag)
	end

	matchPos = function()
		if type(task.leftUpPos) == "function" then
			mLeftUp = task.leftUpPos()
		else
			mLeftUp = task.leftUpPos
		end
		if type(task.rightDownPos) == "function" then
			mRightDown = task.rightDownPos()
		else
			mRightDown = task.rightDownPos
		end
		if type(task.areaNum) == "function" then
			mAreaNum = task.areaNum()
		else
			mAreaNum = task.areaNum
		end
		
		local x, y 
		if mflag == flag.accurately then
			x,y= CGetMarkingTouchPos(mAreaNum,mLeftUp:x(),mLeftUp:y(),mRightDown:x(),mRightDown:y(),1)
		elseif  mflag == 0 then
			x,y= CGetMarkingTouchPos(mAreaNum,mLeftUp:x(),mLeftUp:y(),mRightDown:x(),mRightDown:y(),0)
		end
		return CGeoPoint:new_local(x,y)
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "MarkingTouch",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}

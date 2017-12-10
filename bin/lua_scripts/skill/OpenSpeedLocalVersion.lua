function OpenSpeedLocalVersion(task)
  local mspeedX = task.x or 0
  local mspeedY = task.y or 0
  local mspeedW = task.w or 0
  
  execute = function(runner)
    if type(task.x) == "function" then
      mspeedX = task.x()
    end

    if type(task.y) == "function" then
      mspeedY = task.y()
    end

    if type(task.w) == "function" then
      mspeedW = task.w()
    end

    if task.dir ~= nil then
      local mdir
      if type(task.dir) == "function" then
        mdir = task.dir(runner)
      else
        mdir = task.dir
      end
      mspeedX = task.mod * math.cos(mdir)
      mspeedY = task.mod * math.sin(mdir)
    end

    return COpenSpeed(runner, mspeedX, mspeedY, mspeedW)
  end

  matchPos = function()
    return ball.pos()
  end

  return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "OpenSpeedLocalVersion",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}
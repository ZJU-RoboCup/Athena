function DribbleTurn(task)
  local finalDir = task.fDir
  local pre = task.pre

  execute = function(runner)
    return CDribbleTurn(runner, finalDir, pre)
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
  name = "DribbleTurn",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}

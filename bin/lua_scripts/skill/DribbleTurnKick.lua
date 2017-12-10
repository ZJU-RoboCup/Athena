function DribbleTurnKick(task)
  local finalDir  
  local rotVel = 3
  local kickPower = 1200

  execute = function(runner)
    if type(task.fDir) == "function" then
      finalDir = task.fDir(runner)
    else
      finalDir = task.fDir
    end

    if type(task.rotV) == "function" then
      rotVel = task.rotV(runner)
    else
      rotVel = task.rotV
    end

    if type(task.kPower) == "function" then
      kickPower = task.kPower(runner)
    else
      kickPower = task.kPower
    end

    return CDribbleTurnKick(runner, finalDir,rotVel,kickPower)
  end

  matchPos = function()
    return ball.pos()
  end

  return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "DribbleTurnKick",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}

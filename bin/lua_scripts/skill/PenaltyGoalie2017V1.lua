function PenaltyGoalie2017V1(task)
  local mflag = task.flag or 0

  execute = function(runner)
    return CPenaltyGoalie2017V2(runner, mflag)
  end

  matchPos = function()
    return ball.pos()
  end

  return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "PenaltyGoalie2017V1"
}

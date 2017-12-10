function PenaltyGoalieV2(task)
  local mflag = task.flag or 0

  execute = function(runner)
    return CPenaltyGoalieV2(runner, mflag)
  end

  matchPos = function()
    return ball.pos()
  end

  return execute, matchPos
end

gSkillTable.CreateSkill{
  name = "PenaltyGoalieV2"
}

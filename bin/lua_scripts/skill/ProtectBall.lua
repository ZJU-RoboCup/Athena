function ProtectBall(task)
  local mpos
  local mflag = task.flag or 0
  local mrole = task.srole or ""

  execute = function(runner)
    if runner>0 and runner <= param.maxPlayer then
      if mrole ~= "" then
        CRegisterRole(runner, mrole)
      end
    else
      print("Error runner in ProtectBall", runner)
    end

    return CProtectBall(runner, mflag)  -- return CProtectBallOld(runner, mflag)
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
  name = "ProtectBall",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}

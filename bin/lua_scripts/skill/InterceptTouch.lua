function InterceptTouch(task)
  local mpos
  local mdir
  local mpower
  local mbuffer
  local mkick = task.kick
  execute = function(runner)
    if task.pos == nil then
      mpos = CGeoPoint:new_local(999, 999)
    elseif type(task.pos) == "function" then
      mpos = task.pos(runner)
    else
      mpos = task.pos
    end

    if type(task.dir) == "function" then
      mdir = task.dir(runner)
    else
      mdir = task.dir
    end

    if type(task.power) == "function" then
      mpower = task.power(runner)
    else
      mpower = task.power
    end

    if type(task.buffer) == "function" then
      mbuffer = task.buffer()
    else
      mbuffer = task.buffer
    end

    local useChip = (mkick == kick.chip()) and 1 or 0
    local testMode = task.testMode
    return CInterceptTouch(runner, mpos:x(), mpos:y(), mdir, mpower, mbuffer, useChip, testMode)
  end

  matchPos = function()
    return ball.pos()
  end

  return execute, matchPos
end

gSkillTable.CreateSkill {
  name = "InterceptTouch",
  execute = function (self)
    print("This is in skill"..self.name)
  end
}

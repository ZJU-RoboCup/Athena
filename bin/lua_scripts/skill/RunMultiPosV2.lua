-- by yys 2014-03-27
-- 传入多个点，多个flag, 多个acc, 若acc为0,则会使用默认加速度
function RunMultiPosV2(task)
  local currentIndex = 1
  local mpos         = {}
  local mdir
  local mflag
  local macc
  local msender      = task.sender
  local mclose       = task.close
  local mdist        = task.dist or 20
  local lastCycle    = 0

  execute = function(runner)
    if player.pos(runner):dist(mpos[currentIndex]) < mdist then
      local nextIndex = (currentIndex) % table.getn(mpos) + 1
      if mclose == nil or mclose then
        currentIndex = nextIndex
      else
        if nextIndex > currentIndex then
          currentIndex = nextIndex
        end
      end
    end

    if type(task.dir) == "function" then
      mdir = task.dir(runner)
    else
      mdir = player.toPointDir(mpos[currentIndex], runner)
    end

    if type(task.sender) == "string" then
      msender = player.num(task.sender)
    end

    if type(task.flag) == "table" then
      mflag = (task.flag)[currentIndex]
    else
      mflag = 0
    end

    if type(task.acc) == "table" then
      macc = (task.acc)[currentIndex]
    else
      macc = 0
    end

    lastCycle = vision:Cycle()
    return SmartGotoPos(runner, mpos[currentIndex]:x(), mpos[currentIndex]:y(), mdir, mflag, msender, macc)
  end

  matchPos = function()
    if vision:Cycle() - lastCycle > 6 then
      currentIndex = 1
    end
    for i = 1, table.getn(task.pos) do
      if type(task.pos[i]) == "function" then
        mpos[i] = task.pos[i]()
      else
        mpos[i] = task.pos[i]
      end
    end
    return mpos[currentIndex]
  end

  return execute, matchPos
end

 gSkillTable.CreateSkill{
  name = "RunMultiPosV2",
  execute = function (self)
    print("This is in skill"..self.name)
  end
 }
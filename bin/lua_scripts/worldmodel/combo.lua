module(..., package.seeall)

-- 对task.lua更高一层的抽象,包括一些通过标志位决定的task和一些组合的task
-- by yys 2014/04/28

---------------------------------2014/04/28---------------------------------------------------------

-- 守门员:使用时可以当作goCmuRush,标志位可在相应的对手文件里配置; f优先级高于配置文件中的标志位,用来设置强制跑位;
-- 其中enforce只能为true(强制跑位)或者空缺; p, d, a与goCmuRush中的意义相同.
function goalie(p, enforce, d, a)
  if p == nil then
    return task.goalie()
  else
    if enforce == true then
      return task.goCmuRush(p, d, a)
    elseif enforce == nil then
      return ( gOppoConfig.dangerous and task.goalie() or task.goCmuRush(p, d, a) )
    end
  end
end

-- 开球车:开球车的拿球动作, f是给slowGetBall使用的,一般空缺,用法见task.lua; p1, p2 可以只传其中一个(表示p1 == p2);
-- 其中enforce只能为true(强制吸球)或者空缺; p1传给staticGetBall, p2传给slowGetBall.
function getBall(p1, p2, f, enforce)
  if enforce == true then
    if p2 ~= nil then
      return task.slowGetBall(p2, f)
    else
      return task.slowGetBall(p1, f)
    end
  else
    if p1 ~= nil and p2 == nil then
      return ( gOppoConfig.dribbleGet and task.slowGetBall(p1, f) or task.staticGetBall(p1) )
    elseif p1 == nil and p2 ~= nil then
      return ( gOppoConfig.dribbleGet and task.slowGetBall(p2, f) or task.staticGetBall(p2) )
    else
      return ( gOppoConfig.dribbleGet and task.slowGetBall(p2, f) or task.staticGetBall(p1) )
    end
  end
end

-- 开球车:开球车的传球动作, f是给chipPass使用的,一般空缺,用法见task.lua; p1, p2 可以只传一个(表示p1 == p2), c1不会默认等于c2;
-- 其中enforce只能为true(强制挑球)或者空缺; p1传给goAndTurnKick, p2传给chipPass.
function passBall(p1, c1, p2, c2, f, enforce)
  if enforce == true then
    if p2 ~= nil then
      return task.chipPass(p2, c2, f)
    else
      return task.chipPass(p1, c2, f)
    end
  else
    if p1 ~= nil and p2 == nil then
      return ( gOppoConfig.useChip and task.chipPass(p1, c2, f) or task.goAndTurnKick(p1, c1) )
    elseif p1 == nil and p2 ~= nil then
      return ( gOppoConfig.useChip and task.chipPass(p2, c2, f) or task.goAndTurnKick(p2, c1) )
    else
      return ( gOppoConfig.useChip and task.chipPass(p2, c2, f) or task.goAndTurnKick(p1, c1) )
    end
  end
end

-- 快捷防守函数, n为防守车数量, role为要躲避其射门线的角色, back为后卫数量, middle为中卫数量(可省略)
function defend(n, role, back, middle)
  if n == nil then
    return task.leftBack(), task.rightBack(), task.defendMiddle(role)
  elseif n == 1 then
    if back == 0 then
      return task.defendMiddle(role)
    else
      return task.singleBack()
    end
  elseif n == 2 then
    if back == 0 then
      return task.defendMiddle(role), task.defendMiddle(role)
    elseif back == 1 then
      return task.singleBack(), task.defendMiddle(role)
    elseif back == 2 then
      return task.leftBack(), task.rightBack()
    else
      return task.leftBack(), task.rightBack()
    end
  elseif n == 3 then
    if back == 1 then
      return task.singleBack(), task.defendMiddle(role), task.defendMiddle(role)
    elseif back == 2 then
      return task.leftBack(), task.rightBack(), task.defendMiddle(role)
    else
      return task.leftBack(), task.rightBack(), task.defendMiddle(role)
    end
  end
end
----------------------------------------------------------------------------------------------------

---------------------------------2014/04/29---------------------------------------------------------

function turnToPass(...)
  -- body
end
----------------------------------------------------------------------------------------------------
function universal(decision)
  local idecision = decision
  if idecision()() == "LightKick" then
    return task.flatPass(CGeoPoint:new_local(450, 0),50)
  elseif idecision()() == "Pass" then
    return task.flatPass(CGeoPoint:new_local(0, 0),200)
  else
    return task.advance()
  end
end
local targetdierction
local realdirection = 0
local ballspeed
local balldirection
local init = true
local maxSpeed=-999
local pTarget = CGeoPoint:new_local(450, 0)
local cont=false
local record_flag=true
local pKick=CGeoPoint:new_local(150,-250) --接球点
local pass_pos = pKick
local delta_y=0
local direction=1
local pPrepare=CGeoPoint:new_local(450,300)

gPlayTable.CreatePlay{

firstState = "prepare",
["prepare"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Kicker") < 20 and ball.posX()>0, 120, 6000) then
      cont=false
      record_flag=true
      delta_y=0
        maxSpeed=-999
        return "pass"
    end
  end,

  Kicker = task.goCmuRush(pKick),
  Tier   = task.staticGetBall("Kicker"),
  match = ""
},

["pass"] = {
  switch = function ()
    if bufcnt(player.kickBall("Tier"), 1, 360)then      
      return "kick"
    end
  end,    
  Kicker = task.goCmuRush(pKick), --dir.compensate(pTarget)),
  Tier   = task.goAndTurnKick("Kicker"), --传球力度
  match = ""
},  

["kick"] = {
  switch   = function()
      if ball.pos():dist(player.pos("Kicker")) < 95 and ball.pos():dist(player.pos("Kicker")) > 30 then
        mBallSpeed = ball.velMod()
        mBallDirection = ball.velDir()
        mTargetDirection = Utils.Normalize(Utils.Normalize(ball.velDir() + math.pi) - player.dir("Kicker"))*180/math.pi
      end
    
      if player.kickBall("Kicker") then
          return "record"
      end

      if bufcnt(not player.kickBall("Kicker"),180) then
        print("invalid!")
        return "record"
      end

      maxSpeed=0
    end,

  Kicker = task.InterTouch(),
  Tier   = task.stop(), 
  match = ""
},

["record"] = {

switch   = function()
  
  if ball.velMod()>maxSpeed then 
    maxSpeed=ball.velMod()
  end

  if player.toBallDist("Kicker")>130 and player.toBallDist("Kicker")<160 then
        compensation = Utils.Normalize(player.toBallDir("Kicker") - player.toTheirGoalDir("Kicker"))  -- 补偿角度 = 实际击出的角度 - 需要击出的角度
        compensation = compensation/math.pi*180
    end


    if ball.posX()>445 and record_flag==true then
      delta_y=ball.posY()
      recor_flag=false
    end


    --if bufcnt(ball.toPlayerDist("Kicker") > 150, 20, 120) then
    if bufcnt(ball.posX()>445, 5, 120) then

      mDirection = player.toTheirGoalDir("Kicker")
      mRealDirection = ball.velDir()

      print("--------- Compensation Result ---------")
      if(maxSpeed>800) then
        print("WARNING : Over Speed !!!")
      end
        print("Max Speed (cm/s): "..maxSpeed.."\nPass Speed(cm/s): "..mBallSpeed)
        print("Pass Angle(deg): "..mTargetDirection.."\nDelta Y(cm):"..delta_y)
       
        if(ball.posY()<-25) then
          print("Assesment: Lean Left.")
        end

        if(ball.posY()>25) then
          print("Assesment: Lean Right.")
        end

        if(ball.posY()<=25 and ball.posY()>=-25) then
          print("Assesment: Correct!")
        end

        local recordfile = io.open("CompensateRecordFile.txt","a")

        print("---------------------------------------------------------")
        print("BallSpeedOut  BallSpeedIn  DirectionIn  Compensation ")
        print( maxSpeed.."  "..mBallSpeed.."  "..mTargetDirection.."  "..compensation)
        print("---------------------------------------------------------")
    
        if (compensation*mTargetDirection<0 and math.abs(compensation)>3.5) then
          recordfile:write("-1\t",maxSpeed,"\t",mBallSpeed,"\t", mTargetDirection, "\t",compensation,"\n")
          recordfile:close()
          print("## Result : Compensation Ang is Too Small !")
        elseif (compensation*mTargetDirection>0 and math.abs(compensation)>3.5) then
          recordfile:write("1\t",maxSpeed,"\t",mBallSpeed,"\t", mTargetDirection, "\t",compensation,"\n")
          recordfile:close()
          print("## Result : Compensation Ang is Too Big !")
        else
          print("## Result : Correct !")
          recordfile:write("0\t",maxSpeed,"\t",mBallSpeed,"\t", mTargetDirection, "\t",compensation,"\n")
          recordfile:close()
        end
        
        --recordfile:write(maxSpeed,"\t",mBallSpeed,"\t", mTargetDirection, "\t",compensation,"\n")
        --recordfile:close()
      
      return "wait"
    end

    end,

  Kicker = task.stop(),
  Tier   = task.stop(), 
  match = ""
},

name = "TestOnePassShoot",

applicable ={
  exp = "a",
  a = true
},

attribute = "attack",
timeout = 99999
}


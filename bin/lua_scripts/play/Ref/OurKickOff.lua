if vision:Cycle() - gOurIndirectTable.lastRefCycle > 6 then
  if cond.validNum() <= 3 then
    gCurrentPlay = "Ref_KickOffV1"
  else
    if type(gOppoConfig.KickOff) == "function" then
  	  gCurrentPlay = cond.getOpponentScript("Ref_KickOffV", gOppoConfig.KickOff(), 1)
    else
      gCurrentPlay = cond.getOpponentScript("Ref_KickOffV", gOppoConfig.KickOff, 1)
    end
  end
end

gOurIndirectTable.lastRefCycle = vision:Cycle()
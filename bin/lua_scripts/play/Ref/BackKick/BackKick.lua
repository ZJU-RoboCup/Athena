if vision:Cycle() - gOurIndirectTable.lastRefCycle > 6 then
  if cond.validNum() <= 3 then
  	gCurrentPlay = "Ref_ImmortalKickV2"
  else
    if type(gOppoConfig.BackKick) == "function" then
  	  gCurrentPlay = cond.getOpponentScript("Ref_BackKickV", gOppoConfig.BackKick(), 3)
    else
      gCurrentPlay = cond.getOpponentScript("Ref_BackKickV", gOppoConfig.BackKick, 3)
    end
  end
end
if type(gOppoConfig.BackDef) == "function" then
	gCurrentPlay = cond.getOpponentScript("Ref_BackDefV",gOppoConfig.BackDef(),6)
else
	gCurrentPlay = cond.getOpponentScript("Ref_BackDefV",gOppoConfig.BackDef,6)
end
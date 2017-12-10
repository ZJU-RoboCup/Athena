-- 可指定具体定位球，也可以以table的形式在几个定位球中随机选择

gOppoConfig = {
  ----------------------play---------------------------------------------------------------
	CornerKick  = {7}, -- {5,11,7}, --  守门员出来{6}-- 小招{2,6,7,9} 乱招{10,12}
	FrontKick   = {6},--{1,2,3,5}, --如果前面的招都没效果{4}, --如果希望拖时间{6},
	MiddleKick  = {2}, --{4},打RoboDragon时有很好的效果 --如果没有领先且6被截{4}用来拖时间, 拖时间直接踢 {2},
	BackKick    = "Ref_BackKickV7",
	CornerDef   = "Ref_CornerDefV5",
	BackDef 	= "Ref_BackDefV1",
	MiddleDef   = "Ref_MiddleDefV2",
	FrontDef    = "Ref_FrontDefV2",
	PenaltyKick = "Ref_PenaltyKickV2",
	PenaltyDef  = "Ref_PenaltyDefV1",
	KickOff		  = "Ref_KickOffV4",
	KickOffDef  = "Ref_KickOffDefV1",
	NorPlay     = "Nor_PassAndShoot"
}
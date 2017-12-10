
SPlay_StateNum
8
SPlay_StateName
shoot	[Leader][Special](Defender,Assister,Middle)
1pass	[Leader](Special,Defender,Assister,Middle)
advance	[Leader][Special](Defender,Assister,Middle)
1kick	{Special,Leader,Defender,Assister,Middle}
bdefend	[Leader][Defender,Assister](Special)(Middle)
proball	[Leader,Defender,Assister](Special,Middle)
finish
exit 

SPlay_StateSwitch
shoot	6
	shoot		fast	1	a			0
		a		isBestPlayerChanged
	advance		normal	2	!a&b		-2
		a		canShootOnBallPos
		b		meAndEnemy2balldiff(Leader)<0
	1pass		normal	2	!a&b		-1
		a		canShootOnBallPos
		b		isPosReached(Special)<20
	bdefend		normal	2	a&b			1
		a		realTimeBallx<-50
		b		meAndEnemy2balldiff(Leader)<0
	proball		normal	1	a			1
		a		isBallInProArea
	finish		fast	1	a			0
		a		isBallKicked(Leader)
1pass	3
	1kick		fast	2	a|b			1
		a		isBallKicked(Leader)
		b		isBallPassed(Leader,Special)
	shoot		normal	2	a&b			1
		a		canShootOnBallPos
		b		currentBayes(attack)
	advance		normal	2	!a&!b		-2
		a		isBallControlled(Leader)
		b		currentBayes(attack)
advance	4
	advance		fast	1	a		0
		a		isBestPlayerChanged
	shoot		fast	2	a&b		1
		a		canShootOnBallPos
		b		meAndEnemy2balldiff(Leader)>10
	bdefend		normal	1	a		1
		a		realTimeBallx<-50
	1pass		normal	3	!a&b&c	-1
		a		canShootOnBallPos
		b		isPosReached(Special)<20
		c		bestenemy2balldist>20
1kick	2
	exit		normal	1	a		-2
		a		timeOut=120
	finish		fast	1	a		2
		a		isBallKicked(Special)
bdefend	3
	advance		normal	1	a		-2
		a		realTimeBallx>-20
	shoot		fast	2	a&b		1
		a		canShootOnBallPos
		b		meAndEnemy2balldiff(Leader)>10
	proball		normal	1	a		1
		a		isBallInProArea
proball	1
	bdefend		normal	1	!a		1
		a		isBallInProArea
finish		0
exit		0

SPlay_StateAction
	shoot		6
		Leader		shoot			Leader
		Special		goPassPos		Leader	Special
		Middle		goBackPos		Middle	Leader
		Defender	assist_goal_l
		Assister	assist_goal_r
		Goalie		goalie2
	1pass		6
		Special		goPassPos		Leader	Special
		Leader		pass	7
			pos				auto		Pos_Field_Ball			none
			angle			auto		Dir_Robot_Pass			Leader>Special
			ispass			auto		specified_pass			true
			ischipkick		auto		Kic_Auto_Robot1Robot2	Leader>Special
			chipkickpower	auto		Pow_Auto_Robot1Robot2	Leader>Special
			kickprecision	auto		Pre_Field_Specified		5
			kickpower		auto		Pow_Auto_Robot1Robot2	Leader>Special
		Middle		goBackPos		Middle	Special
		Defender	assist_goal_l
		Assister	assist_goal_r
		Goalie		goalie2
	advance		6
		Leader		advance
		Special		goPassPos		Leader	Special
		Middle		goBackPos		Middle	Leader
		Defender	assist_goal_l
		Assister	assist_goal_r
		Goalie		goalie2
	1kick		6
		Leader		goPassPos		Special		Leader
		Special		shoot			Special
		Middle		goBackPos		Middle		Special	
		Defender	assist_goal_l
		Assister	assist_goal_r
		Goalie		goalie2
	bdefend		6
		Leader		advance
		Special		marking			First
		Middle		goPassPos		Leader		Middle
		Defender	assist_goal_l
		Assister	assist_goal_r
		Goalie		goalie2
	proball		6
		Special		marking			First
		Middle		goPassPos		Leader			Middle
		Leader		goto1	2
			pos		auto			Pos_Protect_Up		none		
			angle	auto			Dir_Robot_Ball		Leader
		Defender	assist_goal_l
		Assister	assist_goal_r
		Goalie		goto1	2
			pos		auto			Pos_Protect_Down	none		
			angle	auto			Dir_Robot_Ball		Goalie
	finish		0
	exit		0
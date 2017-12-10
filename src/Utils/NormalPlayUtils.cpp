#include "NormalPlayUtils.h"
#include "gpuBestAlgThread.h"
#include "TaskMediator.h"
#include "Global.h"
//场地标号
// 对方球门
//  0 1
//  2 3
//  4 5
struct FieldSquare{
	FieldSquare(CGeoPoint ld,CGeoPoint ru){
		range=50;
		_leftDownPos=ld;
		_rightUpPos=ru;
		_leftUpPos=CGeoPoint(ru.x(),ld.y());
		_rightDownPos=CGeoPoint(ld.x(),ru.y());
		double tempX=(ld.x()+ru.x())/2,tempY=(ld.y()+ru.y())/2;
		if (tempY<0){
			tempY=tempY-10;
		}else{
			tempY=tempY+10;
		}
		_centerPos=CGeoPoint(tempX,tempY);
	}
	FieldSquare operator +(FieldSquare& param){
		if (param._leftDownPos.x()<this->_leftDownPos.x()
			&&param._leftDownPos.y()<this->_leftDownPos.y()){
			return FieldSquare(param._leftDownPos,this->_rightUpPos);
		}else{
			return FieldSquare(this->_leftDownPos,param._rightUpPos);
		}
	}
	FieldSquare centerArea(){
		return FieldSquare(CGeoPoint(_centerPos.x()-range,_centerPos.y()-range),CGeoPoint(_centerPos.x()+range,_centerPos.y()+range));
	}
	CGeoPoint getCenter(){
		return _centerPos;
	}
	double range;
	CGeoPoint _leftDownPos;
	CGeoPoint _rightUpPos;
	CGeoPoint _leftUpPos;
	CGeoPoint _rightDownPos;
	CGeoPoint _centerPos;
};

namespace NormalPlayUtils{
	double fieldWide=Param::Field::PITCH_WIDTH/2-5;
	double fieldLengh=Param::Field::PITCH_LENGTH/2-5;

	double middleFrontBorderX=100;
	double middleBackBorderX=-100;

	FieldSquare fieldSquareArray[6]={
		FieldSquare(CGeoPoint(middleFrontBorderX,-fieldWide),CGeoPoint(fieldLengh,0)),
		FieldSquare(CGeoPoint(middleFrontBorderX,0),CGeoPoint(fieldLengh,fieldWide)),
		FieldSquare(CGeoPoint(middleBackBorderX,-fieldWide),CGeoPoint(middleFrontBorderX,0)),
		FieldSquare(CGeoPoint(middleBackBorderX,0),CGeoPoint(middleFrontBorderX,fieldWide)),
		FieldSquare(CGeoPoint(-fieldLengh,-fieldWide),CGeoPoint(middleBackBorderX,0)),
		FieldSquare(CGeoPoint(-fieldLengh,0),CGeoPoint(middleBackBorderX,fieldWide)),
	};

	FieldSquare fieldSquareMap[6][2]={
		fieldSquareArray[1].centerArea(),fieldSquareArray[3].centerArea(),
		fieldSquareArray[0].centerArea(),fieldSquareArray[2].centerArea(),
		fieldSquareArray[1].centerArea(),fieldSquareArray[3].centerArea(),
		fieldSquareArray[0].centerArea(),fieldSquareArray[2].centerArea(),
		fieldSquareArray[1].centerArea(),fieldSquareArray[3].centerArea(),
		fieldSquareArray[0].centerArea(),fieldSquareArray[2].centerArea(),
	};
	

	bool noEnemyInPassLine(const CVisionModule* pVision,CGeoPoint ball,CGeoPoint passPos,const double range)
	{

		CGeoSegment seg = CGeoSegment(ball,passPos);
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{
			if (pVision->TheirPlayer(i).Valid())
			{
				CGeoPoint interPoint=seg.projection(pVision->TheirPlayer(i).Pos());
				if (interPoint.dist(pVision->TheirPlayer(i).Pos())<range && seg.IsPointOnLineOnSegment(interPoint))
				{
					return false;
				}
			}
		}
		return true;
	}

	bool canPassBetweenTwoPos(const CVisionModule* pVision,CGeoPoint passPos,const int meNum){
		const MobileVisionT ball=pVision->Ball();
		const PlayerVisionT me=pVision->OurPlayer(meNum);
		const double ballToHeDist=pVision->TheirPlayer(BestPlayer::Instance()->getTheirBestPlayer()).Pos().dist(ball.Pos());
		const double ballToMeDist=me.Pos().dist(ball.Pos());
		const double balltoMeDir=(me.Pos()-ball.Pos()).dir();
		const double passDir=(passPos-ball.Pos()).dir();
		const double metoBallDir=(ball.Pos()-me.Pos()).dir();
		const CGeoPoint predictBallPos=ball.Pos()+Utils::Polar2Vector(20,passDir);
		return !isEnemyCloseToReceiver(pVision,passPos,60)
			&&noEnemyInPassLine(pVision,predictBallPos,passPos,30)
			&&isReceiverCloseToPassPos(pVision,passPos,30)
			&&ballToHeDist>ballToMeDist+50
			//&&fabs(Utils::Normalize(me.Dir()-passDir))<Param::Math::PI/3
			//&&fabs(passDir)<Param::Math::PI*120/180
			//&&fabs((Utils::Normalize(me.Dir()-metoBallDir)))<Param::Math::PI/2
			&&(ball.Vel().mod()<50
				||ball.Vel().mod()>10
					&&(fabs(Utils::Normalize(ball.Vel().dir()-passDir))<Param::Math::PI/18&&ball.Vel().mod()<100
						||fabs(Utils::Normalize(ball.Vel().dir()-balltoMeDir))<Param::Math::PI/18&&ball.Vel().mod()<150)
			  )
			&&me.Pos().dist(passPos)>150
			&&fabs(passDir)<Param::Math::PI*135/180;
	}

	bool isEnemyBlockShootLine(CGeoPoint ball,double shootDir,double range){
		CGeoSegment seg = CGeoSegment(ball,ball+Utils::Polar2Vector(range,shootDir));
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{
			if (vision->TheirPlayer(i).Valid())
			{
				CGeoPoint interPoint=seg.projection(vision->TheirPlayer(i).Pos());
				if (interPoint.dist(vision->TheirPlayer(i).Pos())<20&&seg.IsPointOnLineOnSegment(interPoint))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool isPosCloseBorder(CGeoPoint pos,CGeoPoint smallVertex,CGeoPoint bigVertex){
		double outAreaBufferX=20;
		double outAreaBufferY=20;
		if((pos.x()-smallVertex.x())<-outAreaBufferX||(pos.x()-bigVertex.x())>outAreaBufferX||
			(pos.y()-smallVertex.y())<-outAreaBufferY||(pos.y()-bigVertex.y())>outAreaBufferY){
				return false;
		}else{
			return true;
		}
	}

	bool isPosInSquareArea(CGeoPoint pos,CGeoPoint smallVertex,CGeoPoint bigVertex){
		if (pos.x()>smallVertex.x()&&pos.x()<bigVertex.x()&&pos.y()>smallVertex.y()&&pos.y()<bigVertex.y()){
			return true;
		}
		return false;
	}

	bool generateTwoPassPoint(CGeoPoint ball,CGeoPoint& passPosOne,CGeoPoint& passPosTwo){
		static int _lastAreaNum=0;
		static bool _isBlockShootLine=false;
		int areaNum=6;
		bool isNewField=true;

		if (Utils::OutOfField(ball,1)){
			areaNum=_lastAreaNum;
			isNewField=false;
		}else{
			bool isCloseBorder=false;
			for (int i=0;i<6;i++)
			{
				if(isPosInSquareArea(ball,fieldSquareArray[i]._leftDownPos,fieldSquareArray[i]._rightUpPos)){
					areaNum=i;
					break;
				}
			}
			isCloseBorder=isPosCloseBorder(ball,fieldSquareArray[_lastAreaNum]._leftDownPos,fieldSquareArray[_lastAreaNum]._rightUpPos);
			
			if (isCloseBorder&&areaNum!=_lastAreaNum&&_lastAreaNum!=6||areaNum==6){
				areaNum=_lastAreaNum;
				isNewField=false;
			}else{
				_lastAreaNum=areaNum;
			}
		}
		GPUBestAlgThread::Instance()->setSendPoint(ball);
		passPosOne=GPUBestAlgThread::Instance()->getBestPoint(fieldSquareMap[areaNum][0]._leftUpPos,fieldSquareMap[areaNum][0]._rightDownPos,1,RECT_MODE,true);
		GPUBestAlgThread::Instance()->setSendPoint(ball);
		passPosTwo=GPUBestAlgThread::Instance()->getBestPoint(fieldSquareMap[areaNum][1]._leftUpPos,fieldSquareMap[areaNum][1]._rightDownPos,2,RECT_MODE,true);
		

		/*static CGeoPoint _lastPassPos1=passPosOne;
		static CGeoPoint _lastPassPos2=passPosTwo;

		CGeoPoint theirGoal=CGeoPoint(Param::Field::PITCH_LENGTH/2,0);	
		double shootDir=(ball-theirGoal).dir();
		CGeoSegment ballVelSeg=CGeoSegment(ball,ball+Utils::Polar2Vector(500,shootDir));

		CGeoLineLineIntersection inter=CGeoLineLineIntersection(ballVelSeg,lastNowSeg);
		bool interShootDir=false;

		if (!NormalPlayUtils::isEnemyBlockShootLine(ball,shootDir,30)){
			if (inter.Intersectant()){
				CGeoPoint interPos;
				interPos=inter.IntersectPoint();
				if (ballVelSeg.IsPointOnLineOnSegment(interPos)&&lastNowSeg.IsPointOnLineOnSegment(interPos)){
					interShootDir=true;
					_isBlockShootLine=true;
				}
			}
		}
		if (interShootDir){
			passPosOne=_lastPassPos1;
		}else{
			_lastPassPos1=passPosOne;
		}*/

		//GDebugEngine::Instance()->gui_debug_x(passPosOne,COLOR_RED);
		//GDebugEngine::Instance()->gui_debug_x(passPosTwo,COLOR_RED);
		
		return isNewField;
	}

	bool isEnemyCloseToReceiver(const CVisionModule* pVision,CGeoPoint receiver,double range){
		CGeoPoint theirGoal(Param::Field::PITCH_LENGTH/2,0);
		double shootDir=(theirGoal-receiver).dir();
		CGeoSegment shootSeg(receiver,receiver+Utils::Polar2Vector(180,shootDir));
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{
			if (pVision->TheirPlayer(i).Valid())
			{
				CGeoPoint proj=shootSeg.projection(pVision->TheirPlayer(i).Pos());
				if (pVision->TheirPlayer(i).Pos().dist(receiver)<range 
					|| shootSeg.IsPointOnLineOnSegment(proj)&&proj.dist(pVision->TheirPlayer(i).Pos())<35)
				{
					return true;
				}
			}
		}
		return false;
	}

	bool isReceiverCloseToPassPos(const CVisionModule* pVision,CGeoPoint passPos,double range){
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{
			if (pVision->OurPlayer(i).Valid())
			{
				//cout<<i<<" "<<pVision->OurPlayer(i).Vel().mod()<<endl;
				if (pVision->OurPlayer(i).Pos().dist(passPos)<range&&i!=TaskMediator::Instance()->leftBack()
					&&i!=TaskMediator::Instance()->rightBack()&&i!=PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())
					&&pVision->OurPlayer(i).Vel().mod()<80)
				{
					return true;
				}
			}
		}
		return false;
	}

	int getOurMostClosetoPos(const CVisionModule* pVision,CGeoPoint pos){
		double dist=1000;
		int num=0;
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{
			if (pVision->OurPlayer(i).Valid())
			{
				if (pVision->OurPlayer(i).Pos().dist(pos)<dist&&i!=PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie()))
				{
					dist =pVision->OurPlayer(i).Pos().dist(pos);
					num=i;
				}
			}
		}
		return num;
	}

	int getTheirMostClosetoPos(const CVisionModule* pVision,CGeoPoint pos){
		double dist=1000;
		int num=0;
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{
			if (pVision->TheirPlayer(i).Valid())
			{
				if (pVision->TheirPlayer(i).Pos().dist(pos)<dist)
				{
					dist =pVision->TheirPlayer(i).Pos().dist(pos);
					num=i;
				}
			}
		}
		return num;
	}

	bool isEnemyFrontToBall(const CVisionModule* pVision,double range){
		const MobileVisionT ball=pVision->Ball();
		CGeoSegment ballVelSeg=CGeoSegment(ball.Pos()+Utils::Polar2Vector(-range,ball.Vel().dir()),ball.Pos()+Utils::Polar2Vector(500,ball.Vel().dir()));
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{
			if (pVision->TheirPlayer(i).Valid())
			{
				CGeoPoint seg=ballVelSeg.projection(pVision->TheirPlayer(i).Pos());
				if (ballVelSeg.IsPointOnLineOnSegment(seg)){
					return true;
				}
			}
		}
		return false;
	}

	bool ballMoveToOurDefendArea(const CVisionModule* pVision){
		const MobileVisionT ball=pVision->Ball();
		CGeoLine ballMovingLine = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(1000,ball.Vel().dir()));
		CGeoLine defendLine=CGeoSegment(CGeoPoint(-Param::Field::PITCH_LENGTH/2,-(Param::Field::GOAL_WIDTH+20)),CGeoPoint(-Param::Field::PITCH_LENGTH/2,Param::Field::GOAL_WIDTH+20));
		CGeoSegment defendSeg=CGeoSegment(CGeoPoint(-Param::Field::PITCH_LENGTH/2,-(Param::Field::GOAL_WIDTH+20)),CGeoPoint(-Param::Field::PITCH_LENGTH/2,Param::Field::GOAL_WIDTH+20));	
		CGeoLineLineIntersection inter=CGeoLineLineIntersection(defendLine,ballMovingLine);
		if (ball.Vel().mod()>100)
		{
			if (inter.Intersectant()){
				CGeoPoint interPoint=inter.IntersectPoint();
				if (defendSeg.IsPointOnLineOnSegment(interPoint))
				{
					return true;
				}
			}
		}
		return false;
	}
	double generateKickAtEnemyDir(const CVisionModule* pVision,const PlayerVisionT& me){
		const MobileVisionT ball=pVision->Ball();
		double shootDir=KickDirection::Instance()->getRealKickDir();
		CGeoSegment shootSeg=CGeoSegment(ball.Pos(),ball.Pos()+Utils::Polar2Vector(200,shootDir));
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{

			if (pVision->TheirPlayer(i).Valid())
			{
				CGeoPoint proj=shootSeg.projection(pVision->TheirPlayer(i).Pos());
				if (shootSeg.IsPointOnLineOnSegment(proj)&&pVision->TheirPlayer(i).Pos().dist(proj)<10){
					
				}
			}
		}
		return 0;
	}

	int  patchForBestPlayer(const CVisionModule* pVision,int lastAdvancerNum){
		const MobileVisionT ball=pVision->Ball();
		double distRange=60;
		int advancerNum[Param::Field::MAX_PLAYER]={0,0,0,0,0,0};
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{
			if (pVision->OurPlayer(i).Valid())
			{
				if (pVision->OurPlayer(i).Pos().dist(ball.Pos())<distRange&&i!=PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie()))
				{
					advancerNum[i-1]=1;
				}
			}
		}
		

		double tempAngle=10;
		const CGeoPoint theirGoal=CGeoPoint(Param::Field::PITCH_LENGTH/2,0);
		int result=0;
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{
			if (advancerNum[i-1]!=0){
				double metoGoalDir=(theirGoal-pVision->OurPlayer(i).Pos()).dir();
				double metoBallDir=(ball.Pos()-pVision->OurPlayer(i).Pos()).dir();
				double diff=fabs(Utils::Normalize(metoBallDir-metoGoalDir));
				if (diff<tempAngle){
					tempAngle=fabs(Utils::Normalize(metoBallDir-metoGoalDir));
					result=i;
				}				
			}
		}
		return result;
	}

	double antiNormalize(double dir){
		return dir<0?2*Param::Math::PI+dir:dir;
	}

	double generateTandemCond(const CVisionModule* pVision,CGeoPoint& tandemPos,int meNum){
		const PlayerVisionT me=pVision->OurPlayer(meNum);
		const PlayerVisionT he=pVision->TheirPlayer(BestPlayer::Instance()->getTheirBestPlayer());
		const MobileVisionT ball=pVision->Ball();
		const CGeoPoint ballPos=ball.Pos();
		const CGeoPoint ourGoalPos=CGeoPoint(-Param::Field::PITCH_LENGTH/2,0);
		const double balltoOurGoalDir=(ourGoalPos-ballPos).dir();
		const CGeoPoint theirGoalPos=CGeoPoint(Param::Field::PITCH_LENGTH/2,0);
		const double balltoTheirGoalDir=(theirGoalPos-ballPos).dir();
		const double antiBalltoTheirGoalDir=(ballPos-theirGoalPos).dir();
		tandemPos=ballPos+Utils::Polar2Vector(50,balltoOurGoalDir);
		double tandemDir=balltoTheirGoalDir;
		double toBallDist=50;
		const int buffer=30;
		double borderX=-130;


		static int lastSide=ballPos.y()>0?-1:1;
		static int lastArea=ballPos.x()<borderX?1:-1;

		int curSide=lastSide;
		int curArea=lastArea;

		if (lastSide==-1){
			if (ballPos.y()<-buffer){
				curSide=1;
				lastSide=curSide;
			}
		}else{
			if (ballPos.y()>buffer){
				curSide=-1;
				lastSide=curSide;
			}
		}

		if (lastArea==-1){
			if (ballPos.x()<borderX){
				curArea=1;
				lastArea=curArea;
			}
		}else{
			if (ballPos.x()>borderX+40){
				curArea=-1;
				lastArea=curArea;
			}
		}
		CGeoPoint goalBorderPoint(-Param::Field::PITCH_LENGTH/2,-curSide*Param::Field::PENALTY_AREA_WIDTH/2);
		double ballToGoalBorderDir=(goalBorderPoint-ball.Pos()).dir();
		double posDir=balltoOurGoalDir;
		double hetoOurGoalDir=(ourGoalPos-he.Pos()).dir();
		CGeoSegment heShootSeg=CGeoSegment(he.Pos()+Utils::Polar2Vector(10,he.Dir()),he.Pos()+Utils::Polar2Vector(500,he.Dir()));
		if (fabs(he.Dir())<Param::Math::PI*100/180){
			heShootSeg=CGeoSegment(he.Pos()+Utils::Polar2Vector(10,hetoOurGoalDir),he.Pos()+Utils::Polar2Vector(500,hetoOurGoalDir));
		}
		CGeoPoint metoHeProj=heShootSeg.projection(me.RawPos());
		static int lastState=heShootSeg.IsPointOnLineOnSegment(metoHeProj)&&me.RawPos().dist(metoHeProj)<11?1:-1;
		int curState=lastState;
		if (lastState==-1){
			if (heShootSeg.IsPointOnLineOnSegment(metoHeProj)&&me.RawPos().dist(metoHeProj)<8){
				curState=1;
				lastState=curState;
			}
		}else{
			if (heShootSeg.IsPointOnLineOnSegment(metoHeProj)&&me.RawPos().dist(metoHeProj)>14){
				curState=-1;
				lastState=curState;
			}
		}

		if (curState==1){
			posDir=Utils::Normalize(curSide*Param::Math::PI/2+Utils::Normalize((antiBalltoTheirGoalDir-curSide*Param::Math::PI/2))*1/4);
			tandemDir=0;
		}else{
			posDir=antiNormalize(he.Dir());
			if (curSide==1){
				posDir=max(Param::Math::PI/2,posDir);
				posDir=min(antiNormalize(ballToGoalBorderDir),posDir);
				posDir=Utils::Normalize(posDir);
			}else{
				posDir=min(Param::Math::PI*3/2,posDir);
				posDir=max(antiNormalize(ballToGoalBorderDir),posDir);
				posDir=Utils::Normalize(posDir);
			}

			//防球门连线
			posDir=balltoOurGoalDir;

			tandemDir=Utils::Normalize(posDir+Param::Math::PI);

			/*if (curSide==1){
				if (he.Dir()<Param::Math::PI/2&&he.Dir()>0)
				{
					tandemDir=-curSide*Param::Math::PI/2;
					posDir=curSide*Param::Math::PI/2;
					toBallDist=100;
				}else if (he.Dir()>ballToGoalBorderDir){
					posDir=ballToGoalBorderDir;
					tandemDir=Utils::Normalize(ballToGoalBorderDir+Param::Math::PI);
					toBallDist=50;
				}
			}else{
				if (he.Dir()>-Param::Math::PI/2&&he.Dir()<0)
				{
					tandemDir=-curSide*Param::Math::PI/2;
					posDir=curSide*Param::Math::PI/2;
					toBallDist=100;
				}else if (Utils::Normalize(ballToGoalBorderDir-he.Dir())>0){
					posDir=ballToGoalBorderDir;
					tandemDir=Utils::Normalize(ballToGoalBorderDir+Param::Math::PI);
					toBallDist=50;
				}
			}*/
		}

		//CGeoSegment ballShootSeg=CGeoSegment(ballPos,ballPos+Utils::Polar2Vector(500,balltoOurGoalDir));
		//CGeoPoint metoballShootSegProj=ballShootSeg.projection(me.Pos());
		//if (ballShootSeg.IsPointOnLineOnSegment(metoballShootSegProj)||me.Pos().dist(metoballShootSegProj)>8){
		//	posDir=balltoOurGoalDir;
		//	tandemDir=Utils::Normalize(posDir+Param::Math::PI);
		//}
		tandemPos=ballPos+Utils::Polar2Vector(toBallDist,posDir);
		if (curArea==1){
			posDir=curSide*Param::Math::PI/2;
			tandemPos=ballPos+Utils::Polar2Vector(100,posDir);
			tandemPos=Utils::MakeOutOfOurPenaltyArea(tandemPos,40);
			tandemDir=(he.Pos()-tandemPos).dir();
		}
		//GDebugEngine::Instance()->gui_debug_x(tandemPos,COLOR_WHITE);
		return tandemDir;
	}


	bool faceTheirGoal(const CVisionModule* pVision, const int executor,const double pre) {
		double dist = Param::Field::PITCH_LENGTH / 2 - pVision->OurPlayer(executor).Pos().x();
		double buffer = 0;
		double bufferDir=pre;
		const CGeoPoint theirLeft = CGeoPoint(Param::Field::PITCH_LENGTH / 2, -Param::Field::GOAL_WIDTH / 2 - buffer);
		const CGeoPoint theirRight = CGeoPoint(Param::Field::PITCH_LENGTH / 2, Param::Field::GOAL_WIDTH / 2 + buffer);
		double leftAngle = Utils::Normalize((theirLeft - pVision->OurPlayer(executor).Pos()).dir());
		double rightAngle = Utils::Normalize((theirRight - pVision->OurPlayer(executor).Pos()).dir());
		double myDir = pVision->OurPlayer(executor).Dir();
		double myX=pVision->OurPlayer(executor).Pos().x();
		if (myX<270) // 已修改为Brazil zhyaic
		{
			bufferDir=Param::Math::PI*1/180;
		}
		//if (myX>300&&fabs(myDir)<Param::Math::PI*120/180) // 已修改为Brazil zhyaic
		//{
		//	return true;
		//}

		double rawLeftAngle = Utils::Normalize((theirLeft - pVision->OurPlayer(executor).RawPos()).dir());
		double rawRifhtAngle = Utils::Normalize((theirRight - pVision->OurPlayer(executor).RawPos()).dir());
		double myRawDir = pVision->OurRawPlayer(executor).dir;

		bool isToTheirGoal = ((myDir > Utils::Normalize(leftAngle-bufferDir)) && (myDir < Utils::Normalize(rightAngle+bufferDir)));
		bool isRawToTheirGoal = ((myRawDir > Utils::Normalize(rawLeftAngle-bufferDir)) && (myRawDir < Utils::Normalize(rawRifhtAngle+bufferDir)));

		if (true) {
			GDebugEngine::Instance()->gui_debug_line(theirLeft, pVision->OurPlayer(executor).Pos());
			GDebugEngine::Instance()->gui_debug_line(theirRight, pVision->OurPlayer(executor).Pos());
			GDebugEngine::Instance()->gui_debug_line(pVision->OurPlayer(executor).Pos() + Utils::Polar2Vector(500, myDir), pVision->OurPlayer(executor).Pos());

			//GDebugEngine::Instance()->gui_debug_line(theirLeft, pVision->OurPlayer(executor).RawPos());
			//GDebugEngine::Instance()->gui_debug_line(theirRight, pVision->OurPlayer(executor).RawPos());
			//GDebugEngine::Instance()->gui_debug_line(pVision->OurPlayer(executor).RawPos() + Utils::Polar2Vector(100, myRawDir), pVision->OurPlayer(executor).RawPos());
		}
		return (isToTheirGoal || isRawToTheirGoal);
	}

	int  getSuitSider(const CVisionModule* pVision){
		const CGeoPoint sideBackPos=DefPos2015::Instance()->getDefPos2015(pVision).getSidePos();
		for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
		{
			if (pVision->OurPlayer(i).Valid())
			{
				if (pVision->OurPlayer(i).Pos().y()/sideBackPos.y()>0 && i!=PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())
					&& i!=TaskMediator::Instance()->leftBack() && i!=TaskMediator::Instance()->rightBack()
					&& i!=TaskMediator::Instance()->defendMiddle())
				{
					return i;
				}
			}
		}
		return 0;
	}

}

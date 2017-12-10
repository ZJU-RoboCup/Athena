#include "MarkingTouch.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <ControlModel.h>
#include <KickStatus.h>
#include <RobotSensor.h>
#include "utils.h"
#include "ShootRangeList.h"
#include <CommandFactory.h>
#include "TouchKickPos.h"
#include "BallSpeedModel.h"
#include <GDebugEngine.h>
#include "PlayInterface.h"
#include "NormalPlayUtils.h"
#include "WorldModel.h"
#include "BestPlayer.h"
#include "defence/DefenceInfo.h"

namespace
{
	//2.轨迹生成算法使用变量
	enum{
		Marking = 1,
		GotoFirstPos,
		Touch,
		MarkingAgain,
		AdvanceBall,
	};

	bool verBos=false;
}

CMarkingTouch::CMarkingTouch()
{
	_lastCycle = 0;
	_stateCounter=0;
	_markEnemyNum=0;
	_marktoTouchCouter=0;
	_touchToBeginCouter=0;
	_markAgainToBeginCouter=0;
	_kickEnemyNum=0;
	_ballVelChangeCouter=0;
	_ballVelDirChanged=false;
}

void CMarkingTouch::caculMarkingPos(const CGeoSegment& passSegment,const bool markDirection){
	double XRangeValue=25;
	double YRangeValue=80;
	if (markDirection){
		CGeoLine markline=CGeoLine(_markPos,Param::Math::PI/2);
		CGeoLineLineIntersection inter=CGeoLineLineIntersection(markline,passSegment);
		if (inter.Intersectant()&&passSegment.IsPointOnLineOnSegment(inter.IntersectPoint())){
			double interY=inter.IntersectPoint().y();
			double markPosY=_markPos.y();
			if (interY>_markPos.y()+YRangeValue)
			{
				markPosY=_markPos.y()+YRangeValue;
			}else if (interY<_markPos.y()-YRangeValue)
			{
				markPosY=_markPos.y()-YRangeValue;
			}else{
				markPosY=interY;
			}
			if (fabs(markPosY)>Param::Field::PITCH_WIDTH/2-5)
			{
				markPosY=Utils::Sign(markPosY)*(Param::Field::PITCH_WIDTH/2-5);
			}
			_markPos.setY(markPosY);
		}
		//_pointA=CGeoPoint(_markPos.x(),leftUpPos.y()),_pointB=CGeoPoint(_markPos.x(),rightDownPos.y());
	}else{
		CGeoLine markline=CGeoLine(_markPos,0);
		CGeoLineLineIntersection inter=CGeoLineLineIntersection(markline,passSegment);
		if (inter.Intersectant()&&passSegment.IsPointOnLineOnSegment(inter.IntersectPoint())){
			double interX=inter.IntersectPoint().x();
			double markPosX=_markPos.x();
			if (interX>_markPos.x()+XRangeValue)
			{
				markPosX=_markPos.x()+XRangeValue;
			}else if (interX<_markPos.x()-XRangeValue)
			{
				markPosX=_markPos.x()-XRangeValue;
			}else{
				markPosX=interX;
			}
			if (fabs(markPosX)>Param::Field::PITCH_LENGTH/2-5)
			{
				markPosX=Utils::Sign(markPosX)*(Param::Field::PITCH_LENGTH/2-5);
			}
			_markPos.setX(markPosX);
		}
		//_pointA=CGeoPoint(leftUpPos.x(),_markPos.y()),_pointB=CGeoPoint(rightDownPos.x(),_markPos.y());
	}
}

void CMarkingTouch::resetState(){
	setState(BEGINNING);
	_kickEnemyNum=0;
	_markEnemyNum=0;
	_stateCounter=0;
	_marktoTouchCouter=0;
	_touchToBeginCouter=0;
	_markAgainToBeginCouter=0;
	_ballVelChangeCouter=0;
	_ballVelDirChanged=false;
}

void CMarkingTouch::plan(const CVisionModule* pVision){

	const int robotNum = task().executor;
	const double finalKickDir=task().player.angle;
	const int playerFlag = task().player.flag;
	const CGeoPoint leftUpPos=task().ball.pos;
	const CGeoPoint rightDownPos=task().player.pos;
	//true = horizal
	bool markDirection=false;
	if (playerFlag & PlayerStatus::ACCURATELY){
		markDirection=true;
	}

	const MobileVisionT& ball = pVision->Ball();
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const double dist2Ball=me.Pos().dist(ball.Pos());
	//重要！！！！！注册车号
	DefenceInfo::Instance()->setMarkingTouchInfo(pVision,robotNum,leftUpPos,rightDownPos);
	/*if ((_lastLeftUpPos.x()!=leftUpPos.x()||_lastLeftUpPos.y()!=leftUpPos.y()||
	_lastRightDownPos.x()!=rightDownPos.x()||_lastRightDownPos.y()!=rightDownPos.y())&&
	state()!=Touch)
	{
	resetState();
	if (markDirection)
	{
	double markX=fabs(leftUpPos.x()-ball.X())<fabs(rightDownPos.x()-ball.X())?leftUpPos.x():rightDownPos.x();
	_markPos=CGeoPoint(markX,(leftUpPos.y()+rightDownPos.y())/2.0);
	}else{
	double markY=fabs(leftUpPos.y()-ball.Y())<fabs(rightDownPos.y()-ball.Y())?leftUpPos.y():rightDownPos.y();
	_markPos=CGeoPoint((leftUpPos.x()+rightDownPos.x())/2.0,markY);
	}
	}*/
	_lastLeftUpPos=leftUpPos;
	_lastRightDownPos=rightDownPos;

	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 ){
		resetState();
		_lastCycle=0;
		if (markDirection)
		{
			double markX=fabs(leftUpPos.x()-ball.X())<fabs(rightDownPos.x()-ball.X())?leftUpPos.x():rightDownPos.x();
			_markPos=CGeoPoint(markX,(leftUpPos.y()+rightDownPos.y())/2.0);
		}else{
			double markY=fabs(leftUpPos.y()-ball.Y())<fabs(rightDownPos.y()-ball.Y())?leftUpPos.y():rightDownPos.y();
			_markPos=CGeoPoint((leftUpPos.x()+rightDownPos.x())/2.0,markY);
		}
	}

	
	//cout<<"enemy"<<_kickEnemyNum<<endl;
	double ballVelDir = ball.Vel().dir();									// 球速方向
	double ballVelReverse = Utils::Normalize(ballVelDir+Param::Math::PI);	// 球速反向
	double kickDirReverse = Utils::Normalize(finalKickDir+Param::Math::PI);		// 目标方向反向
	double angleDiff_ballVelDir2kickDirReverse = fabs(Utils::Normalize(finalKickDir - ballVelReverse));

	CGeoRectangle markField = CGeoRectangle(leftUpPos,rightDownPos);
	int markEnemyNum=0;
	for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
	{
		if (pVision->TheirPlayer(i).Valid()){
			if (markField.HasPoint(pVision->TheirPlayer(i).Pos()))
			{
				if (markEnemyNum!=0)
				{
					if (pVision->TheirPlayer(i).Pos().dist(ball.Pos())<pVision->TheirPlayer(markEnemyNum).Pos().dist(ball.Pos())-20)
					{
						markEnemyNum=i;
					}
				}else{
					markEnemyNum=i;
				}
			}
		}
	}
	//if (_markEnemyNum==0 || !pVision->TheirPlayer(_markEnemyNum).Valid()
	//	||!markField.HasPoint(pVision->TheirPlayer(_markEnemyNum).Pos())){
	//	_markEnemyNum=markEnemyNum;
	//}
	if (state()!=Touch){
		_markEnemyNum=markEnemyNum;
		int temptNum=NormalPlayUtils::getTheirMostClosetoPos(pVision,ball.Pos());
		PlayerVisionT temptKickEnemy=pVision->TheirPlayer(temptNum);
		double hetoBallDir=(ball.Pos()-temptKickEnemy.Pos()).dir();
		if (fabs(Utils::Normalize(hetoBallDir-temptKickEnemy.Dir()))<Param::Math::PI*20/180)
		{
			_kickEnemyNum=temptNum;
		}
	}
	const PlayerVisionT kickEnemy=pVision->TheirPlayer(_kickEnemyNum);

	//待测试 将ball改成kickenemy
	CGeoLine passLine=CGeoLine(kickEnemy.Pos(),kickEnemy.Dir());
	CGeoSegment passSegment=CGeoSegment(ball.Pos(),ball.Pos()+Utils::Polar2Vector(800,kickEnemy.Dir()));
	GDebugEngine::Instance()->gui_debug_line(ball.Pos(),ball.Pos()+Utils::Polar2Vector(600,kickEnemy.Dir()),COLOR_BLACK);
	//cout<<_markEnemyNum<<endl;
	const PlayerVisionT he=pVision->TheirPlayer(_markEnemyNum);
	const double metoBallDir=(ball.Pos()-me.Pos()).dir();
	const double antiMetoBallDir=Utils::Normalize(metoBallDir+Param::Math::PI);
	const double hetoBallDir=(ball.Pos()-he.Pos()).dir();
	const double antiHetoBallDir=Utils::Normalize(hetoBallDir+Param::Math::PI);
	const CGeoSegment metoBallSeg = CGeoSegment(me.Pos()+Utils::Polar2Vector(8,metoBallDir),ball.Pos());
	const CGeoSegment hetoBallSeg = CGeoSegment(he.Pos()+Utils::Polar2Vector(8,hetoBallDir),ball.Pos());
	const double meBall_ballVelDiff=fabs(Utils::Normalize(metoBallDir - ballVelDir))* 180 / Param::Math::PI;
	double kickEnemyToBallDir=(ball.Pos()-kickEnemy.Pos()).dir();
	double diffAngleBallVel_HeDir=fabs(Utils::Normalize(ball.Vel().dir()-kickEnemy.Dir()));
	double diffAngleBallVel_HetoBall=fabs(Utils::Normalize(kickEnemyToBallDir-ball.Vel().dir()));
	double diffAngleHeDir_HetoBall=fabs(Utils::Normalize(kickEnemyToBallDir-kickEnemy.Dir()));
	//state jump
	//CGeoPoint meProjPassLine=passSegment.projection(me.Pos());
	//CGeoPoint heProjPassLine=passSegment.projection(he.Pos());
	CGeoLine ballLine = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(100,ballVelDir));
	CGeoSegment ballSeg = CGeoSegment(ball.Pos(),ball.Pos()+Utils::Polar2Vector(800,ballVelDir));
	CGeoPoint meInBallVel=ballLine.projection(me.Pos());
	const double distMeProjBallVeltoMe=meInBallVel.dist(me.Pos());
	bool isBallCrossMe=!ballSeg.IsPointOnLineOnSegment(meInBallVel);


	CGeoPoint meProjhe=hetoBallSeg.projection(me.Pos());
	bool isMarked=hetoBallSeg.IsPointOnLineOnSegment(meProjhe)
		&&me.Pos().dist(he.Pos())<65||me.Pos().dist(_markPos)<18||_markEnemyNum==0;
	CGeoPoint heProjme=metoBallSeg.projection(he.Pos());
	bool isEnemyFrontMe=metoBallSeg.IsPointOnLineOnSegment(heProjme)
		&&Utils::pointToLineDist(he.Pos(),metoBallSeg)<40;

	bool isBallVelOk=fabs(Utils::Normalize(pVision->Ball(_lastCycle-1).Vel().dir()-ball.Vel().dir()))<Param::Math::PI*8/180;
	isBallVelOk=true;
	bool isBallPassed=//isBallVelOk&&
		//&&diffAngleBallVel_HeDir<Param::Math::PI*20/180&&diffAngleBallVel_HetoBall<Param::Math::PI*20/180
		diffAngleHeDir_HetoBall<Param::Math::PI*75/180&&
		kickEnemy.Pos().dist(ball.RawPos())>15
		&&ball.Vel().mod()>50;
		//&&(fabs(Utils::Normalize(antiMetoBallDir-ballVelDir))<Param::Math::PI*45/180
		//||fabs(Utils::Normalize(antiHetoBallDir-ballVelDir))<Param::Math::PI*45/180);

	bool failTouch=(ball.Vel().mod()<100 && !RobotSensor::Instance()->IsInfraredOn(robotNum)
		||meBall_ballVelDiff<Param::Math::PI/2 && ball.Vel().mod()<300);
		
		

	switch (state())
	{
	case BEGINNING:
		setState(Marking);
	case Marking:
		if (isBallPassed){
			_marktoTouchCouter++;
			//计算车开球瞬间车位置-车朝向连线与站位线的交点
			if (_marktoTouchCouter==1){
				CGeoLine markLine;
				double rangeValue=35;
				if (markDirection){
					markLine=CGeoLine(_markPos,Param::Math::PI/2);
				}else{
					markLine=CGeoLine(_markPos,0);
				}
				CGeoLineLineIntersection inter=CGeoLineLineIntersection(markLine,passLine);
				if (inter.Intersectant())
				{
					_markPos=inter.IntersectPoint();
				}
				if (markDirection)
				{
					_pointA=CGeoPoint(_markPos.x(),_markPos.y()+rangeValue),_pointB=CGeoPoint(_markPos.x(),_markPos.y()-rangeValue);
				}else{
					_pointA=CGeoPoint(_markPos.x()+rangeValue,_markPos.y()),_pointB=CGeoPoint(_markPos.x()-rangeValue,_markPos.y());
				}
				//if (markDirection&&me.Pos().dist(_markPos+Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse))>10)
				//{
				//	setState(GotoFirstPos);
				//}else{
				//	setState(Touch);
				//}
				setState(Touch);
				_marktoTouchCouter=0;
			}
		}
		break;
	case GotoFirstPos:
		if(verBos) cout<<"dist"<<me.Pos().dist(_markPos+Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse))<<endl;
		if (me.Pos().dist(_markPos+Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse))<5)
		{
			setState(Touch);
		}
		break;
	case Touch:
		if ((isEnemyFrontMe||failTouch) && VisionModule::Instance()->ballVelValid()){
			_touchToBeginCouter++;
			if (_touchToBeginCouter==8){
				setState(AdvanceBall);
				_touchToBeginCouter=0;
			}
		}
		if (WorldModel::Instance()->IsBallKicked(robotNum))
		{
			setState(BEGINNING);
		}
		break;
	case MarkingAgain:
		_markAgainToBeginCouter++;
		if (_markAgainToBeginCouter==300)
		{
			setState(BEGINNING);
			_markAgainToBeginCouter=0;
		}
		break;
	case  AdvanceBall:
		break;
	}
	if(verBos) cout<<robotNum<<" stateJump: "<<state()<<" "<<isMarked<<" "<<isBallPassed<<endl;

	if (state()==Marking||state()==MarkingAgain)
	{
		if (_markEnemyNum==0){
			if (markDirection)
			{
				double markX=fabs(leftUpPos.x()-ball.X())<fabs(rightDownPos.x()-ball.X())?leftUpPos.x():rightDownPos.x();
				_markPos=CGeoPoint(markX,(leftUpPos.y()+rightDownPos.y())/2.0);
			}else{
				double markY=fabs(leftUpPos.y()-ball.Y())<fabs(rightDownPos.y()-ball.Y())?leftUpPos.y():rightDownPos.y();
				_markPos=CGeoPoint((leftUpPos.x()+rightDownPos.x())/2.0,markY);
			}
		}else{
			if (markDirection){
				_markPos=he.Pos()+Utils::Polar2Vector(35,hetoBallDir);
			}else{
				_markPos=he.Pos()+Utils::Polar2Vector(40,hetoBallDir);
			}
		}
		if (state()==Marking)
		{
			caculMarkingPos(passSegment,markDirection);
			if (markDirection)
			{
				if (_markPos.y()>rightDownPos.y())
				{
					_markPos.setY(rightDownPos.y());
				}
				if (_markPos.y()<leftUpPos.y())
				{
					_markPos.setY(leftUpPos.y());
				}
			}else{
				if (_markPos.x()>leftUpPos.x())
				{
					_markPos.setX(leftUpPos.x());
				}
				if (_markPos.x()<rightDownPos.x())
				{
					_markPos.setX(rightDownPos.x());
				}
			}
		}		
	}

	CGeoPoint markTouchPos=_markPos;
	if (state()==Touch)
	{
		CGeoLine pointLine = CGeoLine(_pointA,_pointB);
		CGeoSegment pointSegment = CGeoSegment(_pointA,_pointB);
		CGeoLineLineIntersection intersect = CGeoLineLineIntersection(pointLine,ballLine);

		double ball2playerdir = (pVision->OurPlayer(robotNum).Pos() - ball.Pos()).dir();
		double diff_ball2playerdir_ballveldir = fabs(Utils::Normalize(ballVelDir - ball2playerdir));

		double ball2playerdist = (pVision->OurPlayer(robotNum).Pos() - ball.Pos()).mod();
		if (markDirection)
		{
			ball2playerdist=fabs(pVision->OurPlayer(robotNum).Pos().x() - ball.Pos().x());
		}else{
			ball2playerdist=fabs(pVision->OurPlayer(robotNum).Pos().y() - ball.Pos().y());
		}
		
		//判断球速是否直线
		if (fabs(Utils::Normalize(pVision->Ball().Vel().dir()-pVision->Ball(_lastCycle-2).Vel().dir()))>Param::Math::PI*3.5/180)
		{
			_ballVelChangeCouter++;
			_ballVelChangeCouter=min(_ballVelChangeCouter,3);
		}else{
			_ballVelChangeCouter--;
			_ballVelChangeCouter=max(_ballVelChangeCouter,0);
		}
		if (_ballVelChangeCouter==2){
			_ballVelDirChanged=true;		
		}
		if (_ballVelChangeCouter==0)
		{
			_ballVelDirChanged=false;
		}
		if (verBos) cout<<"ballVel"<<_ballVelDirChanged<<" "<<_ballVelChangeCouter<<endl;
			
		if(intersect.Intersectant()){
			CGeoPoint interP = intersect.IntersectPoint();
			if (!_ballVelDirChanged&&ball2playerdist<300||ball2playerdist<50)
			{	
				if (interP.dist(_markPos)<50)
				{
					markTouchPos=interP;
				}		
			}else if (!_ballVelDirChanged&&ball2playerdist<150)
			{
				markTouchPos=interP;
			}else 
			{
				double compensateDist=8;
				if (markDirection){
					if (interP.y()>_markPos.y()){
						markTouchPos.setY(_markPos.y()+compensateDist);
					}else{
						markTouchPos.setY(_markPos.y()-compensateDist);
					}
				}else{
					if (interP.x()>_markPos.x()){
						markTouchPos.setX(_markPos.x()+compensateDist);
					}else{
						markTouchPos.setX(_markPos.x()-compensateDist);
					}
				}
			}
		}
		/*if (ball2playerdist>150 &&_ballVelDirChanged)
		{
			markTouchPos=_markPos;
		}else if(ball2playerdist < 600){	
			if(intersect.Intersectant()){
				CGeoPoint interP = intersect.IntersectPoint();
				if(pointSegment.IsPointOnLineOnSegment(interP)){
					markTouchPos = interP;
					markTouchPos = markTouchPos;
				}
				else {
					if (ball2playerdist<200&&!_ballVelDirChanged && interP.dist(me.Pos())<200 && markField.HasPoint(interP)||ball2playerdist<80)
					{
						markTouchPos=interP;
					}else{
						if (markDirection)
						{
							markTouchPos=interP.y()>_markPos.y()?Utils::CenterOfTwoPoint(_pointA,_markPos):Utils::CenterOfTwoPoint(_pointB,_markPos);
						}else{
							markTouchPos=interP.x()>_markPos.x()?Utils::CenterOfTwoPoint(_pointA,_markPos):Utils::CenterOfTwoPoint(_pointB,_markPos);
						}
					}

				}
			}
		}*/
		GDebugEngine::Instance()->gui_debug_line(_pointA,_pointB);	
	}
	
	GDebugEngine::Instance()->gui_debug_line(kickEnemy.Pos(),kickEnemy.Pos()+Utils::Polar2Vector(800,kickEnemy.Dir()));
	GDebugEngine::Instance()->gui_debug_line(ball.Pos(),ball.Pos()+Utils::Polar2Vector(100,ballVelDir));

	//state excution	
	if (NormalPlayUtils::faceTheirGoal(pVision,robotNum)){
		KickStatus::Instance()->setKick(robotNum,1200);
	}
	TaskT markTouchTask;
	markTouchTask.executor=robotNum;
	switch (state()){
	case Marking:
		markTouchTask.player.pos=_markPos+Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse);
		markTouchTask.player.angle=finalKickDir;
		markTouchTask.player.flag=playerFlag;
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(markTouchTask));
		break;
	case MarkingAgain:
		markTouchTask.player.pos=_markPos+Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse);
		markTouchTask.player.angle=finalKickDir;
		markTouchTask.player.flag=playerFlag;
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(markTouchTask));
		break;
	case GotoFirstPos:
		markTouchTask.player.pos=_markPos+Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse);
		markTouchTask.player.angle=finalKickDir;
		markTouchTask.player.flag=playerFlag;
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(markTouchTask));
		break;
	case Touch:
		markTouchTask.player.pos=markTouchPos+ Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse);
		markTouchTask.player.angle=finalKickDir;
		markTouchTask.player.flag=playerFlag;
		markTouchTask.player.max_acceleration=1400;
		setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(markTouchTask));
		break;
	case AdvanceBall:
		setSubTask(PlayerRole::makeItAdvanceBallV1(robotNum,playerFlag));
		break;
	}

	_lastCycle = pVision->Cycle();
	CStatedTask::plan(pVision);
}




CPlayerCommand* CMarkingTouch::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}
	return NULL;
}

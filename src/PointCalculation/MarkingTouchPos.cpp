#include "MarkingTouchPos.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include <ControlModel.h>
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
#include "Global.h"

namespace
{
	//2.轨迹生成算法使用变量
	enum{
		Marking = 1,
		Touch = 2,	
		Advance = 3,
	};

	bool verBos=false;
	const int State_Counter_Num=3;

	const CGeoPoint theirGoal = CGeoPoint(Param::Field::PITCH_LENGTH/2,0);

}

CMarkingTouchPos::CMarkingTouchPos()
{
	_lastCycle = 0;
	resetState();
}

void CMarkingTouchPos::caculMarkingPos(int areaNum,const CGeoSegment& passSegment,const bool markDirection){
	double XRangeValue=20;
	double YRangeValue=30;
	if (markDirection){
		CGeoLine markline=CGeoLine(_markPos[areaNum],Param::Math::PI/2);
		CGeoLineLineIntersection inter=CGeoLineLineIntersection(markline,passSegment);
		if (inter.Intersectant()&&passSegment.IsPointOnLineOnSegment(inter.IntersectPoint())){
			double interY=inter.IntersectPoint().y();
			double markPosY=_markPos[areaNum].y();
			if (interY>_markPos[areaNum].y()+YRangeValue)
			{
				markPosY=_markPos[areaNum].y()+YRangeValue;
			}else if (interY<_markPos[areaNum].y()-YRangeValue)
			{
				markPosY=_markPos[areaNum].y()-YRangeValue;
			}else{
				markPosY=interY;
			}
			_markPos[areaNum].setY(markPosY);
		}
		//_pointA=CGeoPoint(_markPos[areaNum].x(),leftUpPos.y()),_pointB=CGeoPoint(_markPos[areaNum].x(),rightDownPos.y());
	}else{
		CGeoLine markline=CGeoLine(_markPos[areaNum],0);
		CGeoLineLineIntersection inter=CGeoLineLineIntersection(markline,passSegment);
		if (inter.Intersectant()&&passSegment.IsPointOnLineOnSegment(inter.IntersectPoint())){
			double interX=inter.IntersectPoint().x();
			double markPosX=_markPos[areaNum].x();
			if (interX>_markPos[areaNum].x()+XRangeValue)
			{
				markPosX=_markPos[areaNum].x()+XRangeValue;
			}else if (interX<_markPos[areaNum].x()-XRangeValue)
			{
				markPosX=_markPos[areaNum].x()-XRangeValue;
			}else{
				markPosX=interX;
			}
			_markPos[areaNum].setX(markPosX);
		}
		//_pointA=CGeoPoint(leftUpPos.x(),_markPos[areaNum].y()),_pointB=CGeoPoint(rightDownPos.x(),_markPos[areaNum].y());
	}
}

void CMarkingTouchPos::resetState(){

	_kickEnemyNum=0;
	for (int i=0;i<MaxNum;i++)
	{
		setState(BEGINNING,i);
		_markEnemyNum[MaxNum]=0;
		_marktoTouchCouter[MaxNum]=0;
		_touchtoAdvanceCouter[MaxNum]=0;
		_ballVelChangeCouter[MaxNum]=0;
		_ballVelDirChanged[MaxNum]=false;

	}
	
}

CGeoPoint CMarkingTouchPos::caculMarkingTouchPos(int areaNum,CGeoPoint leftUpPos,CGeoPoint rightDownPos,bool markDirection){

	const MobileVisionT& ball = vision->Ball();
	CGeoPoint initMarkPos;
	if (markDirection)
	{
		double markX=fabs(leftUpPos.x()-ball.X())<fabs(rightDownPos.x()-ball.X())?leftUpPos.x():rightDownPos.x();
		initMarkPos=CGeoPoint(markX,(leftUpPos.y()+rightDownPos.y())/2.0);
	}else{
		double markY=fabs(leftUpPos.y()-ball.Y())<fabs(rightDownPos.y()-ball.Y())?leftUpPos.y():rightDownPos.y();
		initMarkPos=CGeoPoint((leftUpPos.x()+rightDownPos.x())/2.0,markY);
	}

	CGeoPoint goalPos=CGeoPoint(Param::Field::PITCH_LENGTH/2,0);
	double finalKickDir=(goalPos-initMarkPos).dir();

	if ((_lastLeftUpPos[areaNum].x()!=leftUpPos.x()||_lastLeftUpPos[areaNum].y()!=leftUpPos.y()||
		_lastRightDownPos[areaNum].x()!=rightDownPos.x()||_lastRightDownPos[areaNum].y()!=rightDownPos.y())&&
		state(areaNum)!=Touch)
	{
		_markEnemyNum[areaNum]=0;
		_marktoTouchCouter[areaNum]=0;
		_touchtoAdvanceCouter[areaNum]=0;
		_ballVelChangeCouter[areaNum]=0;
		_ballVelDirChanged[areaNum]=false;
		_markPos[areaNum]=initMarkPos;
	}
	_lastLeftUpPos[areaNum]=leftUpPos;
	_lastRightDownPos[areaNum]=rightDownPos;

	if ( vision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 ){
		resetState();
		_markPos[areaNum]=initMarkPos;
	}



	//cout<<"enemy"<<_kickEnemyNum<<endl;
	double ballVelDir = ball.Vel().dir();									// 球速方向
	double ballVelReverse = Utils::Normalize(ballVelDir+Param::Math::PI);	// 球速反向
	double kickDirReverse = Utils::Normalize(finalKickDir+Param::Math::PI);		// 目标方向反向
	double angleDiff_ballVelDir2kickDirReverse = fabs(Utils::Normalize(finalKickDir - ballVelReverse));


	//true = horizal
	CGeoRectangle markField = CGeoRectangle(leftUpPos,rightDownPos);
	int markEnemyNum=0;
	for (int i=1;i<=Param::Field::MAX_PLAYER;i++)
	{
		if (vision->TheirPlayer(i).Valid()){
			if (markField.HasPoint(vision->TheirPlayer(i).Pos()))
			{
				if (markEnemyNum!=0)
				{
					if (vision->TheirPlayer(i).Pos().dist(ball.Pos())<vision->TheirPlayer(markEnemyNum).Pos().dist(ball.Pos())-20)
					{
						markEnemyNum=i;
					}
				}else{
					markEnemyNum=i;
				}
			}
		}
	}
	//if (_markEnemyNum[areaNum]==0 || !vision->TheirPlayer(_markEnemyNum[areaNum]).Valid()
	//	||!markField.HasPoint(vision->TheirPlayer(_markEnemyNum[areaNum]).Pos())){
	//		_markEnemyNum[areaNum]=markEnemyNum;
	//}

	if (state(areaNum)!=Touch){
		_kickEnemyNum=NormalPlayUtils::getTheirMostClosetoPos(vision,ball.Pos());
		_markEnemyNum[areaNum]=markEnemyNum;
	}
	const PlayerVisionT kickEnemy=vision->TheirPlayer(_kickEnemyNum);
	CGeoLine passLine=CGeoLine(ball.Pos(),kickEnemy.Dir());
	CGeoSegment passSegment=CGeoSegment(ball.Pos(),ball.Pos()+Utils::Polar2Vector(800,kickEnemy.Dir()));
	GDebugEngine::Instance()->gui_debug_line(ball.Pos(),ball.Pos()+Utils::Polar2Vector(600,kickEnemy.Dir()),COLOR_BLACK);
	if (markEnemyNum==0){
		_markPos[areaNum]=initMarkPos;		
		caculMarkingPos(areaNum,passSegment,markDirection);
		return _markPos[areaNum];
	}else{
		const PlayerVisionT he=vision->TheirPlayer(_markEnemyNum[areaNum]);
		const double hetoBallDir=(ball.Pos()-he.Pos()).dir();
		const double antiHetoBallDir=Utils::Normalize(hetoBallDir+Param::Math::PI);
		const CGeoSegment hetoBallSeg = CGeoSegment(he.Pos()+Utils::Polar2Vector(8,hetoBallDir),ball.Pos());
		double kickEnemyToBallDir=(ball.Pos()-kickEnemy.Pos()).dir();
		double diffAngleBallVel_HeDir=fabs(Utils::Normalize(ball.Vel().dir()-kickEnemy.Dir()));
		double diffAngleBallVel_HetoBall=fabs(Utils::Normalize(kickEnemyToBallDir-ball.Vel().dir()));
		double diffAngleHeDir_HetoBall=fabs(Utils::Normalize(kickEnemyToBallDir-kickEnemy.Dir()));


		bool isBallVelOk=fabs(Utils::Normalize(vision->Ball(_lastCycle-1).Vel().dir()-ball.Vel().dir()))<Param::Math::PI*8/180;
		isBallVelOk=true;
		bool isBallPassed=//isBallVelOk&&
			//&&diffAngleBallVel_HeDir<Param::Math::PI*20/180&&diffAngleBallVel_HetoBall<Param::Math::PI*20/180
			diffAngleHeDir_HetoBall<Param::Math::PI*30/180
			&&kickEnemy.Pos().dist(ball.Pos())>15&&ball.Valid()&&ball.Vel().mod()>100;
		//&&(fabs(Utils::Normalize(antiMetoBallDir-ballVelDir))<Param::Math::PI*45/180
		//||fabs(Utils::Normalize(antiHetoBallDir-ballVelDir))<Param::Math::PI*45/180);

		bool failTouch=ball.Vel().mod()<100;




		switch (state(areaNum))
		{
		case BEGINNING:
			setState(Marking,areaNum);
		case Marking:
			if (isBallPassed){
				_marktoTouchCouter[areaNum]++;
				if (_marktoTouchCouter[areaNum]==1){
					CGeoLine markLine;
					double rangeValue=35;
					if (markDirection){
						markLine=CGeoLine(_markPos[areaNum],Param::Math::PI/2);
					}else{
						markLine=CGeoLine(_markPos[areaNum],0);
					}
					CGeoLineLineIntersection inter=CGeoLineLineIntersection(markLine,passLine);
					if (inter.Intersectant()&&inter.IntersectPoint().dist(_markPos[areaNum])<50)
					{
						_markPos[areaNum]=inter.IntersectPoint();
					}
					if (markDirection)
					{
						_pointA[areaNum]=CGeoPoint(_markPos[areaNum].x(),_markPos[areaNum].y()+rangeValue),_pointB[areaNum]=CGeoPoint(_markPos[areaNum].x(),_markPos[areaNum].y()-rangeValue);
					}else{
						_pointA[areaNum]=CGeoPoint(_markPos[areaNum].x()+rangeValue,_markPos[areaNum].y()),_pointB[areaNum]=CGeoPoint(_markPos[areaNum].x()-rangeValue,_markPos[areaNum].y());
					}

					setState(Touch,areaNum);
					_marktoTouchCouter[areaNum]=0;
				}
			}
			break;
		case Touch:
			if (failTouch)
			{
				_touchtoAdvanceCouter[areaNum]++;
				if (_touchtoAdvanceCouter[areaNum]==180){
					setState(BEGINNING,areaNum);
					_touchtoAdvanceCouter[areaNum]=0;
				}
			}
			//if (WorldModel::Instance()->IsBallKicked(robotNum))
			//{
			//	setState(BEGINNING);
			//}
			break;
		case Advance:
			break;
		}
		if(verBos) cout<<areaNum<<" stateJump: "<<state(areaNum)<<" "<<isBallPassed<<endl;
		if (state(areaNum)==Marking)
		{
			if (markDirection){
				_markPos[areaNum]=he.Pos()+Utils::Polar2Vector(35,hetoBallDir);
			}else{
				_markPos[areaNum]=he.Pos()+Utils::Polar2Vector(35,hetoBallDir);
			}
			caculMarkingPos(areaNum,passSegment,markDirection);
			_markPos[areaNum]=_markPos[areaNum]+Utils::Polar2Vector(7,kickDirReverse);
		}



		CGeoPoint markTouchPos=_markPos[areaNum];
		if (state(areaNum)==Touch)
		{
			CGeoLine pointLine = CGeoLine(_pointA[areaNum],_pointB[areaNum]);
			CGeoSegment pointSegment = CGeoSegment(_pointA[areaNum],_pointB[areaNum]);
			CGeoLine ballLine = CGeoLine(ball.Pos(),ball.Pos()+Utils::Polar2Vector(100,ballVelDir));
			CGeoLineLineIntersection intersect = CGeoLineLineIntersection(pointLine,ballLine);


			if (fabs(Utils::Normalize(vision->Ball().Vel().dir()-vision->Ball(_lastCycle-2).Vel().dir()))>Param::Math::PI*3.5/180)
			{
				_ballVelChangeCouter[areaNum]++;
				_ballVelChangeCouter[areaNum]=min(_ballVelChangeCouter[areaNum],4);
			}else{
				_ballVelChangeCouter[areaNum]--;
				_ballVelChangeCouter[areaNum]=max(_ballVelChangeCouter[areaNum],0);
			}
			if (_ballVelChangeCouter[areaNum]==3){
				_ballVelDirChanged[areaNum]=true;		
			}
			if (_ballVelChangeCouter[areaNum]==0)
			{
				_ballVelDirChanged[areaNum]=false;
			}
			if (verBos) cout<<"ballvel"<<_ballVelDirChanged[areaNum]<<" "<<_ballVelChangeCouter[areaNum]<<endl;

			if (_ballVelDirChanged[areaNum]&&ball.Pos().dist(_markPos[areaNum])>120)
			{
				markTouchPos=_markPos[areaNum];
			}else{	
				if(intersect.Intersectant()){
					CGeoPoint interP = intersect.IntersectPoint();
					if(pointSegment.IsPointOnLineOnSegment(interP)){
						markTouchPos = interP;
						markTouchPos = markTouchPos + Utils::Polar2Vector(Param::Vehicle::V2::TOUCH_SHIFT_DIST,kickDirReverse);
					}
					else {
						if (!_ballVelDirChanged[areaNum] && interP.dist(he.Pos())<120 && markField.HasPoint(interP))
						{
							markTouchPos=interP+Utils::Polar2Vector(Param::Vehicle::V2::PLAYER_FRONT_TO_CENTER+2,kickDirReverse);
						}else{
							if (markDirection)
							{
								markTouchPos=interP.y()>_markPos[areaNum].y()?Utils::CenterOfTwoPoint(_pointA[areaNum],_markPos[areaNum]):Utils::CenterOfTwoPoint(_pointB[areaNum],_markPos[areaNum]);
							}else{
								markTouchPos=interP.x()>_markPos[areaNum].x()?Utils::CenterOfTwoPoint(_pointA[areaNum],_markPos[areaNum]):Utils::CenterOfTwoPoint(_pointB[areaNum],_markPos[areaNum]);
							}
						}

					}
				}
			}
			//GDebugEngine::Instance()->gui_debug_line(_pointA[areaNum],_pointB[areaNum]);
		}
		//cout<<markTouchPos<<" "<<_pointB<<endl;		

		GDebugEngine::Instance()->gui_debug_line(ball.Pos(),ball.Pos()+Utils::Polar2Vector(100,ballVelDir));
		//GDebugEngine::Instance()->gui_debug_line(_pointA,_pointB);

		//state excution	
		switch (state(areaNum)){
		case Marking:
			//cout<<"marking"<<endl;
			return _markPos[areaNum];
			break;
		case Touch:
			//cout<<"touch"<<endl;
			return markTouchPos;
			break;
		case Advance:
			return  _markPos[areaNum];
			break;

		}
	}
	_lastCycle = vision->Cycle();
}

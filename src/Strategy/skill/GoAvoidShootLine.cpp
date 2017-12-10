#include <VisionModule.h>
#include "skill/Factory.h"
#include "GoAvoidShootLine.h"
#include <utils.h>
#include <GDebugEngine.h>
#include "utils.h"
#include "NormalPlayUtils.h"
#include "KickDirection.h"
namespace{
	CGeoPoint mediumPos=CGeoPoint(9999,9999);
}

CGoAvoidShootLine::CGoAvoidShootLine()
{
	{
		//DECLARE_PARAM_READER_BEGIN(CGotoPositionV2)
		//READ_PARAM(TRAJECTORY_METHORD)
		//	DECLARE_PARAM_READER_END
	}	
	_lastCycle = 0;
	enum AvoidShootLineState{
		Avoid=0,
		NotAvoid=1
	};
}


void CGoAvoidShootLine::plan(const CVisionModule* pVision)
{
	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1 ){
		setState(BEGINNING);
		_stateCouter=0;
		mediumPos=CGeoPoint(9999,9999);
	}
	const MobileVisionT& ball = pVision->Ball();
	const int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const int playerFlag = task().player.flag;
	const PlayerVisionT& shooter=pVision->OurPlayer(task().ball.Sender);
	const CGeoPoint avoidPos=shooter.Pos();
	const CGeoPoint targetPos=task().player.pos;
	const double arriveAngle=task().player.angle;
	
	const CGeoPoint theirGoalPos=CGeoPoint(Param::Field::PITCH_LENGTH/2,0);
	
	const double avoidPostoGoalDir=(theirGoalPos-avoidPos).dir();
	const double avoidPostoMeDir=(me.Pos()-avoidPos).dir();

	bool willBlock=false;
	bool willCrash=false;

	static int lastSideY=Utils::Normalize(avoidPostoMeDir-avoidPostoGoalDir)>0?1:-1;

	int sideY=lastSideY;

	double buffer=Param::Math::PI*3/180;
	if (lastSideY==-1){
		if (Utils::Normalize(avoidPostoMeDir-avoidPostoGoalDir)>buffer){
			sideY=1;
			lastSideY=sideY;
		}
	}else{
		if (Utils::Normalize(avoidPostoMeDir-avoidPostoGoalDir)<-buffer){
			sideY=-1;
			lastSideY=sideY;
		}
	}

	CVector avoidPostoGoalVector=avoidPos-theirGoalPos;
	CVector metoTargetVector=targetPos-me.Pos();
	CGeoLine avoidPostoGoalLine=CGeoLine(avoidPos,theirGoalPos);
	CGeoLine metoTargetLine=CGeoLine(me.Pos(),targetPos);
	CGeoSegment avoidPostoGoalSeg=CGeoSegment(avoidPos,theirGoalPos);
	CGeoSegment metoTargetSeg=CGeoSegment(me.Pos(),targetPos+metoTargetVector/metoTargetVector.mod()*20);
	CGeoLineLineIntersection Inter=CGeoLineLineIntersection(metoTargetLine,avoidPostoGoalLine);
	CGeoPoint InterPoint=CGeoPoint(9999,9999);
	if (Inter.Intersectant()){
		InterPoint=Inter.IntersectPoint();
	}
	if (metoTargetSeg.IsPointOnLineOnSegment(InterPoint) && avoidPostoGoalSeg.IsPointOnLineOnSegment(InterPoint)){
		willBlock=true;
	}

	bool canShoot=shooter.Pos().dist(ball.Pos())<60&&fabs(Utils::Normalize(shooter.Dir()-avoidPostoGoalDir))<Param::Math::PI*120/180;
	canShoot=true;

	const double shooterProjDist=Utils::pointToLineDist(shooter.Pos(),metoTargetLine);
	bool shooterBlock=metoTargetSeg.IsPointOnLineOnSegment(metoTargetSeg.projection(shooter.Pos()))
		&&shooterProjDist<15;

	TaskT goTask(task());
	if (canShoot&&willBlock&&!NormalPlayUtils::isEnemyBlockShootLine(avoidPos,avoidPostoGoalDir,50)){
		double projDir=Utils::Normalize(avoidPostoGoalDir+sideY*Param::Math::PI*100/180);
		//CGeoPoint goPoint=Utils::CenterOfTwoPoint(theirGoalPos,shooter.Pos());
		CGeoPoint goPoint=avoidPostoGoalSeg.projection(targetPos);
		if (goPoint.x()>(avoidPos.x()+theirGoalPos.x())/2){
			goPoint.setX(avoidPos.x()+(theirGoalPos.x()-avoidPos.x())*1/2);
			goPoint.setY(avoidPos.y()+(theirGoalPos.y()-avoidPos.y())*1/2);
		}else{
			goPoint.setX(avoidPos.x()+(theirGoalPos.x()-avoidPos.x())*1/4);
			goPoint.setY(avoidPos.y()+(theirGoalPos.y()-avoidPos.y())*1/4);
		}
		mediumPos=goPoint+Utils::Polar2Vector(80,projDir);
		if (mediumPos.x()>Param::Field::PITCH_LENGTH/2-10){
			double tempY=Param::Field::PITCH_WIDTH/2-fabs(shooter.Pos().y());
			tempY=max(100,tempY);
			tempY=min(215,tempY);
			mediumPos.setY(Utils::Sign(shooter.Y())*fabs(tempY));
			mediumPos.setX(Param::Field::PITCH_LENGTH/2-10);
		}
		goTask.player.pos=mediumPos;
		goTask.player.angle=(theirGoalPos-me.Pos()).dir();
	}else if (shooterBlock&&me.Pos().x()<shooter.Pos().x()){
		mediumPos=CGeoPoint(shooter.Pos().x()-50,shooter.Pos().y()+sideY*20);
		goTask.player.pos=mediumPos;
		goTask.player.angle=(theirGoalPos-me.Pos()).dir();
	}
	setSubTask(TaskFactoryV2::Instance()->SmartGotoPosition(goTask));
	CStatedTask::plan(pVision);
}

CPlayerCommand* CGoAvoidShootLine::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}
	return NULL;
}
#include "SupportPos.h"

#include "BallSpeedModel.h"
#include "BestPlayer.h"
#include "ChipBallJudge.h"
#include "GDebugEngine.h"
#include "TaskMediator.h"
#include "WorldModel.h"
#include "RobotSensor.h"
#include "param.h"
#include "utils.h"

#include <cmath>

using namespace Param::Field;
using namespace Param::Math;
using namespace Param::Vehicle::V2;
namespace{
	enum{
		Left,
		Right
	};
	const bool _debugEngine= false;
	const bool intendedCreateFreeKick = false;
	const int defendAreaBuffer = 30;
	const double shootPower = 1000;
	const double shootSpeed = 800;

	const double vel_factor = 0.9;
	const double cal_acc = 10; //摩擦因子 Copied from BallSpeedModel

	CGeoRectangle enemyDefendArea = CGeoRectangle(CGeoPoint(PITCH_LENGTH / 2, PENALTY_AREA_WIDTH / 2 + defendAreaBuffer), 
		CGeoPoint(PITCH_LENGTH / 2 - PENALTY_AREA_DEPTH- defendAreaBuffer, -PENALTY_AREA_WIDTH / 2 - defendAreaBuffer));
	CGeoSegment goalLine = CGeoSegment(CGeoPoint(PITCH_LENGTH / 2, PENALTY_AREA_WIDTH / 2), CGeoPoint(PITCH_LENGTH / 2, -PENALTY_AREA_WIDTH / 2));   //球门线段
}

CSupportPos::CSupportPos(){
	_lastCycle = 0;
	for (int _i = 0; _i < 10; _i++){
		_defenderList[_i] = 0;
	}
	reflectDir = 0;
	reflectPos = CGeoPoint(PITCH_LENGTH / 2, 0);
	_supportPos = CGeoPoint(0, 0);
	ourLeaderNum = 0;
	ourSupporterNum = 0;
	_leaderChangeCounter = 0;
	_supporterChangeCounter = 0;
	_roleChangeCounter = 0;
	_savedCycle = 0;
	_isRoleChanged = false;
}	

void CSupportPos::generatePos(const CVisionModule* pVision,int num,int num2){
	if (pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.1){
		for (int _i = 0; _i < 10; _i++){
			_defenderList[_i] = 0;
		}
		reflectDir = 0;
		reflectPos = CGeoPoint(PITCH_LENGTH / 2, 0);
		_supportPos = CGeoPoint(0, 0);
		_lastPos = CGeoPoint(1000, 1000);
		ourLeaderNum = 0;
		ourSupporterNum = 0;
		_leaderChangeCounter = 0;
		_supporterChangeCounter = 0;
		_savedCycle = pVision->Cycle();
		_roleChangeCounter = 0;
		_isRoleChanged = false;
	}
	//对Leader\Supporter的匹配保持一定帧，来维持跑点的恒定（若角色由Lua直传的话，就不需要维持一定帧了）
	//由Lua直接传Leader的车号，然后根据Leader的车号来计算助攻车的跑点（助攻车的跑点基本依赖于主攻车的位置以及场上的形势）
	if (ourLeaderNum == 0){
		ourLeaderNum = num;
		_leaderChangeCounter = 0;
	}
	if (num != ourLeaderNum){
		_leaderChangeCounter++;
		_supporterChangeCounter++;
	}
	else{
		_leaderChangeCounter=0;
		_supporterChangeCounter=0;
	}
	if (_leaderChangeCounter == 1){
		ourLeaderNum = num;
		_leaderChangeCounter = 0;
		_roleChangeCounter++;
	}
	if (num2 == 0){
		if (ourSupporterNum == 0){
			if (BestPlayer::Instance()->ourFastestPlayerToBallList().at(0).num != num){
				ourSupporterNum = BestPlayer::Instance()->ourFastestPlayerToBallList().at(0).num;
			}
			else{
				ourSupporterNum = BestPlayer::Instance()->ourFastestPlayerToBallList().at(1).num;
			}
			_supporterChangeCounter = 0;
		}
		if (_supporterChangeCounter == 1){
			if (BestPlayer::Instance()->ourFastestPlayerToBallList().at(0).num != num){
				ourSupporterNum = BestPlayer::Instance()->ourFastestPlayerToBallList().at(0).num;
			}
			else{
				ourSupporterNum = BestPlayer::Instance()->ourFastestPlayerToBallList().at(1).num;
			}
			_supporterChangeCounter = 0;
		}
	}
	else
	{
		ourSupporterNum = num2;
	}
	

	PlayerVisionT ourSupporter = pVision->OurPlayer(ourSupporterNum);
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	const int enemyDefenderAmount = calcEnemyDefenderAmount(pVision);
	double reflectPower;
	CGeoPoint defaultPos = CGeoPoint(ourLeader.Pos().x(), -ourLeader.Pos().y());  //对方无防守车时，默认助攻点与Leader对称
	CGeoCirlce bestZone;//最佳进攻区域
	CGeoPoint  bestPoint;//最佳进攻点
	bool isNeedStay = false;//是否需要维持之前的助攻点
	if ( enemyDefenderAmount!= 0&&isBallInOurControl(pVision)){
		if (calcBallReflectDirAndPos(pVision, calcEnemyDefenderAmount(pVision)) != 0){
			reflectPower = calcBallReflectPower(pVision, ourLeader.Pos(), reflectPos);
			bestZone  = calcTheBestZone(pVision,1.5*reflectPower);
			bestPoint = calcTheBestPoint(pVision, bestZone);
			if (_supportPos == CGeoPoint(0, 0)){
				_supportPos = bestPoint;
			}
			if (!BallStatus::Instance()->IsBallKickedOut()&&(bestPoint-CGeoPoint(PITCH_LENGTH/2,0)).mod()>100){
				_supportPos = bestPoint;
			}
		}
	}
	else{
		_supportPos = defaultPos;
	}
	checkAvoidShootLine(pVision);
	checkAvoidCenterLine(pVision, 130, 30);
	

	if (_lastPos == CGeoPoint(1000, 1000)){
		_lastPos = _supportPos;
	}
	if (!ball.Valid() || BallStatus::Instance()->IsBallKickedOut()){
		_supportPos = _lastPos;
	}
	_lastPos = _supportPos;
	//Debug信息
	if (_debugEngine){
		//GDebugEngine::Instance()->gui_debug_msg(ourLeader.Pos()+Utils::Polar2Vector(PLAYER_SIZE/2,0), "LLLLL");
		//GDebugEngine::Instance()->gui_debug_msg(ourSupporter.Pos() + Utils::Polar2Vector(PLAYER_SIZE / 2, 0), "SSSSS");
		//GDebugEngine::Instance()->gui_debug_arc(bestZone.Center(), bestZone.Radius(), 0, 360, 1);
		GDebugEngine::Instance()->gui_debug_arc(_supportPos, 10, 0, 360, 1);
		//GDebugEngine::Instance()->gui_debug_x(_supportPos);
	}

}

CGeoPoint CSupportPos::getSupPosNew(const CVisionModule* pVision,int num1,int num2){
	if (advancePassed == true)
		return _supportPos;
	if (pVision->Cycle() == _lastCycle &&_lastCycle != 0 ) {
		return _supportPos;
	}
	else{
		_lastCycle = pVision->Cycle();
	}
	//cout<<"debug:"<<num2<<endl;
	generatePos(pVision,num1,num2);
	//cout << "real:"<<_supportPos.x() << "   " << _supportPos.y() << endl;
	return _supportPos;
}

CGeoPoint CSupportPos::getSupPos(const CVisionModule* pVision,int num){
	if (advancePassed == true)
		return _supportPos;
	if (pVision->Cycle() == _lastCycle &&_lastCycle != 0 ) {
		return _supportPos;
	}
	else{
		_lastCycle = pVision->Cycle();
	}
	generatePos(pVision,num);
	return _supportPos;
}


//判断球是否在我方的控制下
bool CSupportPos::isBallInOurControl(const CVisionModule* pVision){
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	const bool isSensored = RobotSensor::Instance()->IsInfoValid(ourLeaderNum) && RobotSensor::Instance()->IsInfraredOn(ourLeaderNum);	//If Infrared on 
	const bool isHasBallInVision = (ball.Pos() - ourLeader.Pos()).mod() < PLAYER_SIZE + 3 && fabs((ball.Pos() - ourLeader.Pos()).dir() - ourLeader.Dir()) < PI * 6 / 180;
	const bool isUnderControlled = (ourLeader.Pos() - ball.Pos()).mod() < (enemyLeader.Pos() - ball.Pos()).mod();
	return isSensored || isHasBallInVision || isUnderControlled;
}



//统计在禁区附近的防守车的总数量
int CSupportPos::calcEnemyDefenderAmount(const CVisionModule* pVision){
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	int enemyDefenderAmount=0;
	int enemyCount = 0;
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const int enemyAmount =VisionModule::Instance()->getTheirValidNum(); //Size存疑 与列表的对应关系需要校对
	if (enemyAmount == 0){
		return 0;
	}
	if (BestPlayer::Instance()->getTheirGoalie() != 0){
		_defenderList[enemyCount] = BestPlayer::Instance()->getTheirGoalie();
		enemyCount++;
		enemyDefenderAmount++;
	}
	for (int _i = 0; _i < enemyAmount; _i++){
		if (enemyDefendArea.HasPoint(pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(_i).num).Pos()) 
			&& BestPlayer::Instance()->theirFastestPlayerToBallList().at(_i).num != BestPlayer::Instance()->getTheirGoalie()){
			enemyDefenderAmount++;
			_defenderList[enemyCount] = BestPlayer::Instance()->theirFastestPlayerToBallList().at(_i).num;
			enemyCount++;
		}
	}
	return enemyDefenderAmount;
}


//计算反弹角度与反弹点,当且仅当对方存在防守球员时（包括守门员)
//无防守队员时，返回0,计算成功返回1，计算失败返回2
//计算结果存储在reflectPos与reflectDir中
double CSupportPos::calcBallReflectDirAndPos(const CVisionModule* pVision, const int enemyDefenderAmount){
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	int currentDefender=0;//当前索引防守者
	int effectDefender = 0;//实际生效防守者(一般实际比赛中，一次射门仅由一辆防守车实际完成阻挡）
	bool isAnyDefenderEffect = false;
	CGeoPoint TreflectPos;  //Temp 临时变量
	CGeoCirlce defenderCircle;
	CGeoSegment defenderFaceSeg;
	double defenderFaceDir;
	double defaultDir;
	double finalDir = 0;
	double minDist = PITCH_LENGTH;

	finalDir = (CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0) - ourLeader.Pos()).dir();
	static bool isInBack = false;
	
	if (ourLeader.Pos().x()<-150){
		isInBack = true;
	}
	if (ourLeader.Pos().x()>50){
		isInBack = false;
	}

	double antiFinalDir = Utils::Normalize(finalDir + PI);
	CGeoLine shootLine = CGeoLine(pVision->OurPlayer(ourLeaderNum).Pos(), finalDir);

	
	//只有一名防守队员，默认此防守队员仅为守门员,且守门员横在门前
	if (enemyDefenderAmount == 0){
		GDebugEngine::Instance()->gui_debug_line(CGeoPoint(PITCH_LENGTH / 2, PENALTY_AREA_WIDTH / 2), CGeoPoint(PITCH_LENGTH / 2, -PENALTY_AREA_WIDTH / 2),2);
		CGeoLineLineIntersection inter = CGeoLineLineIntersection(shootLine, goalLine);
		reflectPos = inter.IntersectPoint();
		reflectDir = PI - finalDir; 
	}

	//多名防守队员，查找第一个碰撞点,若无碰撞点，则处理为默认在球门横向处发生碰撞。
	else{
		for (int _i = 0; _i < enemyDefenderAmount; _i++){
			currentDefender = _defenderList[_i];
			defenderFaceDir = pVision->TheirPlayer(currentDefender).Dir();
			defaultDir = (pVision->TheirPlayer(currentDefender).Pos() - CGeoPoint(PITCH_LENGTH / 2, 0)).dir();
			if (fabs(defenderFaceDir-defaultDir)>PI*30/180){
				defenderFaceDir = defaultDir;  //当防守车的朝向存在问题时，默认朝向为背对球门
			}
			defenderCircle = CGeoCirlce(pVision->TheirPlayer(currentDefender).Pos(), PLAYER_SIZE);
			CGeoLineCircleIntersection inter = CGeoLineCircleIntersection(shootLine, defenderCircle);
			//由于机器人一般为存在缺口的圆弧，对于真实情况而言，射门一般会被防守车的正面所阻挡，此处将碰撞点统一假设在与圆弧相切且与小车朝向平行的线段上，与真实碰撞点相差不会超过3cm，在实际跑点的过程中可以忽略
			if (inter.intersectant()){
				isAnyDefenderEffect = true;
				defenderFaceSeg = CGeoSegment((pVision->TheirPlayer(currentDefender).Pos() + Utils::Polar2Vector(PLAYER_SIZE, defenderFaceDir) + Utils::Polar2Vector(PLAYER_SIZE, defenderFaceDir + PI / 2)),
					(pVision->TheirPlayer(currentDefender).Pos() + Utils::Polar2Vector(PLAYER_SIZE, defenderFaceDir) + Utils::Polar2Vector(PLAYER_SIZE, defenderFaceDir - PI / 2)));
				if (_debugEngine){
					GDebugEngine::Instance()->gui_debug_line((pVision->TheirPlayer(currentDefender).Pos() + Utils::Polar2Vector(PLAYER_SIZE, antiFinalDir) + Utils::Polar2Vector(100, defenderFaceDir + PI / 2)),
						(pVision->TheirPlayer(currentDefender).Pos() + Utils::Polar2Vector(PLAYER_SIZE, defenderFaceDir) + Utils::Polar2Vector(100, defenderFaceDir - PI / 2)), 2);
				}
				CGeoLineLineIntersection inter2 = CGeoLineLineIntersection(shootLine, defenderFaceSeg);
				TreflectPos = inter2.IntersectPoint();
				if ((TreflectPos - ourLeader.Pos()).mod() < minDist){
					effectDefender = currentDefender;
					minDist = (TreflectPos - ourLeader.Pos()).mod();
					reflectPos = TreflectPos;
					reflectDir = antiFinalDir + 2 * (defenderFaceDir - antiFinalDir);  //To Check
				}
			}
		}
		if (!isAnyDefenderEffect){
			CGeoLineLineIntersection inter = CGeoLineLineIntersection(shootLine, goalLine);
			reflectPos = inter.IntersectPoint();
			reflectDir = PI - finalDir; 
		}
	}
	if (fabs(reflectDir - antiFinalDir) < PI * 30 / 180){
		if (reflectDir >= antiFinalDir){
			reflectDir = antiFinalDir + PI * 30 / 180;
		}
		else{
			reflectDir = antiFinalDir - PI * 30 / 180;
		}
	}
	if (_debugEngine){
		GDebugEngine::Instance()->gui_debug_line(ourLeader.Pos(), ourLeader.Pos() + Utils::Polar2Vector(1000, finalDir));
		GDebugEngine::Instance()->gui_debug_x(reflectPos);
		GDebugEngine::Instance()->gui_debug_line(reflectPos, reflectPos + Utils::Polar2Vector(800, reflectDir));
	}
	return 1;
}



//计算反弹力度,根据射门点，以及反弹点计算反弹力度(力度将用反弹距离表征）（仅反映大概力度，没有必要太准确）
//需要实地测试
//此处假设碰撞无损失
//关于球速模型，Copied from BallSpeedModel
double CSupportPos::calcBallReflectPower(const CVisionModule *pVision,CGeoPoint fisrtPos, CGeoPoint secondPos){
	double dist = (fisrtPos - secondPos).mod();
	double power = (max(shootSpeed - 2.5*dist - 100, 0) + 50) / 2;
	if (power < 100){
		power = 100;
	}
	else if (power>150){
		power = 150;
	}
	return power;
}


//计算最佳分区,根据反弹点来确定，同时反弹力度会对分区半径存在影响
//To Check
CGeoCirlce CSupportPos::calcTheBestZone(const CVisionModule *pVision, const double power){
	CGeoPoint centerPos = reflectPos + Utils::Polar2Vector(power, reflectDir);
	if (_debugEngine)
		GDebugEngine::Instance()->gui_debug_arc(centerPos, (1000 - power) / 12, 0, 360, 1);
	return CGeoCirlce(centerPos, (1000 - power) / 12);
}




//计算最佳点
//最佳点衡限制在最佳区域内，若算点存在问题，可能是最佳区域的范围不够所导致的
CGeoPoint CSupportPos::calcTheBestPoint(const CVisionModule *pVision,const CGeoCirlce bestZone){
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	const bool isEnemyExist = BestPlayer::Instance()->theirFastestPlayerToBallList().size();
	CGeoPoint targetPos = bestZone.Center();
	double projDist = 0;
	double targetDir = (CGeoPoint(PITCH_LENGTH / 2, 0) - targetPos).dir();
	CGeoSegment targetSeg = CGeoSegment(CGeoPoint(PITCH_LENGTH / 2, 0), targetPos);
	CGeoLine targetLine = CGeoLine(CGeoPoint(PITCH_LENGTH / 2, 0), targetPos);
	int currentDefender = 0;
	if (isEnemyExist){
		for (int _i = 0; _i < BestPlayer::Instance()->theirFastestPlayerToBallList().size(); _i++){
			currentDefender=BestPlayer::Instance()->theirFastestPlayerToBallList().at(_i).num;
			if ((pVision->TheirPlayer(currentDefender).Pos() - bestZone.Center()).mod() < bestZone.Radius()&&
				targetSeg.IsPointOnLineOnSegment(targetLine.projection(pVision->TheirPlayer(currentDefender).Pos()))){
				projDist= (targetLine.projection(pVision->TheirPlayer(currentDefender).Pos())-targetPos).mod();
				targetPos = targetPos + Utils::Polar2Vector(projDist + PLAYER_SIZE, targetDir);
				if (!bestZone.HasPoint(targetPos)){
					targetPos = bestZone.Center() + Utils::Polar2Vector(bestZone.Radius(), targetDir);
					return targetPos;
				}
				targetDir = (CGeoPoint(PITCH_LENGTH / 2, 0) - targetPos).dir();
				targetSeg = CGeoSegment(CGeoPoint(PITCH_LENGTH / 2, 0), targetPos);
				targetLine = CGeoLine(CGeoPoint(PITCH_LENGTH / 2, 0), targetPos);
			}
			
		}
	}
	return targetPos;
}

//避开射门路径
void CSupportPos::checkAvoidShootLine(const CVisionModule *pVision){
	PlayerVisionT ourSupporter = pVision->OurPlayer(ourSupporterNum);
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	double finalDir = 0;
	double changeDir = 0;
	bool needDetourToAvoid = false;
	CGeoPoint leaderDeviantPos = CGeoPoint(0, 0);
	if (fabs(ourLeader.Dir() - finalDir) < PI * 7 / 180 && (ourLeader.Pos() - ballPos).mod()<30 || ourLeader.Pos().x()>70){
		needDetourToAvoid = true;
	}
	else{
		needDetourToAvoid = false;
	}
	needDetourToAvoid = true;
	finalDir = (CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0) - ourLeader.Pos()).dir();
	
	double antiFinalDir = Utils::Normalize(finalDir + PI);
	CGeoLine shootLine = CGeoLine(ourLeader.Pos(), finalDir);
	CGeoSegment supportSeg = CGeoSegment(ourSupporter.Pos(), _supportPos);
	CGeoSegment shootSeg = CGeoSegment(ourLeader.Pos(), ourLeader.Pos() + Utils::Polar2Vector(1000, finalDir));
	CGeoLineLineIntersection shootInter = CGeoLineLineIntersection(supportSeg, shootLine);
	CGeoPoint interPoint;
	if (shootInter.Intersectant()){
		interPoint = shootInter.IntersectPoint();
	}
	bool isInter = shootSeg.IsPointOnLineOnSegment(interPoint);
	if (isInter){
	//	cout < "1";
	}
	CGeoPoint projPoint = shootLine.projection(_supportPos);
	CGeoPoint projPoint2 = shootLine.projection(ourSupporter.Pos());
	double projDist = (_supportPos - projPoint).mod();
	if (projDist < PLAYER_SIZE * 5){
		if (shootSeg.IsPointOnLineOnSegment(_supportPos)){
			_supportPos = projPoint + Utils::Polar2Vector(PLAYER_SIZE * 5 + 10, finalDir + PI / 2);
		}
		else{
			changeDir = (projPoint - _supportPos).dir();
			_supportPos = projPoint + Utils::Polar2Vector(PLAYER_SIZE * 5 + 10, changeDir);
		}
	}
	if (needDetourToAvoid && ((_supportPos - ourLeader.Pos()).dir() - finalDir)*((ourSupporter.Pos() - ourLeader.Pos()).dir()-finalDir) < 0
		 &&((shootSeg.IsPointOnLineOnSegment(projPoint2)||!shootSeg.IsPointOnLineOnSegment(projPoint2)&&(projPoint2-ourLeader.Pos()).mod()<PLAYER_SIZE*2)||isInter)){
		if ((ourSupporter.Pos() - ourLeader.Pos()).dir() - finalDir > 0){
			leaderDeviantPos = ourLeader.Pos() + Utils::Polar2Vector(PLAYER_SIZE * 7, finalDir + PI / 2);
		}
		else{
			leaderDeviantPos = ourLeader.Pos() + Utils::Polar2Vector(PLAYER_SIZE * 7, finalDir - PI / 2);
		}
		_supportPos = leaderDeviantPos + Utils::Polar2Vector(PLAYER_SIZE * 15, antiFinalDir);
	}
}

void CSupportPos::keepLeastDistanceFromLeader(const CVisionModule *pVision,double leastDist){
	PlayerVisionT ourSupporter = pVision->OurPlayer(ourSupporterNum);
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	//double changeDir = (ourSupporter.Pos()-ourLeader.Pos()).dir();
	double changeDir = (_supportPos - ball.Pos()).dir();
	if ((_supportPos - ball.Pos()).mod() < leastDist){
		changeDir = (_supportPos - ball.Pos()).dir();
		_supportPos = ball.Pos() + Utils::Polar2Vector(leastDist+20, changeDir);
	}
}


void CSupportPos::checkAvoidCenterLine(const CVisionModule *pVision, double radius,double buffer){
	PlayerVisionT ourSupporter = pVision->OurPlayer(ourSupporterNum);
	PlayerVisionT ourLeader = pVision->OurPlayer(ourLeaderNum);
	const MobileVisionT& ball = pVision->Ball();
	const CGeoPoint ballPos = ball.Pos();
	const CGeoPoint ballRawPos = ball.RawPos();
	const double BallVelDir = Utils::Normalize(ball.Vel().dir());
	const double antiBallVelDir = Utils::Normalize(ball.Vel().dir() + PI);
	const PlayerVisionT& enemyLeader = pVision->TheirPlayer(BestPlayer::Instance()->theirFastestPlayerToBallList().at(0).num);
	const PlayerVisionT& enemyGoalie = pVision->TheirPlayer(BestPlayer::Instance()->getTheirGoalie());
	static int ourLeaderInLeftOrRight = ourLeader.Pos().y() < 0 ? Left : Right;
	if (ourLeader.Pos().y()>buffer){
		ourLeaderInLeftOrRight = Right;
	}
	else if(ourLeader.Pos().y() < -buffer){
		ourLeaderInLeftOrRight = Left;
	}
	if (fabs(ourLeader.Pos().y()) < radius ||ourLeader.Pos().y()*_supportPos.y()>0 ||ourLeader.Pos().y()*ourSupporter.Pos().y()>0||ball.Vel().mod()<50){
		if (ourLeaderInLeftOrRight==Right){
			_supportPos.setY(-radius - buffer);
		}
		else{
			_supportPos.setY(radius + buffer);
		}
	}
}
#include "PenaltyGoalie2012.h"
#include "GDebugEngine.h"
#include <VisionModule.h>
#include "skill/Factory.h"
#include <utils.h>
#include "WorldModel.h"
#include "BestPlayer.h"

namespace{
	const bool debug = true;

	//门柱
	CGeoPoint GOAL_LEFT;
	CGeoPoint GOAL_RIGHT;
	CGeoPoint GOAL_CENTER;
	//防守的极限站位点
	CGeoPoint DEF_LEFT;
	CGeoPoint DEF_RIGHT;
	//防守冲刺极限
	CGeoPoint DEF_LEFT_EX;
	CGeoPoint DEF_RIGHT_EX;

	CGeoPoint penaltyPos;

	const double ROTVEL_LIMIT = 0.03;
}

CPenaltyGoalie2012::CPenaltyGoalie2012()
{
	//门柱
	GOAL_LEFT = CGeoPoint(-Param::Field::PITCH_LENGTH/2,-Param::Field::GOAL_WIDTH/2 + 23);
	GOAL_RIGHT = CGeoPoint(-Param::Field::PITCH_LENGTH/2,Param::Field::GOAL_WIDTH/2 - 23);
	GOAL_CENTER = CGeoPoint(-Param::Field::PITCH_LENGTH/2,0);
	//防守的极限站位点
	DEF_LEFT = CGeoPoint(-Param::Field::PITCH_LENGTH/2 + 10,GOAL_LEFT.y());
	DEF_RIGHT = CGeoPoint(-Param::Field::PITCH_LENGTH/2 + 10,GOAL_RIGHT.y());
	//防守冲刺极限
	DEF_LEFT_EX = CGeoPoint(-Param::Field::PITCH_LENGTH/2 + 10,-Param::Field::GOAL_WIDTH/2 + 9);
	DEF_RIGHT_EX = CGeoPoint(-Param::Field::PITCH_LENGTH/2 + 10,Param::Field::GOAL_WIDTH/2 - 9);
	penaltyPos = CGeoPoint(-250*Param::Field::RATIO,0); // 已经修改为Brazil
	_dirDiff = 0;
	_enemyDir = Param::Math::PI;
	_lastEnemyDir = Param::Math::PI;
	_rushGo = false;
	_rushPoint = DEF_LEFT_EX;
	_lastCycle = 0;
}

void CPenaltyGoalie2012::plan(const CVisionModule* pVision)
{
	static bool isIn = true;
	if (debug)
	{
		if (isIn == true)
		{
			setState(BEGINNING);
			cout<<endl<<"get into PenaltyGoalie2012 !"<<endl;
			isIn = false;
		}
	}

	if ( pVision->Cycle() - _lastCycle > Param::Vision::FRAME_RATE * 0.5 )
	{
		_lastEnemyDir = Param::Math::PI;
		_rushGo = false;
		_dirDiff = 0;
	}
	int robotNum = task().executor;
	const PlayerVisionT& me = pVision->OurPlayer(robotNum);
	const MobileVisionT& ball = pVision->Ball();
	const int enemyNum = BestPlayer::Instance()->getTheirBestPlayer();
	//门将站位基线
	const CGeoLine defendLine = CGeoLine(DEF_LEFT,DEF_RIGHT);
	//待计算的门将站位点
	CGeoPoint taskPoint = CGeoPoint(DEF_LEFT.x(),0);
	if (0 != enemyNum && pVision->TheirPlayer(enemyNum).Valid() && ball.X() > -Param::Field::PITCH_LENGTH/2.0 || true == _rushGo)
	{
		const PlayerVisionT& enemy = pVision->TheirPlayer(enemyNum);
		_enemyDir = enemy.Dir();

		CVector ball2leftPost = GOAL_LEFT - penaltyPos;
		CVector ball2rightPost = GOAL_RIGHT - penaltyPos;
		//判断对手朝向是否在门内
		bool dir_in_ourgoal = (_enemyDir > 0 && _enemyDir > ball2rightPost.dir()) 
			|| (_enemyDir < 0 && _enemyDir < ball2leftPost.dir());
		if (dir_in_ourgoal)
		{
			CGeoLine enemyDirLine = CGeoLine(penaltyPos,_enemyDir);
			CGeoLineLineIntersection linesInter = CGeoLineLineIntersection(defendLine,enemyDirLine);
			if (linesInter.Intersectant())
			{
				taskPoint = linesInter.IntersectPoint();
			}
		}else{
			if (_enemyDir > 0)
			{
				taskPoint = DEF_RIGHT;
			} else {
				taskPoint = DEF_LEFT;
			}
		}
		
		if (false == _rushGo)
		{
			//加速
			double alpha = 0.0;
			double extr_dist = 50;
			double taskPos2meDist = me.Pos().dist(taskPoint);
			if (taskPos2meDist > 10 ) {
				alpha = taskPos2meDist / 50;
				if (alpha > 1.0) {
					alpha = 1.0;
				}
				CGeoPoint fastPoint = taskPoint + Utils::Polar2Vector(100,CVector(taskPoint - me.Pos()).dir());
				taskPoint = fastPoint;
			}
			//计算对手旋转速度
			_dirDiff = Utils::Normalize(_enemyDir - _lastEnemyDir);
			if (fabs(_dirDiff) > ROTVEL_LIMIT && enemy.Pos().dist(ball.Pos()) < 15)
			{
				_rushGo = true;
				if (_dirDiff >= 0)
				{
					_rushPoint = DEF_LEFT_EX;
				} else {
					_rushPoint = DEF_RIGHT_EX;
				}
			}
		} else {
			taskPoint = _rushPoint;
			//加速
			double alpha = 0.0;
			double extr_dist = 90;
			double taskPos2meDist = me.Pos().dist(taskPoint);
			if (taskPos2meDist > 10 ) {
				alpha = taskPos2meDist / 50;
				if (alpha > 1.0) {
					alpha = 1.0;
				}
				CGeoPoint fastPoint = taskPoint + Utils::Polar2Vector(500,CVector(taskPoint - me.Pos()).dir());
				taskPoint = fastPoint;
			}
		}
	}
	//极限限制
	if (me.Y() > DEF_RIGHT_EX.y())
	{
		taskPoint = DEF_RIGHT;
		_rushGo = false;
	} else if (me.Y() < DEF_LEFT_EX.y())
	{
		taskPoint = DEF_LEFT;
		_rushGo = false;
	}
	
	TaskT myTask(task());
	myTask.player.pos = taskPoint;
	myTask.player.angle = CVector(ball.Pos() - me.Pos()).dir();
	myTask.player.max_acceleration = 1000;
	myTask.player.flag |= PlayerStatus::QUICKLY;

	setSubTask(TaskFactoryV2::Instance()->GotoPosition(myTask));

	CStatedTask::plan(pVision);
	_lastEnemyDir = _enemyDir;
	_lastCycle = pVision->Cycle();
}

CPlayerCommand* CPenaltyGoalie2012::execute(const CVisionModule* pVision)
{
	if( subTask() ){
		return subTask()->execute(pVision);
	}
	return NULL;
}
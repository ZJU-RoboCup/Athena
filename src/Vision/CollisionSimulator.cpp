#include "CollisionSimulator.h"
#include <utils.h>
#include <GDebugEngine.h>

void CCollisionSimulator::reset(const CGeoPoint& ballRawPos, const CVector& ballVel)
{
	_ball.SetValid(true);
	_ball.SetRawPos(ballRawPos);
	_ball.SetPos(ballRawPos);
	_ball.SetVel(ballVel);
	_hasCollision = false;
}
void CCollisionSimulator::simulate(PlayerVisionT robot, const double time)
{
	const double timeStep = 0.005; // ģ��ĵ���ʱ��
	double timeSimulated = 0;
	const double boxSize = Param::Field::BALL_SIZE + Param::Vehicle::V2::PLAYER_SIZE;
	const double dribbleBarSize = (Param::Field::BALL_SIZE + Param::Vehicle::V2::PLAYER_SIZE * std::sin(Param::Vehicle::V2::HEAD_ANGLE/2) * 2); // ����װ�õĿ��
	const double dribbleBarDist = Param::Vehicle::V2::PLAYER_SIZE * std::cos(Param::Vehicle::V2::HEAD_ANGLE/2) + Param::Field::BALL_SIZE;
	if( !_hasCollision ){
		while( !_hasCollision && timeSimulated < time - 0.001 ){
			timeSimulated += timeStep;
			robot.SetPos(robot.Pos() + robot.Vel() * timeStep);
			robot.SetDir(robot.Dir() + robot.RotVel() * timeStep);
			_ball.SetPos(_ball.Pos() + _ball.Vel() * timeStep);
			// ���Ƿ���ײ��
			const CVector player2ball(_ball.Pos() - robot.Pos());
			const double ballDist = player2ball.mod();
			if( ballDist < boxSize){
				const double ballAbsDir = player2ball.dir(); // ��ľ��ԽǶ�
				const double ballRelDir = Utils::Normalize( ballAbsDir - robot.Dir()); // �����ԽǶ�
				const double absBallRelDir = std::abs(ballRelDir);
				const double ballVerticalDist = ballDist * std::sin(ballRelDir);
				//ԭ��absBallRelDir < Param::Math::PI*38/180 .ʵ�ʳ��Ŀ���Ƕ�ֻ��15������,�ָ�ΪPI*15/180����,2009.1.14
				if( absBallRelDir < Param::Math::PI*10.1/180 /*&& std::abs(ballVerticalDist)  < dribbleBarSize*/ ){
					if ( ballDist - dribbleBarDist/cos(absBallRelDir) < 0.1 && 
						ballDist - dribbleBarDist/cos(absBallRelDir) > 0 ){
						_hasCollision = true;
						_ball.SetPos(robot.Pos() + Utils::Polar2Vector(dribbleBarDist/cos(absBallRelDir), ballAbsDir));
					} else if ( ballDist - dribbleBarDist/cos(absBallRelDir) < 0 ){
						_hasCollision = true;
						double minRelDir = min(absBallRelDir, Param::Vehicle::V2::DRIBBLE_ANGLE);
						double minAbsDir = robot.Dir() + Utils::Sign(ballRelDir)*minRelDir;
						_ball.SetPos(robot.Pos() + Utils::Polar2Vector(dribbleBarDist/cos(minRelDir), minAbsDir));
					} else {
						_hasCollision = false;
					}
				}else if( absBallRelDir > Param::Math::PI*10/180){
					_ball.SetPos(robot.Pos() + Utils::Polar2Vector(boxSize, ballAbsDir));
				}

				if( _hasCollision){
					_ballRelToRobot = (_ball.Pos() - robot.Pos()).rotate(-robot.Dir());
					//_ball.SetRawPos(robot.RawPos() + (_ball.Pos() - robot.Pos()));
			//		_ball.SetVel(robot.Vel());
				}
			}
		}
	}
	if( _hasCollision){
		const double timeRemnant = time - timeSimulated; // ʣ�µ�ʱ��
		robot.SetPos(robot.Pos() + robot.Vel() * timeRemnant);
		robot.SetDir(robot.Dir() + robot.RotVel() * timeRemnant);
		_ball.SetPos(robot.Pos() + _ballRelToRobot.rotate(robot.Dir()));
	//	_ball.SetVel(robot.Vel());
		_ball.SetValid(true);
	}
}
#include "StopRobot.h"
#include <CommandFactory.h>
#include "skill/Factory.h"
#include <VisionModule.h>
#include <robot_power.h>
#include <RobotCapability.h>

CStopRobotV2::CStopRobotV2()
{
}
CPlayerCommand* CStopRobotV2::execute(const CVisionModule* pVision)
{
	// ����ͣ����
	const int vecNumber = task().executor;
	const PlayerVisionT& self = pVision->OurPlayer(vecNumber);
	// ��û����˵�����
	const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(pVision->Side(), vecNumber);
	const double maxDeccel = 1.0*robotCap->maxAcceleration(self.Vel(), 0);
	const double vecSpeed = self.Vel().mod();

	double decSpeedPerFrame = maxDeccel/Param::Vision::FRAME_RATE;
	if( vecSpeed > decSpeedPerFrame ){
		CVector newVel = (self.Vel() * (vecSpeed - decSpeedPerFrame) / vecSpeed).rotate(-self.Dir());
		return CmdFactory::Instance()->newCommand(CPlayerSpeedV2(task().executor, newVel.x(), newVel.y(), 3, DRIBBLE_DISABLED));
	}
	else
		return CmdFactory::Instance()->newCommand(CPlayerSpeedV2(task().executor, 0, 0, 0, DRIBBLE_DISABLED));
} 

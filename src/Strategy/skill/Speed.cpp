#include "Speed.h"
#include <CommandFactory.h>
#include "VisionModule.h"
#include "robot_power.h"
#include "DribbleStatus.h"

CSpeed::CSpeed()
{

}

void CSpeed::plan(const CVisionModule* pVision)
{

}

CPlayerCommand* CSpeed::execute(const CVisionModule* pVision)
{
	int myNum = task().executor;
	double myDir = pVision->OurPlayer(myNum).Dir();
	double speed_x = task().player.speed_x; // x����ƽ���ٶ�
	double speed_y = task().player.speed_y; // y����ƽ���ٶ�
	double rotSpeed = task().player.rotate_speed; // ת���ٶ�
	double dribblePower = DribbleStatus::Instance()->getDribbleCommand(myNum);
	/*task().player.flag & PlayerStatus::DRIBBLING? DRIBBLE_LOWEST: DRIBBLE_DISABLED*/ // ��������
	CVector globalVel(speed_x, speed_y); // ȫ������ϵ�е��ٶ�ʸ��
	CVector localVel = globalVel.rotate(-myDir);
	return CmdFactory::Instance()->newCommand(CPlayerSpeedV2(myNum, localVel.x(), localVel.y(), rotSpeed, dribblePower));
}
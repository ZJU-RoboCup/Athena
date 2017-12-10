#include "./Message.h"
#include "tinyxml/ParamReader.h"
#include "PlayInterface.h"
#include "game_state.h"

namespace{
	// 注意: 这里对于我方车还是对方车与颜色没有关系,都是默认低位给我方机器人用,所以存到低位去;
	// 至于我方车的队标是什么, 只用在发裁判信息时用来判断该信息是否针对自己的
	bool VERBOSE_MODE = false;			// 输出调试信息
	float ANLGE_CALIBRATION[12]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	COptionModule *_pOption = 0;
}

Message::Message(COptionModule *pOption){
	_pOption = pOption;
}

void Message::message2VisionInfo(CServerInterface::VisualInfo& info)
{
	// ball
	info.ball.x = msgX2InfoX(this->_ballx);
	info.ball.y = msgY2InfoY(this->_bally);
	info.ball.valid = this->_ballFound;
	info.imageBall.valid= this->_CameraID;
	info.imageBall.x = this->_ballImageX;
	info.imageBall.y = this->_ballImageY;
	info.cycle = this->_cycle;

	// player
	//PlayInterface::Instance()->clearRealIndex();

	for (int i = 0; i < Param::Field::MAX_PLAYER*2; ++i ) {
		// 低位(0-5)永远存放我方机器人信息
		if (i < Param::Field::MAX_PLAYER) {
			if (_pOption->MyColor() == TEAM_BLUE) {
				// 没有ENEMY_INVERT, 直接图像信息里我方车信息就是我方车信息, 敌方车就是敌方车
				info.player[i].angle = msgAngle2InfoAngle(RobotRotation[0][i])+ANLGE_CALIBRATION[RobotINDEX[0][i]-1];
				info.player[i].pos.valid = RobotFound[0][i];
				info.player[i].pos.x = msgX2InfoX(RobotPosX[0][i]);
				info.player[i].pos.y = msgY2InfoY(RobotPosY[0][i]);
				info.ourRobotIndex[i] = RobotINDEX[0][i];
				
				if ((int)RobotINDEX[0][i]==13 && info.before_cycle[i]<=300){
					//cout<<"before:"<<info.ourRobotIndexBefore[i]<<endl;
					//cout<<"after:"<<(int)info.ourRobotIndex[i]<<endl;
					info.ourRobotIndex[i] = info.ourRobotIndexBefore[i];
					info.before_cycle[i]++;
					continue;
				}
				info.ourRobotIndexBefore[i] = info.ourRobotIndex[i];
				info.before_cycle[i] = 0;
			//	PlayInterface::Instance()->setRealIndex(i+1,RobotINDEX[0][i]);
			} else {
				// 进行ENEMY_INVERT, 图像信息里给的我方车信息恰是敌方车信息, 敌方车则恰是我方车
				info.player[i].angle = msgAngle2InfoAngle(RobotRotation[1][i])+ANLGE_CALIBRATION[RobotINDEX[1][i]-1];
				info.player[i].pos.valid = RobotFound[1][i];
				info.player[i].pos.x = msgX2InfoX(RobotPosX[1][i]);
				info.player[i].pos.y = msgY2InfoY(RobotPosY[1][i]);
				info.ourRobotIndex[i] = RobotINDEX[1][i];
				if ((int)RobotINDEX[1][i]==13 && info.before_cycle[i]<=300){
					//cout<<"before:"<<info.ourRobotIndexBefore[i]<<endl;
					//cout<<"after:"<<(int)info.ourRobotIndex[i]<<endl;
					info.ourRobotIndex[i] = info.ourRobotIndexBefore[i];
					info.before_cycle[i]++;
					continue;
				}
				info.ourRobotIndexBefore[i] = info.ourRobotIndex[i];
				info.before_cycle[i] = 0;
			//	PlayInterface::Instance()->setRealIndex(i+1, RobotINDEX[1][i]);
			}
		// 高位(6-11)永远存放敌方机器人信息
		} else {										
			if (_pOption->MyColor() == TEAM_BLUE) {
				// 没有invert,直接图像信息里我方车信息就是我方车信息, 敌方车就是敌方车
				info.player[i].angle = msgAngle2InfoAngle(RobotRotation[1][i-Param::Field::MAX_PLAYER]);
				info.player[i].pos.valid = RobotFound[1][i-Param::Field::MAX_PLAYER];
				info.player[i].pos.x = msgX2InfoX(RobotPosX[1][i-Param::Field::MAX_PLAYER]);
				info.player[i].pos.y = msgY2InfoY(RobotPosY[1][i-Param::Field::MAX_PLAYER]);
				info.theirRobotIndex[i-Param::Field::MAX_PLAYER] = RobotINDEX[1][i-Param::Field::MAX_PLAYER];
				if ((int)RobotINDEX[1][i] == 13 && info.before_cycle[i] <= 300) {
					//cout<<"before:"<<info.ourRobotIndexBefore[i]<<endl;
					//cout<<"after:"<<(int)info.ourRobotIndex[i]<<endl;
					info.theirRobotIndex[i] = info.theirRobotIndexBefore[i];
					info.before_cycle[i]++;
					continue;
				}
				info.theirRobotIndexBefore[i] = info.theirRobotIndex[i];
				info.before_cycle[i] = 0;
			}else{
				// 进行ENEMY_INVERT, 图像信息里给的我方车信息恰是敌方车信息, 敌方车则恰是我方车
				info.player[i].angle = msgAngle2InfoAngle(RobotRotation[0][i-Param::Field::MAX_PLAYER]);
				info.player[i].pos.valid = RobotFound[0][i-Param::Field::MAX_PLAYER];
				info.player[i].pos.x = msgX2InfoX(RobotPosX[0][i-Param::Field::MAX_PLAYER]);
				info.player[i].pos.y = msgY2InfoY(RobotPosY[0][i-Param::Field::MAX_PLAYER]);
				info.theirRobotIndex[i-Param::Field::MAX_PLAYER] = RobotINDEX[0][i-Param::Field::MAX_PLAYER];
				if ((int)RobotINDEX[1][i] == 13 && info.before_cycle[i] <= 300) {
					//cout<<"before:"<<info.ourRobotIndexBefore[i]<<endl;
					//cout<<"after:"<<(int)info.ourRobotIndex[i]<<endl;
					info.theirRobotIndex[i] = info.theirRobotIndexBefore[i];
					info.before_cycle[i]++;
					continue;
				}
				info.theirRobotIndexBefore[i] = info.theirRobotIndex[i];
				info.before_cycle[i] = 0;
			}
		}
	}
	return ;
}
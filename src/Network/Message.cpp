#include "./Message.h"
#include "tinyxml/ParamReader.h"
#include "PlayInterface.h"
#include "game_state.h"

namespace{
	// ע��: ��������ҷ������ǶԷ�������ɫû�й�ϵ,����Ĭ�ϵ�λ���ҷ���������,���Դ浽��λȥ;
	// �����ҷ����Ķӱ���ʲô, ֻ���ڷ�������Ϣʱ�����жϸ���Ϣ�Ƿ�����Լ���
	bool VERBOSE_MODE = false;			// ���������Ϣ
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
		// ��λ(0-5)��Զ����ҷ���������Ϣ
		if (i < Param::Field::MAX_PLAYER) {
			if (_pOption->MyColor() == TEAM_BLUE) {
				// û��ENEMY_INVERT, ֱ��ͼ����Ϣ���ҷ�����Ϣ�����ҷ�����Ϣ, �з������ǵз���
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
				// ����ENEMY_INVERT, ͼ����Ϣ������ҷ�����Ϣǡ�ǵз�����Ϣ, �з�����ǡ���ҷ���
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
		// ��λ(6-11)��Զ��ŵз���������Ϣ
		} else {										
			if (_pOption->MyColor() == TEAM_BLUE) {
				// û��invert,ֱ��ͼ����Ϣ���ҷ�����Ϣ�����ҷ�����Ϣ, �з������ǵз���
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
				// ����ENEMY_INVERT, ͼ����Ϣ������ҷ�����Ϣǡ�ǵз�����Ϣ, �з�����ǡ���ҷ���
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
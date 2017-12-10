#include "Network\UDPSocket.h"
#include "RefereeBoxIf.h"
#include <iostream>
#include <playmode.h>
#include <tinyxml/ParamReader.h>
#include "referee.pb.h"

namespace {
	CThreadCreator* threadCreator = 0;
	u_short REFEREE_PORT = 10003;    //新版本
	//u_short REFEREE_PORT = 10001;  //老版本

	struct sCMD_TYPE {
		char cmd;
		unsigned int step;
	};

	struct stGamePacket {
		char cmd;
		unsigned char cmd_counter;
		unsigned char goals_blue;
		unsigned char goals_yellow;
		unsigned short time_remaining;
	};
}

CRefereeBoxInterface::~CRefereeBoxInterface() {
	stop();
}

CRefereeBoxInterface::CRefereeBoxInterface():_playMode(PMNone) {
	{
		DECLARE_PARAM_READER_BEGIN(CRefereeBoxInterface)
		READ_PARAM(REFEREE_PORT)
		DECLARE_PARAM_READER_END
	}
	_UDPPort = new UDPSocket;
	_UDPPort->initWithMultiCast(REFEREE_PORT);
}

CThreadCreator::CallBackReturnType THREAD_CALLBACK CRefereeBoxInterface::listener(CThreadCreator::CallBackParamType refBoxInt) {
	CRefereeBoxInterface* refBoxIf = (CRefereeBoxInterface*) refBoxInt;
	refBoxIf->receivingLoop();
	return 0;
}

void CRefereeBoxInterface::start() {
	_alive = true;
	try {
		threadCreator = new CThreadCreator(listener, this);
		std::cout<< "create referee thread" << std::endl;
	}
	catch(std::exception e) {
		std::cout << "Error: Can't start RefereeBox Interface thread" << std::endl;
	}
}

void CRefereeBoxInterface::stop() {
	_alive = false;
	std::cout << "Delete _udpport" << std::endl;
	if (_UDPPort) {
		delete _UDPPort;
	}
	if (threadCreator) {
		delete threadCreator;
	}
}

void CRefereeBoxInterface::receivingLoop() {
	while( _alive ) {
		// added for Protobuf Protocal(port 10003) 2014/2/27 yys  
		if (!(REFEREE_PORT == 10001 || REFEREE_PORT == 10002)) {
			SSL_Referee ssl_referee;
			char buffer[65536];
			int len = sizeof(buffer);
			int receive_byteCnt = _UDPPort->receiveData(buffer, len);

			if (receive_byteCnt > 0) {
				ssl_referee.ParseFromArray(buffer, len);
				unsigned long long packet_timestamp = ssl_referee.packet_timestamp();
				SSL_Referee_Stage stage = ssl_referee.stage();
				SSL_Referee_Command command = ssl_referee.command();
				unsigned long command_counter = ssl_referee.command_counter();
				unsigned long long command_timestamp = ssl_referee.command_timestamp();
				SSL_Referee_TeamInfo yellow = ssl_referee.yellow();
				SSL_Referee_TeamInfo blue = ssl_referee.blue();
				long long stage_time_left = 0;
				if (ssl_referee.has_stage_time_left())
				{
					stage_time_left = ssl_referee.stage_time_left();
				}
				//std::cout << "packet_timestamp : " << packet_timestamp << std::endl;
				//std::cout << "stage : " << stage << std::endl;
				//std::cout << "stage_time_left : " << stage_time_left / 1000000 << std::endl;
				//std::cout << "command : " << command << std::endl;
				//std::cout << "command_counter : " << command_counter << std::endl;
				//std::cout << "command_timestamp : " << command_timestamp << std::endl;
				//std::cout << "yellow score: " << yellow.score() << std::endl;
				//std::cout << "blue score: " << blue.score() << std::endl;
				char cmd;
				struct stGamePacket pCmd_temp;
				unsigned char cmd_index = 0;

				//switch(stage) {
				//case 0: cmd = 'H'; break; // stage : NORMAL_FIRST_HALF_PRE == cmd : Halt
				//case 1: ; break;          // stage : NORMAL_FIRST_HALF
				//case 2: cmd = 'H'; break; // stage : NORMAL_HALF_TIME == cmd : Halt
				//case 3: cmd = 'H'; break; // stage : NORMAL_SECOND_HALF_PRE == cmd : Halt
				//case 4: ; break;          // stage : NORMAL_SECOND_HALF
				//case 5: cmd = 'H'; break; // stage : EXTRA_TIME_BREAK == cmd : Halt
				//case 6: cmd = 'H'; break; // stage : EXTRA_FIRST_HALF_PRE == cmd : Halt
				//case 7: ; break;          // stage : EXTRA_FIRST_HALF
				//case 8: cmd = 'H'; break; // stage : EXTRA_HALF_TIME == cmd : Halt
				//case 9: cmd = 'H'; break; // stage : EXTRA_SECOND_HALF_PRE == cmd : Halt
				//case 10: ; break;         // stage : EXTRA_SECOND_HALF
				//case 11: cmd = 'H'; break;// stage : PENALTY_SHOOTOUT_BREAK == cmd : Halt
				//case 12: cmd = 'H'; break;// stage : PENALTY_SHOOTOUT == cmd : Halt
				//case 13: cmd = 'H'; break;// stage : POST_GAME == cmd : Halt
				//default: cmd = 'H'; break;
				//}
				switch(command) {
					case 0: cmd = 'H'; break; // Halt
					case 1: cmd = 'S'; break; // Stop
					case 2: cmd = ' '; break; // Normal start (Ready)
					case 3: cmd = 's'; break; // Force start (Start)
					case 4: cmd = 'k'; break; // Kickoff Yellow
					case 5: cmd = 'K'; break; // Kickoff Blue
					case 6: cmd = 'p'; break; // Penalty Yellow
					case 7: cmd = 'P'; break; // Penalty Blue
					case 8: cmd = 'f'; break; // Direct Yellow
					case 9: cmd = 'F'; break; // Direct Blue
					case 10: cmd = 'i'; break; // Indirect Yellow
					case 11: cmd = 'I'; break; // Indirect Blue
					case 12: cmd = 't'; break; // Timeout Yellow
					case 13: cmd = 'T'; break; // Timeout Blue
					case 14: cmd = 'g'; break; // Goal Yellow
					case 15: cmd = 'G'; break; // Goal Blue
					default: cmd = 'H';break;
				}
				pCmd_temp.cmd = cmd;
				pCmd_temp.cmd_counter = command_counter; 
				pCmd_temp.goals_blue = blue.score();
				pCmd_temp.goals_yellow = yellow.score();
				pCmd_temp.time_remaining = unsigned short(stage_time_left / 1000000);

				static unsigned char former_cmd_index = 0;
				cmd_index = pCmd_temp.cmd_counter;
				if (cmd_index != former_cmd_index) {
					former_cmd_index = cmd_index;	// 更新上一次指令得标志值
					PlayMode pmode = PMNone;
					for( int pm = PMStop; pm <= PMIndirectBlue; ++pm ) {
						if( playModePair[pm].ch == cmd ) {
							// 寻找匹配的指令名字
							pmode = playModePair[pm].mode;
							break;
						}
					}
					if( pmode != PMNone ) {
						_playMode = pmode;
						_blueGoalNum = (int)pCmd_temp.goals_blue;
						_yellowGoalNum = (int)pCmd_temp.goals_yellow;
						_blueGoalie = (int)blue.goalie();
						_yellowGoalie = (int)yellow.goalie();
						_remainTime = pCmd_temp.time_remaining;
						std::cout << "Protobuf Protocol: RefereeBox Command : " << cmd << " what : " << playModePair[pmode].what << std::endl;
						std::cout << "Stage_time_left : "<< _remainTime << " Goals for blue : "<< (int)(pCmd_temp.goals_blue)
							      << " Goals for yellow : "<< (int)(pCmd_temp.goals_yellow) << std::endl;
					}
				}
			}
		}
		else if (REFEREE_PORT == 10001 || REFEREE_PORT == 10002) {
			char cmd;
			struct stGamePacket pCmd_temp;
			unsigned char cmd_index = 0;
			int receive_byteCnt = _UDPPort->receiveData((char*)(&pCmd_temp), sizeof(struct stGamePacket));
			if (receive_byteCnt> 0) {
				static unsigned char former_cmd_index = 0;
				cmd = pCmd_temp.cmd;
				cmd_index = pCmd_temp.cmd_counter;
				if (cmd_index != former_cmd_index) {
					former_cmd_index = cmd_index;	// 更新上一次指令得标志值
					PlayMode pmode = PMNone;
					for( int pm = PMStop; pm <= PMIndirectBlue; ++pm ) {
						if( playModePair[pm].ch == cmd ) {
							// 寻找匹配的指令名字
							pmode = playModePair[pm].mode;
							break;
						}
					}
					if( pmode != PMNone ) {
						_playMode = pmode;
						_blueGoalNum = (int)pCmd_temp.goals_blue;
						_yellowGoalNum = (int)pCmd_temp.goals_yellow;
						_remainTime = ntohs(pCmd_temp.time_remaining);
						std::cout << "Legacy Protocol: RefereeBox Command : " << cmd << " what : " << playModePair[pmode].what << std::endl;
						std::cout << "Time remaining : "<< ntohs(pCmd_temp.time_remaining) << " Goals for blue : "<< (int)(pCmd_temp.goals_blue)
							      << " Goals for yellow : "<< (int)(pCmd_temp.goals_yellow) << std::endl;
					}
				}
			}
		}
	}
}
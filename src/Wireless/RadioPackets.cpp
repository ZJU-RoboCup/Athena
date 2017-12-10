#include "RadioPackets.h"
#include <CommControl.h>
#include <crc.h>
#include <stdlib.h>
#include <PlayInterface.h>
#include <CommandSender.h>
#include <vector>
#include <iostream>
#include "stdio.h"
#include "RadioPacketsLog.h"
#include <tinyxml/ParamReader.h>
#include <stdexcept>

#define WITH_MODE_PACKET
#define WITH_TOW_PACKET_MODE_2011
#define WITH_TOW_PACKET_MODE_2012

/////////////////////////////////////////////////////////////////////
namespace {
	ofstream record("D:\\record.txt",ios::app);
	ofstream testpacket("D:\\testpacket.txt");
	CRadioPacketsLog *_radioLog;  // added by yys 2014-04-02
	bool WRITE_RADIO_LOG = false; // 默认不记radio log
}

RadioPacketV2011::RadioPacketV2011() {
	{
		DECLARE_PARAM_READER_BEGIN(General)
			READ_PARAM(WRITE_RADIO_LOG)
		DECLARE_PARAM_READER_END
	}

#ifdef WITH_TOW_PACKET_MODE_2011
	send_packet_len = 24;
	recv_packet_len = 18;
#else
	send_packet_len = 19;
	recv_packet_len = 15;
#endif
	
	TXBuff = new BYTE[send_packet_len+1];
	current_pgknum = 0;

	_radioLog = new CRadioPacketsLog(200);
	if (WRITE_RADIO_LOG) {
		_radioLog->createFile();
		std::cout << "Radio Packets Log create"<< std::endl;
		_radioLog->isLogging = true;
	}
}
RadioPacketV2011::~RadioPacketV2011()
{
	delete [] TXBuff;
	delete _radioLog;

	if ( send_log.is_open() )
		send_log.close();
}
void RadioPacketV2011::encode(const CRobotControl* robotCommand, CSerialPort* pSerial)
{
	///> 1.获取按序排列的小车
	// 获取图像收到的车号
	std::vector<int> num_list;
	num_list.clear();
	for (int i=1; i<=Param::Field::MAX_PLAYER; i++) {
		int vision_num = PlayInterface::Instance()->getRealIndexByNum(i);
		if (vision_num > 0 && vision_num <= 12) {
			num_list.push_back(vision_num);
		}
	}
	// 按车号进行排序
	sort(num_list.begin(), num_list.end());

	///> 2.根据协议模式进行编码及下发
	this->encodeNormal(robotCommand,num_list,pSerial);

	return ;
}
bool RadioPacketV2011::decode(BYTE* RXBuff, CRobotControl* recvPool, int& Num)
{
	static int receivecnt = 0;
	int mode;

/*	cout <<receivecnt<<" Decode On............................."<<endl;
	printf("RXBuff[0] %02X \n",RXBuff[0]);
	for(int i = 0; i < 15; i++){
		printf("%0x ",RXBuff[i]);
	}
	cout<<endl*/;

	if(RXBuff[0] == 0xff){
		receivecnt ++;

		mode = RXBuff[1] & 0x0f;
		if ( mode == 2 ){
			int real_num = RXBuff[2] & 0x0f;
			Num = real_num;
			if ( real_num >= 1 && real_num <= 12 ){
				recvPool[real_num].bInfraredInfo = ((RXBuff[3] & 0x40) > 0);
				recvPool[real_num].kickInfo = 0;
				recvPool[real_num].kickInfo += ((RXBuff[3] & 0x20) >> 5);
				recvPool[real_num].kickInfo += ((RXBuff[3] & 0x10) >> 4);
				recvPool[real_num].bControledInfo = ((RXBuff[3] & 0x08) >> 3);
				recvPool[real_num].changeNum = (RXBuff[4] & 0xff);
				recvPool[real_num].changeCountNum = (RXBuff[5] & 0x0f);


				//static int changecntS[12] = {0};
				//if(recvPool[real_num].changeNum > changecntS[real_num]){
				//	changecntS[real_num]++;
				//}
				//cout<<"RealNum "<<Num<<" PCNum: "<<changecntS[real_num]<<" RobotNum: "<<recvPool[real_num].changeNum<<endl;
				//for(int i = 0; i<15; i++){
				//	printf("%02x ", RXBuff[i]);
				//}
				//std::cout << endl;
				return true;
			}
		}
		else if (mode == 1){
			int real_num = RXBuff[2] & 0x0f;
			Num = real_num;
			if ( real_num >= 1 && real_num <= 12 ){
				//电池电压
				recvPool[real_num].battery = RXBuff[3] & 0xff;
				//电容电压
				recvPool[real_num].capacity = RXBuff[4] & 0xff;
				//红外好坏等
				recvPool[real_num].bInfraredInfo=RXBuff[5] & 0x80;
				//第几次上传
				recvPool[real_num].changeCountNum = RXBuff[6] & 0x0f;
				return true;
			}
		}
	}
	//else if ( mode == MODE_DEBUG_OneCar ){
	//	return decodeSingleTest(RXBuff, recvPool, Num);
	//}

	return false;
}
void RadioPacketV2011::encodeNormal(const CRobotControl* rcmd, const std::vector<int>& robots, CSerialPort* pSerial)
{
	// ！！robots 是按照车号排序的！！
	
#ifdef WITH_TOW_PACKET_MODE_2011
	// 先根据当时场上车的数量决定下发包的个数
	int packet_num = 3;

	memset(TXBuff, 0, send_packet_len+1);
	int robots_size = robots.size();
	if (0 == current_pgknum) {
		if (3 < robots.size()) {
			robots_size = 3;
		}
	} else {
		if (3 < robots.size()) {
			robots_size = robots.size() - 3;
		} else {
			robots_size = 0;
		}
	}

	// 编码下发
	TXBuff[0] = 0xFF;
	for (unsigned int i = 0; i < robots_size; i++){
		int real_num = robots[i+current_pgknum*packet_num]-1;

		// num
		if (real_num > 7) {
			TXBuff[1] += (1 << (real_num-8)) & 0xF;
		} else {
			TXBuff[2] += (1 << (real_num)) & 0xFF;
		}

		// ctrl & shoot
		TXBuff[i*4+3] = (((rcmd[real_num].nKick & SHOOT_CHIP) >> 8) << 6) & 0xFF;
		TXBuff[i*4+3] += ((rcmd[real_num].nControlBall & 0x3 ) << 4);
		TXBuff[i*4+3] += (rcmd[real_num].nControlBall & 0x4) ? 0x80 : 0;
		
		if(rcmd[real_num].stop){
			TXBuff[i*4+3] |= 0x08;
		}

		// gyro
		if(rcmd[real_num].gyro){
			unsigned int gyro_angle = abs(rcmd[real_num].angle*180*64/3.1415926);
			TXBuff[i*4+3] += 0x01;
			TXBuff[21] = (gyro_angle & 0xFF);
			if(TXBuff[21] == 0xFF){
				TXBuff[21] = 0xFE;
			}
			TXBuff[22] = ((gyro_angle >> 8) & 0x7F);
			TXBuff[23] = rcmd[real_num].radius & 0xFF;
			if(TXBuff[23] == 0xFF){
				TXBuff[23] = 0xFE;
			}
		} else{
			TXBuff[i*4+3] += 0x00;
		}

		// vx
		unsigned int vx_value_uint = abs(rcmd[real_num].fX);
		unsigned char vx_value_uchar = abs(rcmd[real_num].fX);
		TXBuff[i*4+4] = (vx_value_uint & 0x7F);
		if (TXBuff[i*4+4] == 0x7F) {
			TXBuff[i*4+4] = 0x7E;
		}
		TXBuff[i*4+4] |= (rcmd[real_num].fX>=0)?0:0x80;
		TXBuff[i*1+15] |= ((vx_value_uint >> 7) & 0x3) << 6;

		// vy 
		unsigned int vy_value_uint = abs(rcmd[real_num].fY);
		unsigned char vy_value_uchar = abs(rcmd[real_num].fY);
		TXBuff[i*4+5] = (vy_value_uint & 0x7F);
		if (TXBuff[i*4+5] == 0x7F) {
			TXBuff[i*4+5] = 0x7E;
		}
		TXBuff[i*4+5] |= (rcmd[real_num].fY>=0)?0:0x80;
		TXBuff[i*1+15] |= ((vy_value_uint >> 7) & 0x3) << 4;

		// w
		unsigned int w_value_uint = abs(rcmd[real_num].fRotate);
		unsigned char w_value_uchar = abs(rcmd[real_num].fRotate);
		TXBuff[i*4+6] = (w_value_uint & 0x7F);
		if (TXBuff[i*4+6] == 0x7F) {
			TXBuff[i*4+6] = 0x7E;
		}
		TXBuff[i*4+6] |= (rcmd[real_num].fRotate>=0)?0:0x80;
		TXBuff[i*1+15] |= ((w_value_uint >> 7) & 0x3) << 2;


		// kick pow
		unsigned int kick_value_uint = abs(rcmd[real_num].nKick*1.0);
		unsigned char kick_value_uchar = abs(rcmd[real_num].nKick*1.0);
		TXBuff[i*1+18] = kick_value_uchar & 0x7F;

		/*if ((rcmd[real_num].nKick & SHOOT_CHIP) == 0x00) {
			if (kick_value_uchar > 91) {
				throw("error91");
			}
		}*/
	}

	//if (WorldModel::Instance()->CurrentRefereeMsg()=="gameStop"){
	//for(int i = 0; i < 25; i++){
	//	printf("%0x ",TXBuff[i]);
	//}
	//cout<<endl;
	//}

	// 记录发送的数据到文件中
	if (_radioLog->isLogging) {
		if (_radioLog->checkSize()) {
			_radioLog->createFile();
			std::cout << "New Radio Packets Log create"<< std::endl;
		}
		_radioLog->writeLog(TXBuff, send_packet_len + 1);
	}

	// Send
	pSerial->WriteBlock(TXBuff, send_packet_len+1);
#else
#endif
}
void RadioPacketV2011::encodePIDWrite(const CRobotControl* rcmd, const int robotID)
{
	// TODO [5/22/2011 cliffyin]
	return ;
}
void RadioPacketV2011::encodeSingleTest(const CRobotControl* rcmd, const int robotID)
{
	// TODO [5/22/2011 cliffyin]
	return ;
}
void RadioPacketV2011::encodeStatusCheck(int robot_num)
{
	// TODO [5/22/2011 cliffyin]
	return ;
}
bool RadioPacketV2011::decodeSingleTest(BYTE* RXBuff, CRobotControl* recvPool, int& Num)
{
	// TODO [5/22/2011 cliffyin]
	return false;
}

RadioPacketV2013::RadioPacketV2013()
{
#ifdef WITH_TOW_PACKET_MODE_2011
	send_packet_len = 24;
	recv_packet_len =17 ;
#else
	send_packet_len = 19;
	recv_packet_len = 15;
#endif

	TXBuff = new BYTE[send_packet_len+1];
	current_pgknum = 0;
}

RadioPacketV2013::~RadioPacketV2013()
{
	delete [] TXBuff;

	if ( send_log.is_open() )
		send_log.close();
}

void RadioPacketV2013::encode(const CRobotControl* robotCommand, CSerialPort* pSerial){
	///> 1.获取按序排列的小车
	// 获取图像收到的车号
	std::vector<int> num_list;
	num_list.clear();
	for (int i=1; i<=Param::Field::MAX_PLAYER; i++) {
		int vision_num = PlayInterface::Instance()->getRealIndexByNum(i);
		if (vision_num > 0 && vision_num <= 12) {
			num_list.push_back(vision_num);
		}
	}
	// 按车号进行排序
	sort(num_list.begin(), num_list.end());

	///> 2.根据协议模式进行编码及下发
	this->encodeNormal(robotCommand,num_list,pSerial);

	return ;
}

bool RadioPacketV2013::decode(BYTE* RXBuff, CRobotControl* recvPool, int& Num){
	//cout <<"Decode On............................."<<endl;
	//for(int i =0;i<17;i++){
	//	printf("//%0x// ",RXBuff[i]);
	//}
	//cout<<endl;
	double rotatespeed[4]={0,0,0,0};
	int dutyratio[4]={0,0,0,0};
	double ratio = 9476.7517630972125;
	static int num =0;
	if(RXBuff[0] == 0xff){
		num++;
		for (int i =0;i<4;i++){
		int rotateflag = (RXBuff[i*4+1] & 0x80) >> 7;
		int t;
		if (rotateflag == 1){
			t=-1;
		}else{
			t=1;
		}
		int Hrotatespeed = (RXBuff[i*4+1] & 0x7F)*256;
		int Lrotatespeed = RXBuff[i*4+2];
		rotatespeed[i] =  t*(Hrotatespeed +Lrotatespeed)/ratio;
		//cout<<rotatespeed[i]<<endl;
		int dutyflag = (RXBuff[i*4+3] & 0X80) >> 7;
		if (dutyflag == 1){
			t=-1;
		}else{
			t=1;
		}
		int Hdutyratio =(RXBuff[i*4+3] & 0x7F)*256;
		int Ldutyratio = RXBuff[i*4+4];
		dutyratio[i] = (Hdutyratio + Ldutyratio)*t;
		//cout<<dutyratio[i]<<endl;
		}
		record<<num<<" speed "<<rotatespeed[0]<<" "<<rotatespeed[1]<<" "<<rotatespeed[2]<<" "<<rotatespeed[3]<<endl;
		record<<num<<"  ratio "<<dutyratio[0]<<" "<<dutyratio[1]<<" "<<dutyratio[2]<<" "<<dutyratio[3]<<endl;
	}
	Num = 1;
	return true;
}

void RadioPacketV2013::encodeNormal(const CRobotControl* rcmd, const std::vector<int>& robots, CSerialPort* pSerial)
{
	// ！！robots 是按照车号排序的！！

#ifdef WITH_TOW_PACKET_MODE_2011
	// 先根据当时场上车的数量决定下发包的个数
	int packet_num = 3;
	static int num = 0;
	memset(TXBuff, 0, send_packet_len+1);
	int robots_size = robots.size();
	if (0 == current_pgknum) {
		if (3 < robots.size()) {
			robots_size = 3;
		}
	} else {
		if (3 < robots.size()) {
			robots_size = robots.size() - 3;
		} else {
			robots_size = 0;
		}
	}

	// 编码下发
	TXBuff[0] = 0xFF;
	num++;
	for (unsigned int i = 0; i < robots_size; i++){
		int real_num = robots[i+current_pgknum*packet_num]-1;
		
		// num
		if (real_num > 7) {
			TXBuff[1] += (1 << (real_num-8)) & 0xF;
		} else {
			TXBuff[2] += (1 << (real_num)) & 0xFF;
		}

		// ctrl & shoot
		TXBuff[i*4+3] = (((rcmd[real_num].nKick & SHOOT_CHIP) >> 8) << 6) & 0xFF;
		TXBuff[i*4+3] += ((rcmd[real_num].nControlBall & 0x3 ) << 4);
		TXBuff[i*4+3] += (rcmd[real_num].nControlBall & 0x4) ? 0x80 : 0;

		if(rcmd[real_num].stop){
			cout<<real_num<<"  SS"<<endl;
			TXBuff[i*4+3] |= 0x08;
		}

		// gyro
		if(rcmd[real_num].gyro){
			unsigned int gyro_angle = abs(rcmd[real_num].angle*180*64/3.1415926);
			TXBuff[i*4+3] += 0x01;
			TXBuff[21] = (gyro_angle & 0xFF);
			if(TXBuff[21] == 0xFF){
				TXBuff[21] = 0xFE;
			}
			TXBuff[22] = ((gyro_angle >> 8) & 0x7F);
			TXBuff[23] = rcmd[real_num].radius & 0xFF;
			if(TXBuff[23] == 0xFF){
				TXBuff[23] = 0xFE;
			}
		} else{
			TXBuff[i*4+3] += 0x00;
		}

		// vx
		unsigned int vx_value_uint = abs(rcmd[real_num].fX);
		unsigned char vx_value_uchar = abs(rcmd[real_num].fX);
		TXBuff[i*4+4] = (vx_value_uint & 0x7F);
		if (TXBuff[i*4+4] == 0x7F) {
			TXBuff[i*4+4] = 0x7E;
		}
		TXBuff[i*4+4] |= (rcmd[real_num].fX>=0)?0:0x80;
		TXBuff[i*1+15] |= ((vx_value_uint >> 7) & 0x3) << 6;

		// vy 
		unsigned int vy_value_uint = abs(rcmd[real_num].fY);
		unsigned char vy_value_uchar = abs(rcmd[real_num].fY);
		TXBuff[i*4+5] = (vy_value_uint & 0x7F);
		if (TXBuff[i*4+5] == 0x7F) {
			TXBuff[i*4+5] = 0x7E;
		}
		TXBuff[i*4+5] |= (rcmd[real_num].fY>=0)?0:0x80;
		TXBuff[i*1+15] |= ((vy_value_uint >> 7) & 0x3) << 4;

		// w
		unsigned int w_value_uint = abs(rcmd[real_num].fRotate);
		unsigned char w_value_uchar = abs(rcmd[real_num].fRotate);
		TXBuff[i*4+6] = (w_value_uint & 0x7F);
		if (TXBuff[i*4+6] == 0x7F) {
			TXBuff[i*4+6] = 0x7E;
		}
		TXBuff[i*4+6] |= (rcmd[real_num].fRotate>=0)?0:0x80;
		TXBuff[i*1+15] |= ((w_value_uint >> 7) & 0x3) << 2;


		// kick pow
		unsigned int kick_value_uint = abs(rcmd[real_num].nKick*1.0);
		unsigned char kick_value_uchar = abs(rcmd[real_num].nKick*1.0);
		TXBuff[i*1+18] = kick_value_uchar & 0x7F;
	}

	testpacket<<num<<" "<<rcmd[4].fX<<" "<<rcmd[4].fY<<" "<<rcmd[4].fRotate<<endl;;
	//if (WorldModel::Instance()->CurrentRefereeMsg()=="gameStop"){
	//for(int i = 0; i < 25; i++){
	//	printf("%0x ",TXBuff[i]);
	//}
	//cout<<endl;
	//}

	// Send
	pSerial->WriteBlock(TXBuff, send_packet_len+1);
#else
#endif
}


RadioPacketV2016::RadioPacketV2016() {
	{
		DECLARE_PARAM_READER_BEGIN(General)
		READ_PARAM(WRITE_RADIO_LOG)
			DECLARE_PARAM_READER_END
	}

	send_packet_len = 24;
	recv_packet_len = 24;

	TXBuff = new BYTE[send_packet_len+1];
	current_pgknum = 0;

	_radioLog = new CRadioPacketsLog(200);
	if (WRITE_RADIO_LOG) {
		_radioLog->createFile();
		std::cout << "Radio Packets Log create"<< std::endl;
		_radioLog->isLogging = true;
	}
}
RadioPacketV2016::~RadioPacketV2016()
{
	delete [] TXBuff;
	delete _radioLog;

	if ( send_log.is_open() )
		send_log.close();
}
void RadioPacketV2016::encode(const CRobotControl* robotCommand, CSerialPort* pSerial)
{
	///> 1.获取按序排列的小车
	// 获取图像收到的车号
	std::vector<int> num_list;
	num_list.clear();
	for (int i=1; i<=Param::Field::MAX_PLAYER; i++) {
		int vision_num = PlayInterface::Instance()->getRealIndexByNum(i);
		if (vision_num > 0 && vision_num <= 12) {
			num_list.push_back(vision_num);
		}
	}
	// 按车号进行排序
	sort(num_list.begin(), num_list.end());

	///> 2.根据协议模式进行编码及下发
	this->encodeNormal(robotCommand,num_list,pSerial);

	return ;
}
bool RadioPacketV2016::decode(BYTE* RXBuff, CRobotControl* recvPool, int& Num)
{
	static int receivecnt = 0;
	int mode;
	if(RXBuff[0] == 0xff){
		receivecnt ++;

		mode = RXBuff[1] & 0x0f;
		if ( mode == 2 ){
			int real_num = RXBuff[2] & 0x0f;
			Num = real_num;
			if ( real_num >= 1 && real_num <= 12 ){
				recvPool[real_num].bInfraredInfo = ((RXBuff[3] & 0x40) > 0);
				recvPool[real_num].kickInfo = 0;
				recvPool[real_num].kickInfo += ((RXBuff[3] & 0x20) >> 5);
				recvPool[real_num].kickInfo += ((RXBuff[3] & 0x10) >> 3);
				recvPool[real_num].bControledInfo = ((RXBuff[3] & 0x08) >> 3);
				recvPool[real_num].changeNum = (RXBuff[4] & 0xff);
				recvPool[real_num].changeCountNum = (RXBuff[5] & 0x0f);


				static int changecntS[12] = {0};
				if(recvPool[real_num].changeNum > changecntS[real_num]){
					changecntS[real_num]++;
				}
				//cout<<"RealNum "<<Num<<" PCNum: "<<changecntS[real_num]<<" RobotNum: "<<recvPool[real_num].changeNum<<endl;
				//for(int i = 0; i<15; i++){
				//	printf("%02x ", RXBuff[i]);
				//}
				//std::cout << endl;
				return true;
			}
		}
		else if (mode == 1){
			int real_num = RXBuff[2] & 0x0f;
			Num = real_num;
			if ( real_num >= 1 && real_num <= 12 ){
				//电池电压
				recvPool[real_num].battery = RXBuff[3] & 0xff;
				//电容电压
				recvPool[real_num].capacity = RXBuff[4] & 0xff;
				//红外好坏等
				recvPool[real_num].bInfraredInfo=RXBuff[5] & 0x80;
				//第几次上传
				recvPool[real_num].changeCountNum = RXBuff[6] & 0x0f;
				return true;
			}
		}
	}
	//else if ( mode == MODE_DEBUG_OneCar ){
	//	return decodeSingleTest(RXBuff, recvPool, Num);
	//}

	return false;
}
void RadioPacketV2016::encodeNormal(const CRobotControl* rcmd, const std::vector<int>& robots, CSerialPort* pSerial)
{
	// ！！robots 是按照车号排序的！！
	
#ifdef WITH_TOW_PACKET_MODE_2011
	// 先根据当时场上车的数量决定下发包的个数
	int packet_num = 3;

	memset(TXBuff, 0, send_packet_len+1);
	int robots_size = robots.size();
	if (0 == current_pgknum) {
		if (3 < robots.size()) {
			robots_size = 3;
		}
	} else {
		if (3 < robots.size()) {
			robots_size = robots.size() - 3;
		} else {
			robots_size = 0;
		}
	}

	// 编码下发
	TXBuff[0] = 0xFF;
	for (unsigned int i = 0; i < robots_size; i++){
		int real_num = robots[i+current_pgknum*packet_num]-1;

		// num
		if (real_num > 7) {
			TXBuff[1] += (1 << (real_num-8)) & 0xF;
		} else {
			TXBuff[2] += (1 << (real_num)) & 0xFF;
		}

		// ctrl & shoot
		TXBuff[i*4+3] = (((rcmd[real_num].nKick & SHOOT_CHIP) >> 8) << 6) & 0xFF;
		TXBuff[i*4+3] += ((rcmd[real_num].nControlBall & 0x3 ) << 4);
		TXBuff[i*4+3] += (rcmd[real_num].nControlBall & 0x4) ? 0x80 : 0;
		
		if(rcmd[real_num].stop){
			TXBuff[i*4+3] |= 0x08;
		}

		// gyro
		if(rcmd[real_num].gyro){
			unsigned int gyro_angle = abs(rcmd[real_num].angle*180*64/3.1415926);
			TXBuff[i*4+3] += 0x01;
			TXBuff[21] = (gyro_angle & 0xFF);
			if(TXBuff[21] == 0xFF){
				TXBuff[21] = 0xFE;
			}
			TXBuff[22] = ((gyro_angle >> 8) & 0x7F);
			TXBuff[23] = rcmd[real_num].radius & 0xFF;
			if(TXBuff[23] == 0xFF){
				TXBuff[23] = 0xFE;
			}
		} else{
			TXBuff[i*4+3] += 0x00;
		}

		// vx
		unsigned int vx_value_uint = abs(rcmd[real_num].fX);
		unsigned char vx_value_uchar = abs(rcmd[real_num].fX);
		TXBuff[i*4+4] = (vx_value_uint & 0x7F);
		if (TXBuff[i*4+4] == 0x7F) {
			TXBuff[i*4+4] = 0x7E;
		}
		TXBuff[i*4+4] |= (rcmd[real_num].fX>=0)?0:0x80;
		TXBuff[i*1+15] |= ((vx_value_uint >> 7) & 0x3) << 6;

		// vy 
		unsigned int vy_value_uint = abs(rcmd[real_num].fY);
		unsigned char vy_value_uchar = abs(rcmd[real_num].fY);
		TXBuff[i*4+5] = (vy_value_uint & 0x7F);
		if (TXBuff[i*4+5] == 0x7F) {
			TXBuff[i*4+5] = 0x7E;
		}
		TXBuff[i*4+5] |= (rcmd[real_num].fY>=0)?0:0x80;
		TXBuff[i*1+15] |= ((vy_value_uint >> 7) & 0x3) << 4;

		// w
		unsigned int w_value_uint = abs(rcmd[real_num].fRotate);
		unsigned char w_value_uchar = abs(rcmd[real_num].fRotate);
		TXBuff[i*4+6] = (w_value_uint & 0x7F);
		if (TXBuff[i*4+6] == 0x7F) {
			TXBuff[i*4+6] = 0x7E;
		}
		TXBuff[i*4+6] |= (rcmd[real_num].fRotate>=0)?0:0x80;
		TXBuff[i*1+15] |= ((w_value_uint >> 7) & 0x3) << 2;


		// kick pow
		unsigned int kick_value_uint = abs(rcmd[real_num].nKick*1.0);
		unsigned char kick_value_uchar = abs(rcmd[real_num].nKick*1.0);
		TXBuff[i*1+18] = kick_value_uchar & 0x7F;

		/*if ((rcmd[real_num].nKick & SHOOT_CHIP) == 0x00) {
			if (kick_value_uchar > 91) {
				throw("error91");
			}
		}*/
	}

	//if (WorldModel::Instance()->CurrentRefereeMsg()=="gameStop"){
	//for(int i = 0; i < 25; i++){
	//	printf("%0x ",TXBuff[i]);
	//}
	//cout<<endl;
	//}

	// 记录发送的数据到文件中
	if (_radioLog->isLogging) {
		if (_radioLog->checkSize()) {
			_radioLog->createFile();
			std::cout << "New Radio Packets Log create"<< std::endl;
		}
		_radioLog->writeLog(TXBuff, send_packet_len + 1);
	}

	// Send
	pSerial->WriteBlock(TXBuff, send_packet_len+1);
#else
#endif
}
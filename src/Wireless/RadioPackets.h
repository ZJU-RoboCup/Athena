#ifndef __WIRELESS_RADIO_PACKETS_H__
#define __WIRELESS_RADIO_PACKETS_H__

#include <wtypes.h>
#include <vector>
#include <fstream>

/* 各种协议的编码与解码 created by qxz 2009.3.5 */

class CRobotControl;
class CSerialPort;
class AbstractRadioPackets{
public:
	virtual void encode(const CRobotControl* robotCommand, CSerialPort* pSerial) = 0;
	virtual bool decode(BYTE* RXBuff, CRobotControl* recvPool, int& Num) = 0;
	virtual void setpkgnum(int pkgNum) {;}
};

class RadioPacketV2011 : public AbstractRadioPackets{
public:
	RadioPacketV2011();
	~RadioPacketV2011();
	virtual void encode(const CRobotControl* robotCommand, CSerialPort* pSerial);
	virtual bool decode(BYTE* RXBuff, CRobotControl* recvPool, int& Num);
	virtual void setpkgnum(int pkgNum) { current_pgknum = pkgNum;}
private:
	void encodeNormal(const CRobotControl* rcmd, const std::vector<int>& robots, CSerialPort* pSerial);
	void encodePIDWrite(const CRobotControl* rcmd, const int robotID);
	void encodeSingleTest(const CRobotControl* rcmd, const int robotID);
	void encodeStatusCheck(int robot_num);

	bool decodeSingleTest(BYTE* RXBuff, CRobotControl* recvPool, int& Num);

	int current_pgknum;
	int send_packet_len;
	int recv_packet_len;
	BYTE* TXBuff;

	std::ofstream send_log;
};

class  RadioPacketV2013 : public AbstractRadioPackets{
public:
	RadioPacketV2013();
	~RadioPacketV2013();
	virtual void encode(const CRobotControl* robotCommand, CSerialPort* pSerial);
	virtual bool decode(BYTE* RXBuff, CRobotControl* recvPool, int& Num);
	virtual void setpkgnum(int pkgNum) { current_pgknum = pkgNum;}
private:
	void encodeNormal(const CRobotControl* rcmd, const std::vector<int>& robots, CSerialPort* pSerial);
	int current_pgknum;
	int send_packet_len;
	int recv_packet_len;
	BYTE* TXBuff;

	std::ofstream send_log;
};

class  RadioPacketV2016 : public AbstractRadioPackets{
public:
	RadioPacketV2016();
	~RadioPacketV2016();
	virtual void encode(const CRobotControl* robotCommand, CSerialPort* pSerial);
	virtual bool decode(BYTE* RXBuff, CRobotControl* recvPool, int& Num);
	virtual void setpkgnum(int pkgNum) { current_pgknum = pkgNum;}
private:
	void encodeNormal(const CRobotControl* rcmd, const std::vector<int>& robots, CSerialPort* pSerial);
	int current_pgknum;
	int send_packet_len;
	int recv_packet_len;
	BYTE* TXBuff;

	std::ofstream send_log;
};
#endif
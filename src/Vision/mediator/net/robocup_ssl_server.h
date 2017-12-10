//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
  \file    robocup_ssl_server.h
  \brief   C++ Interface: robocup_ssl_server
  \author  Stefan Zickler, 2009
*/
//========================================================================
#ifndef ROBOCUP_SSL_SERVER_H
#define ROBOCUP_SSL_SERVER_H
#include "netraw.h"
//#include "UDPSocket.h"
#include <string>
//#include <QMutex>
#include "../proto/cpp/messages_robocup_ssl_detection.pb.h"
#include "../proto/cpp/messages_robocup_ssl_geometry.pb.h"
#include "../proto/cpp/messages_robocup_ssl_wrapper.pb.h"
using namespace std;
/**
	@author Stefan Zickler
*/
class RoboCupSSLServer{
protected:
  Net::UDP mc; // multicast server//delete by magic
	//UDPSocket mc;// add by magic
 // QMutex mutex;
  int _port;
  string _net_address;
  string _net_interface;

public:
    RoboCupSSLServer(int port = 10002,
                     string net_ref_address="224.5.23.2",
                     string net_ref_interface="");

    ~RoboCupSSLServer();
    bool open();
    void close();
    bool send(const SSL_WrapperPacket & packet);
    bool send(const SSL_DetectionFrame & frame);
    bool send(const SSL_GeometryData & geometry);
	bool send(char* buffer,int size);	//added by yph
};

#endif

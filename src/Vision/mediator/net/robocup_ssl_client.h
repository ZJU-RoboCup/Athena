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
  \file    robocup_ssl_client.h
  \brief   C++ Interface: robocup_ssl_client
  \author  Stefan Zickler, 2009
*/
//========================================================================
#ifndef ROBOCUP_SSL_CLIENT_H
#define ROBOCUP_SSL_CLIENT_H

//#include "UDPSocket.h"
#include <string>
//#include <QMutex>
#include "netraw.h"
#include "proto/cpp/messages_robocup_ssl_detection.pb.h"
#include "proto/cpp/messages_robocup_ssl_geometry.pb.h"
#include "proto/cpp/messages_robocup_ssl_wrapper.pb.h"
using namespace std;
/**
	@author Author Name
*/

class RoboCupSSLClient{
protected:
  static const int MaxDataGramSize = 65536;
  char * in_buffer;
  Net::UDP mc;   // multicast client // delete by magic
  //UDPSocket mc;// add by magic
  //QMutex mutex;//ssl-vision 不知道为啥定义这个，后面根本没用到
  int _port;
  string _net_address;
  string _net_interface;
public:
	RoboCupSSLClient();
    RoboCupSSLClient(int port,
					string net_ref_address,
					string net_ref_interface="");

    ~RoboCupSSLClient();
    int open(bool blocking=false);
    void close();
    bool receive(SSL_WrapperPacket & packet);//in google buffer
	void setAddress(string net_ref_address){_net_address = net_ref_address; };
	void setPort(int port){ _port = port; };

};

#endif

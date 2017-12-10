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
  \file    robocup_ssl_client.cpp
  \brief   C++ Implementation: robocup_ssl_client
  \author  Stefan Zickler, 2009
*/
//========================================================================
#include "robocup_ssl_client.h"
//#include <QtGui>
//#include <QtCore>

#pragma comment(lib, "Ws2_32.lib")

RoboCupSSLClient::RoboCupSSLClient()
{
	_net_interface = "";
	in_buffer = new char[65536];
}

RoboCupSSLClient::RoboCupSSLClient(int port,
							 string net_address,
							 string net_interface)
{
  _port=port;
  _net_address=net_address;
  _net_interface=net_interface;
  in_buffer=new char[65536];
}


RoboCupSSLClient::~RoboCupSSLClient()
{
  delete[] in_buffer;
}

void RoboCupSSLClient::close() {
  mc.close();
}

int RoboCupSSLClient::open(bool blocking) {
  close();
  if(!mc.open(_port,true,true,blocking)) {
    return 0;
  }

  Net::Address multiaddr,interfaces;
  int test;
  test=multiaddr.setHost(_net_address.c_str(),_port);
  if(_net_interface.length() > 0){
    interfaces.setHost(_net_interface.c_str(),_port);
  }else{
    interfaces.setAny();
  }

  if(!mc.addMulticast(multiaddr,interfaces)) {
    return 1;
  }

  return 2;
}


bool RoboCupSSLClient::receive(SSL_WrapperPacket & packet) {
  Net::Address src;
  int r=0;
  r = mc.recv(in_buffer,MaxDataGramSize,src);
  if (r>0) {
    fflush(stdout);
    return packet.ParseFromArray(in_buffer,r);
  }
  return false;
}


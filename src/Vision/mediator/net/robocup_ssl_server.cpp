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
  \file    robocup_ssl_server.cpp
  \brief   C++ Implementation: robocup_ssl_server
  \author  Stefan Zickler, 2009
*/
//========================================================================
#include "robocup_ssl_server.h"

RoboCupSSLServer::RoboCupSSLServer(int port,
                     string net_address,
                     string net_interface)
{
  _port=port;
  _net_address=net_address;
  _net_interface=net_interface;

}


RoboCupSSLServer::~RoboCupSSLServer()
{
}

void RoboCupSSLServer::close() {
 
//  mc.close(); //delete by magic
}

bool RoboCupSSLServer::open() {
  close();
  
  if(!mc.open(_port,true,true)) {
    fprintf(stderr,"Unable to open UDP network port: %d\n",_port);
    fflush(stderr);
    return(false);
  }

  Net::Address multiaddr,interfaces;//add by magic.....add a s after interface
  multiaddr.setHost(_net_address.c_str(),_port);
  if(_net_interface.length() > 0){
    interfaces.setHost(_net_interface.c_str(),_port);
  }else{
    interfaces.setAny();
  }

  if(!mc.addMulticast(multiaddr,interfaces)) {
    fprintf(stderr,"Unable to setup UDP multicast\n");
    fflush(stderr);
    return(false);
  }

  return(true);
}

bool RoboCupSSLServer::send(const SSL_WrapperPacket & packet) {
  string buffer;
  packet.SerializeToString(&buffer);
  Net::Address multiaddr;
  multiaddr.setHost(_net_address.c_str(),_port);
  bool result;
  //mutex.lock();//cut by magic
  result=mc.send(buffer.c_str(),buffer.length(),multiaddr);
  //mutex.unlock();// cut by magic
  if (result==false) {
    fprintf(stderr,"Sending UDP datagram failed (maybe too large?). Size was: %zu byte(s)\n",buffer.length());
  }
  return(result);
}

bool RoboCupSSLServer::send(const SSL_DetectionFrame & frame) {
  SSL_WrapperPacket pkt;
  SSL_DetectionFrame * nframe = pkt.mutable_detection();
  nframe->CopyFrom(frame);
  return send(pkt);
}

bool RoboCupSSLServer::send(const SSL_GeometryData & geometry) {
  SSL_WrapperPacket pkt;
  SSL_GeometryData * gdata = pkt.mutable_geometry();
  gdata->CopyFrom(geometry);
  return send(pkt);
}

bool RoboCupSSLServer::send(char* buffer,int size)
{
	Net::Address multiaddr;
	multiaddr.setHost(_net_address.c_str(),_port);
	bool result;
	result=mc.send(buffer,size,multiaddr);
	if (result==false) {
		fprintf(stderr,"Sending UDP datagram failed (maybe too large?). Size was: %zu byte(s)\n",size);
	}
	return(result);
}
#ifndef _INCLUDED_NETRAW_H_
#define _INCLUDED_NETRAW_H_

#pragma once

#include <sys/types.h>
//#include <sys/socket.h>
#include <winsock2.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <inaddr.h>//add by magic
#include <stdio.h>
//#include <iostream>
//#include <vector>
#include <ws2def.h>
#include <string.h>
//#include "common.h" //delete by magic 2010.03.19

//using namespace std;

namespace Net{

//====================================================================//
//  Net::Address: Network address class
//  (C) James Bruce
//====================================================================//

class Address{
  sockaddr addr;
  //socklen_t addr_len;
  int addr_len;
public:
  Address()
    {memset(&addr,0,sizeof(addr)); addr_len=0;}
  Address(const Address &src)
    {copy(src);}
  ~Address()
    {reset();}

  bool setHost(const char *hostname,int port);
  void setAny(int port=0);

  bool operator==(const Address &a) const
    {return(addr_len==a.addr_len && memcmp(&addr,&a.addr,addr_len)==0);}
  void copy(const Address &src)
    {memcpy(&addr,&src.addr,src.addr_len); addr_len=src.addr_len;}
  void reset()
    {memset(&addr,0,sizeof(addr)); addr_len=0;}//将addr结构体清空
  void clear()
    {reset();}

  //in_addr_t getInAddr() const;// for linux ssl-vision 
  ULONG getInAddr() const;  // add by magic
  void print(FILE *out = stdout) const;

  friend class UDP;
};

//====================================================================//
//  Net::UDP: Simple raw UDP messaging
//  (C) James Bruce
//====================================================================//

class UDP {
	SOCKET fd;
	static bool only_once;
public:
  unsigned sent_packets;
  unsigned sent_bytes;
  unsigned recv_packets;
  unsigned recv_bytes;
public:
 // UDP() {fd=-1; close();} //by magic
	UDP() { 
			if(only_once)
				{
					WSAData wsaData;
					WSAStartup(MAKEWORD(2, 2), &wsaData);
					only_once=false;
				}
			fd=-1;close();
			}
  ~UDP() {close();}

  bool open(int port = 0, bool share_port_for_multicasting=false, bool multicast_include_localhost=false, bool blocking=false);
  bool addMulticast(const Address &multiaddr,const Address &interfaces);
  void close();
  bool isOpen() const
    {return(fd >= 0);}

  bool send(const char *data,int length,const Address &dest);
  int  recv(char *data,int length,Address &src);
  bool wait(int timeout_ms = -1) const;
  bool havePendingData() const
    {return(wait(0));}

  int getFd() const
    {return(fd);}
};

}; // namespace Net

#endif

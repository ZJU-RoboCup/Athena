#ifndef _FOLLOW_H_
#define _FOLLOW_H_

#include "message.h"
#include "GlobalData.h"

typedef unsigned char BYTE;

class Follow
{
public:
	Follow();
	virtual ~Follow(){}
	virtual void add(int camera, int num){}
    virtual void cancel(int minAddFrame, int num);
	virtual bool checkSend(){ return false;}
	virtual bool determinesend(int minAddFrame, int minLostFrame, bool isSendFalse,int *exitNum){return false;}
	virtual void initSmsg(){};
    int getFollowNum(int num);
	bool FindInArray(int num);
	double distances(float a0,float b0,float a1, float b1);
	void initFollowArray();

	FollowArray followarray[MaxFollowNum];
};
#endif
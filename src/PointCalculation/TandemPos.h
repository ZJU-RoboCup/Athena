#ifndef _TANDEM_POS_H_
#define _TANDEM_POS_H_

#include <singleton.h>
#include "geometry.h"

class CTandemPos
{
public:
	CTandemPos();
	~CTandemPos();	

   void generatePos();
   enum TandemState{
	   Special = 1,
	   BallMovingtoTheirHalf,
	   BallMovingtoOurHalf,
	   OurChance,
	   TheirChance
   };
   CGeoPoint getTandemPos();
   void analyzeSituation(int teamnum,int oppnnum);
   void planAssistAttack(int teamnum,int oppnum);
   void planBlock(int teamnnum,int oppnum);
   void planAssistGetBall(int teamnum,int oppnum);
private:
	void setState(TandemState stateNum){_stateNum = stateNum;}
	TandemState getState(){return _stateNum;}
	CGeoPoint _tandemPos;
	TandemState _stateNum;
};

typedef NormalSingleton< CTandemPos > TandemPos;

#endif 
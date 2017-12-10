#ifndef _ASSISTPOINT_2017_H_
#define _ASSISTPOINT_2017_H_
#include "geometry.h"
#include "singleton.h"
class CVisionModule;

struct point {
	int x;
	int y;
};

class CAssistPoint {
	private:
		int ZoneEnemy[6];
		int ZoneChangeBuff[6];
		CGeoPoint AssistPoint;
		int BestZoneNum;
		int SupZone;
		int LeaderZone;
		point Zone3Angle[6];
		int _lastCycle;
		int AssistNum;
		int CalZone(CGeoPoint pos);
		int CalBestTriangle(int ballZone, int supZone);
		double CalTriangleInfo(point a, point b, point c, bool isSurface);
		double CalPointValue(CGeoPoint p);//计算进攻点权值
		bool advancePassed;
	public:
		CAssistPoint();
		~CAssistPoint() {}
		
		void setNewPoint(CGeoPoint a) { AssistPoint = a; }
		CGeoPoint getBestAssistPoint(const CVisionModule* pVision,int leaderNum,int assistNum);
		void generateBestPoint(const CVisionModule* pVision,int leadNum);
		int getAssistNum() { return AssistNum; }
		void setPassed(bool flag) { advancePassed = flag; }
		
};


typedef NormalSingleton<CAssistPoint> AssistPoint;
#endif 

#include "AssistPoint.h"
#include "SupportPos.h"

#include "BallSpeedModel.h"
#include "BestPlayer.h"
#include "ChipBallJudge.h"
#include "GDebugEngine.h"
#include "WorldModel.h"
#include "param.h"
#include "utils.h"

#include <cmath>

namespace {
	bool Debug = true;
	double ZoneBound_x = 225;
	double ZoneBound_y1 = 100;
	double ZoneBound_y2 = -100;
}


CAssistPoint::CAssistPoint(){
	_lastCycle = 0;
	AssistNum = 0;
	AssistPoint = CGeoPoint(0,0);
	for (int i = 0; i < 6; i++) 
		ZoneEnemy[i] = 0;
	for (int i = 0; i < 6; i++)
		ZoneChangeBuff[i] = 0;
	for (int i = 0; i < 6; i++)
	{
		Zone3Angle[i].x = (i % 2)*2;
		Zone3Angle[i].y = (i / 2)*2;
	}
}

CGeoPoint CAssistPoint::getBestAssistPoint(const CVisionModule* pVision,int leaderNum,int assistNum ) {
	if (advancePassed == true)
		return AssistPoint;
	AssistNum = assistNum;
	if (pVision->Cycle() == _lastCycle &&_lastCycle != 0) {
		return AssistPoint;
	}
	else {
		_lastCycle = pVision->Cycle();
	}

	generateBestPoint(pVision,leaderNum);
	return AssistPoint;
}

int CAssistPoint::CalZone(CGeoPoint pos) {
	double x = pos.x();
	double y = pos.y();
	if (y < ZoneBound_y2) {//zone 0 1
		if (x > ZoneBound_x) {//0
			return 0;
		}
		else {//1
			return 1;
		}
	}
	else if (y < ZoneBound_y1) {//zone 2 3 
		if (x > ZoneBound_x) {//2
			return 2;
		}
		else {//3
			return 3;
		}
	}
	else {//zone 4 5 
		if (x > ZoneBound_x) {//4
			return 4;
		}
		else {//5
			return 5;
		}
	}
}

void CAssistPoint::generateBestPoint(const CVisionModule* pVision,int leadNum) {
	//getSupportPos
	CGeoPoint supPos = SupportPos::Instance()->getSupPosDirectly();
	//ballPos
	const MobileVisionT& ball = pVision->Ball();

	double CurrentZoneEnemy[6];
	//initialize ZoneEnemy
	for (int i = 0; i < 6; i++)
		CurrentZoneEnemy[i] = 0;

	//update CurrentZoneEnemy
	for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
		const PlayerVisionT& enemy = pVision->TheirPlayer(i);
		if (enemy.Valid() && enemy.Pos().x()>0) {
			CurrentZoneEnemy[CalZone(enemy.Pos())]++;
		}
	}

	//update ZoneEnemy
	for (int i = 0; i <= 5; i++) {
		if (CurrentZoneEnemy[i] != ZoneEnemy[i]) {
			ZoneChangeBuff[i]++;
			if (ZoneChangeBuff[i] >= 5) {
				ZoneEnemy[i] = CurrentZoneEnemy[i];
				ZoneChangeBuff[i] = 0;
			}	
		}
		else {
			ZoneChangeBuff[i] = 0;
		}
	}
	//cout << ZoneEnemy[0]<<" " << ZoneEnemy[1] << " " << ZoneEnemy[2] << " " << ZoneEnemy[3] << " " << ZoneEnemy[4] << " " << ZoneEnemy[5] << " " <<endl;
	//cout << supPos.x()<<"   "<< supPos.y() << endl;
	//
	SupZone = CalZone(supPos);
	LeaderZone = CalZone(pVision->OurPlayer(leadNum).Pos());

	//calculate zone of AssistPoint by using supPos and ZoneEnemy
	
	if (false) {//基本形势
		GDebugEngine::Instance()->gui_debug_line(CGeoPoint(225,450),CGeoPoint(225,-450));
		GDebugEngine::Instance()->gui_debug_line(CGeoPoint(450, 100), CGeoPoint(0, 100));
		GDebugEngine::Instance()->gui_debug_line(CGeoPoint(450, -100), CGeoPoint(0, -100));
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(340, -200), "0");
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -200), "1");
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(340, -0), "2");
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -0), "3");
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(340, 200), "4");
		GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, 200), "5");
	}
/*场地分配
--------------------------------------------------------------
||					||					||					||
||					||					||					||
||		  0			||		  2			||		  4			||
||					||					||					||
||					||					||					||
||----------------------------------------------------------||
||					||					||					||
||					||					||					||
||		  1			||		  3			||		  5			||
||					||					||					||
||					||					||					||
--------------------------------------------------------------
*/

	if (SupZone == LeaderZone) {
		switch (SupZone) {
		case 1:
			BestZoneNum = 4;
			break;
		case 2:
			if (ball.Pos().y() < supPos.y())
				BestZoneNum = 1;
			else
				BestZoneNum = 5;
			break;
		case 3:
			if (ball.Pos().y() < supPos.y())
				BestZoneNum = 0;
			else
				BestZoneNum = 4;
			break;
		case 5:
			BestZoneNum = 0;
			break;
		default:
			BestZoneNum = 3;
			break;
		}
	}
	else if (SupZone == 2 && (LeaderZone == 5 || LeaderZone == 1))//特殊情况站位
		BestZoneNum = LeaderZone == 5 ? 0 : 4;
	else if (LeaderZone == 2 && (SupZone == 5 || SupZone == 1))//特殊情况站位
		BestZoneNum = SupZone == 5 ? 0 : 4;
	else {//普遍情况计算三角形面积
		BestZoneNum = CalBestTriangle(LeaderZone, SupZone);
	}

	//计算bestPoint
	if (Debug) {
		switch (BestZoneNum) {
			case 0:
				//GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(340, -200), "IN THIS ZONE");
				AssistPoint = CGeoPoint(340, -200);
				break;
			case 1:
				//GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -200), "IN THIS ZONE");
				AssistPoint = CGeoPoint(120, -200);
				break;
			case 2:
				//GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(340, -0), "IN THIS ZONE");
				AssistPoint = CGeoPoint(340, -0);
				break;
			case 3:
				//GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, -0), "IN THIS ZONE");
				AssistPoint = CGeoPoint(120, -0);
				break;
			case 4:
				//GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(340, 200), "IN THIS ZONE");
				AssistPoint = CGeoPoint(340, 200);
				break;
			case 5:
				//GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(120, 200), "IN THIS ZONE");
				AssistPoint = CGeoPoint(120, 200);
				break;
		}		
	}

	
	
}

struct indexValue {
	int index;
	double surface;
	double burden;
	double perimeter;//周长
};


bool cmpZoneInfo(indexValue&a, indexValue&b) {
	if (fabs(a.surface - b.surface) < 0.001)
		if (fabs(a.perimeter - b.perimeter) < 0.001)
			return (a.burden < b.burden);
		else
			return (a.perimeter > b.perimeter);
	else
		return (a.surface > b.surface);
		
}
int CAssistPoint::CalBestTriangle(int ballZone, int supZone)
{	
	vector <indexValue> ZoneInfo;
	if ((SupZone == 0 && LeaderZone == 4) || (SupZone == 4 && LeaderZone == 0)) {
		if (ZoneEnemy[3] == 0) {
			//cout << "special situation"<< endl;
			return 3;
		}
	}

	//cout << "Sup:" << supZone << " Ball:" << ballZone << endl;
	for (int i = 0; i < 6; i++) {
		if (ballZone == i || supZone == i)
			continue;
		else{
			indexValue zone;
			zone.index = i;
			zone.surface = CalTriangleInfo(Zone3Angle[ballZone],Zone3Angle[supZone],Zone3Angle[i],true);
			zone.perimeter = CalTriangleInfo(Zone3Angle[ballZone], Zone3Angle[supZone], Zone3Angle[i], false);//比赛临时太急了不想细写了，算了两遍 gty
			//cout << i<<":"<< CalTriangleSurface(Zone3Angle[ballZone], Zone3Angle[supZone], Zone3Angle[i])<<endl;
			zone.burden = ZoneEnemy[i];
			//bool 
			ZoneInfo.push_back(zone);
		}
	}
	
	sort(begin(ZoneInfo), end(ZoneInfo),cmpZoneInfo);

	/*cout << "==============================="<< endl;
	for (int i = 0; i < ZoneInfo.size(); i++) {
		cout << ZoneInfo.at(i).index<< " "<< ZoneInfo.at(i).surface <<" "<<ZoneInfo.at(i).burden<< endl;
	}*/
	return ZoneInfo.at(0).index;
}

double CAssistPoint::CalTriangleInfo(point a, point b, point c, bool isSurface)
{	
	
	if (a.x == b.x && b.x == c.x) {
		return 0;
	}

	double A, B, C;
	A = sqrt(pow(b.x - c.x, 2) + pow(b.y - c.y, 2));
	B = sqrt(pow(a.x - c.x, 2) + pow(a.y - c.y, 2));
	C = sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
	if (isSurface) {
		double P = (A + B + C) / 2;
		//海伦公式
		double S = sqrt(P*(P - A)*(P - B)*(P - C));
		//cout << "SurFace:" << S << endl;
		return S;
	}
	else {
		return (A + B + C);
	}
	
}



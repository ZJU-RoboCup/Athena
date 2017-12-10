#include "FreeKickUtils.h"
#include "gpuBestAlgThread.h"
#include "TaskMediator.h"
#include "Global.h"
#include "WorldModel.h"
#include "VisionModule.h"

namespace {
	bool DEBUG_MODE = true;
	double reflectBoardDir = Param::Math::PI / 2 + Param::Math::PI / 30;
	double toIntersectionPointDist = 30;
	double radius = Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE;
}

namespace FreeKickUtils {

	bool isBeingMarked(const CVisionModule* pVision, int myNum) {
		if (!Utils::PlayerNumValid(myNum)) {
			return true;
		}
		static bool beingMarked  = false;
		const PlayerVisionT me   = pVision->OurPlayer(myNum);
		const MobileVisionT ball = pVision->Ball();
		if (me.Valid()) {
			int theirClosestNum = getTheirClosesRobotToOurRobot(pVision, myNum);
			//cout << "theirClosestNum: " << theirClosestNum << endl;
			//cout << "myNum: " << myNum << endl;
			if (theirClosestNum != 0) {
				bool isOppNear    = isOppNearMe(pVision, myNum, theirClosestNum);
				bool isShootBlock = isShootLineBlockedbyTheirMarkingRobot(pVision, myNum, theirClosestNum);
				bool isPassBlock  = isPassLineBlockedbyTheirMarkingRobot(pVision, myNum, theirClosestNum);
				//cout << "isShootBlock: " << isShootBlock << endl;
				//cout << "isPassBlock: " << isPassBlock << endl;
				//cout << "isOppNear: " << isOppNear << endl;
				beingMarked = isOppNear && (isShootBlock || isPassBlock);
			}
		}
		else {
			beingMarked = false;
		}
		//cout << "shootLineBlocked: " << isShootLineBlocked(pVision, myNum, 120) << endl;
		//cout << "passLineBlocked: " << isPassLineBlocked(pVision, myNum) << endl;
		return beingMarked;
	}

	int getTheirClosesRobotToOurRobot(const CVisionModule* pVision, int myNum) {
		double dist  = 1000;
		int theirNum = 0;
		const PlayerVisionT me = pVision->OurPlayer(myNum);
		const CGeoPoint myPos  = me.Pos();
		for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
			if (pVision->TheirPlayer(i).Valid()) {
				if (pVision->TheirPlayer(i).Pos().dist(myPos) < dist) {
					dist = pVision->TheirPlayer(i).Pos().dist(myPos);
					theirNum = i;
				}
			}
		}
		return theirNum;
	}

	bool isShootLineBlockedbyTheirMarkingRobot(const CVisionModule* pVision, int myNum, int theirNum) {
		if (!Utils::PlayerNumValid(myNum)) {
			return true;
		}
		//判断射门角度有没有被挡住
		//图像信息
		const PlayerVisionT& me   = pVision->OurPlayer(myNum);
		const MobileVisionT& ball = pVision->Ball();
		const PlayerVisionT& opp  = pVision->TheirPlayer(theirNum);
		//是否挡住的判断
		bool shootLineBlocked = false;

		KickDirection::Instance()->GenerateShootDir(myNum, pVision->OurPlayer(myNum).Pos());
		double kickDir = KickDirection::Instance()->getRealKickDir();
		
		CGeoSegment me2theirGoal = CGeoSegment(me.Pos(), me.Pos() + Utils::Polar2Vector(80, kickDir));
		CGeoPoint projectionPoint = me2theirGoal.projection(opp.Pos());
		double opp2LineDist = projectionPoint.dist(opp.Pos());

		shootLineBlocked = opp2LineDist < Param::Vehicle::V2::PLAYER_SIZE * 3 && me2theirGoal.IsPointOnLineOnSegment(projectionPoint);
		return shootLineBlocked;
	}

	bool isPassLineBlockedbyTheirMarkingRobot(const CVisionModule* pVision, int myNum, int theirNum) {
		if (!Utils::PlayerNumValid(myNum)) {
			return true;
		}
		//判断传球角度有没有被挡住
		const PlayerVisionT& me   = pVision->OurPlayer(myNum);
		const MobileVisionT& ball = pVision->Ball();
		const PlayerVisionT& opp  = pVision->TheirPlayer(theirNum);

		bool passLineBlocked = false;

		CGeoSegment ball2me = CGeoSegment(ball.Pos(), me.Pos());
		CGeoPoint projectionPoint = ball2me.projection(opp.Pos());
		double opp2LineDist = projectionPoint.dist(opp.Pos());

		passLineBlocked = opp2LineDist < Param::Vehicle::V2::PLAYER_SIZE * 3 && ball2me.IsPointOnLineOnSegment(projectionPoint);
		return passLineBlocked;
	}

	bool isOppNearMe(const CVisionModule* pVision, int myNum, int theirNum) {
		if (!Utils::PlayerNumValid(myNum)) {
			return true;
		}
		const PlayerVisionT& me  = pVision->OurPlayer(myNum);
		const PlayerVisionT& opp = pVision->TheirPlayer(theirNum);

		bool oppNearMe = false;

		CVector me2opp = me.Pos() - opp.Pos();
		double me2oppDist = me2opp.mod();
		
		oppNearMe = me2oppDist < Param::Vehicle::V2::PLAYER_SIZE * 7;
		return oppNearMe;
	}

	// range表示线段有多长
	bool isShootLineBlocked(const CVisionModule* pVision, int myNum, double range, double allowance) {
		bool shootLineBlocked = false;
		if (!Utils::PlayerNumValid(myNum)) {
			return true;
		}
		const PlayerVisionT& me = pVision->OurPlayer(myNum);
		if (!me.Valid()) {
			shootLineBlocked = true;
			return shootLineBlocked;
		}
		const CGeoPoint myPos = me.Pos();
		KickDirection::Instance()->GenerateShootDir(myNum, pVision->OurPlayer(myNum).Pos());
		double kickDir = KickDirection::Instance()->getRealKickDir();
		CGeoSegment segment = CGeoSegment(myPos, myPos + Utils::Polar2Vector(range, kickDir));
		for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
			if (pVision->TheirPlayer(i).Valid()) {
				if (Utils::InTheirPenaltyArea(pVision->TheirPlayer(i).Pos(),0))
					continue;
				CGeoPoint interPoint = segment.projection(pVision->TheirPlayer(i).Pos());
				if (interPoint.dist(pVision->TheirPlayer(i).Pos()) < Param::Vehicle::V2::PLAYER_SIZE * allowance && segment.IsPointOnLineOnSegment(interPoint)) {
					shootLineBlocked = true;
					break;
				}
			}
		}
		return shootLineBlocked;
	}

	bool isShootLineBlockedFromPoint(const CVisionModule* pVision, CGeoPoint pos, double range) {
		bool shootLineBlocked = false;
		CGeoPoint theirGoal = CGeoPoint(Param::Field::PITCH_LENGTH / 2, 0);
		CVector pos2goal = theirGoal - pos;
		double dir = pos2goal.dir();
		CGeoSegment segment = CGeoSegment(pos, pos + Utils::Polar2Vector(range, dir));
		for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
			if (pVision->TheirPlayer(i).Valid()) {
				CGeoPoint interPoint = segment.projection(pVision->TheirPlayer(i).Pos());
				if (interPoint.dist(pVision->TheirPlayer(i).Pos()) < Param::Vehicle::V2::PLAYER_SIZE * 2.5 && segment.IsPointOnLineOnSegment(interPoint)) {
					shootLineBlocked = true;
					break;
				}
			}
		}
		return shootLineBlocked;
	}

	bool isPassLineBlocked(const CVisionModule* pVision, int myNum, double allowance) {
		if (!Utils::PlayerNumValid(myNum)) {
			return true;
		}
		bool passLineBlocked = false;
		const PlayerVisionT& me = pVision->OurPlayer(myNum);
		if (!me.Valid()) {
			passLineBlocked = true;
			return passLineBlocked;
		}
		const MobileVisionT& ball = pVision->Ball();
		const CGeoPoint mePos = me.Pos();
		const CGeoPoint ballPos = ball.Pos();
		CGeoSegment segment = CGeoSegment(mePos, ballPos);
		for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
			if (pVision->TheirPlayer(i).Valid()) {
				CGeoPoint interPoint = segment.projection(pVision->TheirPlayer(i).Pos());
				if (interPoint.dist(pVision->TheirPlayer(i).Pos()) < Param::Vehicle::V2::PLAYER_SIZE * allowance && segment.IsPointOnLineOnSegment(interPoint)) {
					passLineBlocked = true;
					break;
				}
			}
		}
		return passLineBlocked;
	}

	bool isPassLineBlocked(const CVisionModule* pVision, CGeoPoint pos, double allowance) {
		bool passLineBlocked = false;
		const MobileVisionT& ball = pVision->Ball();
		const CGeoPoint mePos = pos;
		const CGeoPoint ballPos = ball.Pos();
		CGeoSegment segment = CGeoSegment(mePos, ballPos);
		for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
			if (pVision->TheirPlayer(i).Valid()) {
				CGeoPoint interPoint = segment.projection(pVision->TheirPlayer(i).Pos());
				if (interPoint.dist(pVision->TheirPlayer(i).Pos()) < Param::Vehicle::V2::PLAYER_SIZE * allowance && segment.IsPointOnLineOnSegment(interPoint)) {
					passLineBlocked = true;
					break;
				}
			}
		}
		return passLineBlocked;
	}

	bool isLightKickBlocked(const CVisionModule* pVision, int myNum, double dir, double R, double halfAngle){
		if (!Utils::PlayerNumValid(myNum)) {
			return true;
		}
		bool LightKickBlocked = false;
		const PlayerVisionT& me = pVision->OurPlayer(myNum);
		if (!me.Valid()) {
			LightKickBlocked = true;
			return LightKickBlocked;
		}
		const MobileVisionT& ball = pVision->Ball();
		const CGeoPoint mePos = me.Pos();
		const CGeoPoint ballPos = ball.Pos();
		for (int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
			if (pVision->TheirPlayer(i).Valid()) {
				CGeoPoint  PlayerPos = pVision->TheirPlayer(i).Pos();
				CVector ball2PlayerDir = PlayerPos - ball.Pos();
				double Dist = PlayerPos.dist(ball.Pos());
				if (Dist < R && fabs(ball2PlayerDir.dir() - dir) * 180 / Param::Math::PI < halfAngle) {
					LightKickBlocked = true;
					break;
				}
			}
		}
		return LightKickBlocked;
	}

	// 用于反射板
	double drawReflect(const CVisionModule* pVision, int myNum) {
		if (!Utils::PlayerNumValid(myNum)) {
			return 0.0;
		}
		if (myNum == 0) { return 0.0; }
		const PlayerVisionT& me = pVision->OurPlayer(myNum);
		if (!me.Valid()) {
			return 0.0;
		}
		double myDir = pVision->OurRawPlayer(myNum).dir;
		CGeoPoint mePos = me.RawPos();
		CGeoPoint meLeft = mePos + Utils::Polar2Vector(radius, Utils::Normalize(myDir - reflectBoardDir));
		CGeoPoint meRight = mePos + Utils::Polar2Vector(radius, Utils::Normalize(myDir + reflectBoardDir));
		const MobileVisionT& ball = pVision->Ball();
		CGeoPoint ballPos = ball.RawPos();
		CVector me2ball = ball.RawPos() - me.RawPos();
		double me2ballDir = me2ball.dir();
		double me2ballDist = me2ball.mod() - Param::Field::BALL_SIZE;
		double diffDir = Utils::Normalize(me2ballDir - myDir) / Param::Math::PI * 180;
		if (DEBUG_MODE) {
			GDebugEngine::Instance()->gui_debug_line(mePos + Utils::Polar2Vector(100, myDir), mePos, COLOR_CYAN); // 朝向
			GDebugEngine::Instance()->gui_debug_line(mePos + Utils::Polar2Vector(100, Utils::Normalize(myDir - reflectBoardDir)), mePos, COLOR_CYAN); // 左法线
			GDebugEngine::Instance()->gui_debug_line(mePos + Utils::Polar2Vector(100, Utils::Normalize(myDir + reflectBoardDir)), mePos, COLOR_CYAN); // 右法线

			if ( Utils::Normalize(myDir - (ball.RawPos() - mePos).dir()) > 0) { // 打左边反射板
				GDebugEngine::Instance()->gui_debug_line(meLeft, ballPos, COLOR_CYAN);
				double reflectDir = Utils::Normalize( (ballPos - meLeft).dir() - Utils::Polar2Vector(100, Utils::Normalize(myDir - reflectBoardDir)).dir() );
				GDebugEngine::Instance()->gui_debug_line(mePos + Utils::Polar2Vector(500, Utils::Normalize(myDir - reflectBoardDir - reflectDir)), meLeft, COLOR_CYAN);
			}
			else { // 打右边反射板
				GDebugEngine::Instance()->gui_debug_line(meRight, ballPos, COLOR_CYAN);
				double reflectDir = Utils::Normalize( (ballPos - meRight).dir() - Utils::Polar2Vector(100, Utils::Normalize(myDir + reflectBoardDir)).dir() );
				GDebugEngine::Instance()->gui_debug_line(mePos + Utils::Polar2Vector(500, Utils::Normalize(myDir + reflectBoardDir - reflectDir)), meRight, COLOR_CYAN);
			}
		}
		return diffDir;
	}

	const CGeoPoint getLeftReflectPos(const CVisionModule* pVision, int myNum) {
		if (!Utils::PlayerNumValid(myNum)) {
			return CGeoPoint(0, 0);
		}
		if (myNum == 0) { return CGeoPoint(0, 0); }
		const PlayerVisionT& me = pVision->OurPlayer(myNum);
		if (!me.Valid()) {
			return CGeoPoint(0, 0);
		}
		double myDir = pVision->OurRawPlayer(myNum).dir;
		CGeoPoint mePos = me.RawPos();
		CGeoPoint meLeft = mePos + Utils::Polar2Vector(radius, Utils::Normalize(myDir - reflectBoardDir));
		return meLeft;
	}

	const CGeoPoint getRightReflectPos(const CVisionModule* pVision, int myNum) {
		if (!Utils::PlayerNumValid(myNum)) {
			return CGeoPoint(0, 0);
		}
		if (myNum == 0) { return CGeoPoint(0, 0); }
		const PlayerVisionT& me = pVision->OurPlayer(myNum);
		if (!me.Valid()) {
			return CGeoPoint(0, 0);
		}
		double myDir = pVision->OurRawPlayer(myNum).dir;
		CGeoPoint mePos = me.RawPos();
		CGeoPoint meRight = mePos + Utils::Polar2Vector(radius, Utils::Normalize(myDir + reflectBoardDir));
		return meRight;
	}

	const CGeoPoint getReflectPos(const CVisionModule* pVision, int myNum) {
		if (!Utils::PlayerNumValid(myNum)) {
			return CGeoPoint(0, 0);
		}
		if (myNum == 0) { return CGeoPoint(0, 0); }
		const PlayerVisionT& me = pVision->OurPlayer(myNum);
		if (!me.Valid()) {
			return CGeoPoint(0, 0);
		}
		double myDir = pVision->OurRawPlayer(myNum).dir;
		CGeoPoint mePos = me.RawPos();
		const MobileVisionT& ball = pVision->Ball();
		CGeoPoint ballPos = ball.RawPos();
		if ( Utils::Normalize(myDir - (ball.RawPos() - mePos).dir()) > 0) { 
			return getLeftReflectPos(pVision, myNum);
		}
		else {
			return getRightReflectPos(pVision, myNum);
		}
	}

	const CGeoPoint getReflectTouchPos(const CVisionModule* pVision, int myNum, double y) {
		if (!Utils::PlayerNumValid(myNum)) {
			return CGeoPoint(0, 0);
		}
		if (myNum == 0) { return CGeoPoint(0, 0); }
		const PlayerVisionT& me = pVision->OurPlayer(myNum);
		if (!me.Valid()) {
			return CGeoPoint(0, 0);
		}
		double myDir = pVision->OurRawPlayer(myNum).dir;
		CGeoPoint mePos = me.RawPos();
		CGeoPoint meLeft = mePos + Utils::Polar2Vector(radius, Utils::Normalize(myDir - reflectBoardDir));
		CGeoPoint meRight = mePos + Utils::Polar2Vector(radius, Utils::Normalize(myDir + reflectBoardDir));
		const MobileVisionT& ball = pVision->Ball();
		CGeoPoint ballPos = ball.RawPos();
		CVector me2ball = ball.RawPos() - me.RawPos();
		if ( Utils::Normalize(myDir - (ball.RawPos() - mePos).dir()) > 0) { // 打左边反射板
			//GDebugEngine::Instance()->gui_debug_line(meLeft, ballPos, COLOR_CYAN);
			double reflectDir = Utils::Normalize( (ballPos - meLeft).dir() - Utils::Polar2Vector(100, Utils::Normalize(myDir - reflectBoardDir)).dir() );
			//GDebugEngine::Instance()->gui_debug_line(mePos + Utils::Polar2Vector(200, Utils::Normalize(myDir - Param::Math::PI / 2 - reflectDir)), meLeft, COLOR_CYAN);
			CGeoLine reflectLine = CGeoLine(mePos + Utils::Polar2Vector(500, Utils::Normalize(myDir - reflectBoardDir - reflectDir)), meLeft);
			if (ball.Pos().y() > 0) {
				y = -abs(y);
			}
			else {
				y = abs(y);
			}
			CGeoLine touchLine = CGeoLine(CGeoPoint(100, y), CGeoPoint(-100, y));
			CGeoLineLineIntersection intersection = CGeoLineLineIntersection(reflectLine, touchLine);
			CGeoPoint intersectionPoint = intersection.IntersectPoint();
			intersectionPoint.setX(intersectionPoint.x() - toIntersectionPointDist);
			GDebugEngine::Instance()->gui_debug_x(intersectionPoint, COLOR_CYAN);
			return intersectionPoint;
		}
		else { // 打右边反射板
			//GDebugEngine::Instance()->gui_debug_line(meRight, ballPos, COLOR_CYAN);
			double reflectDir = Utils::Normalize( (ballPos - meRight).dir() - Utils::Polar2Vector(100, Utils::Normalize(myDir + reflectBoardDir)).dir() );
			//GDebugEngine::Instance()->gui_debug_line(mePos + Utils::Polar2Vector(200, Utils::Normalize(myDir + Param::Math::PI / 2 - reflectDir)), meRight, COLOR_CYAN);
			CGeoLine reflectLine = CGeoLine(mePos + Utils::Polar2Vector(500, Utils::Normalize(myDir + reflectBoardDir - reflectDir)), meRight);
			if (ball.Pos().y() > 0) {
				y = -abs(y);
			}
			else {
				y = abs(y);
			}
			CGeoLine touchLine = CGeoLine(CGeoPoint(100, y), CGeoPoint(-100, y));
			CGeoLineLineIntersection intersection = CGeoLineLineIntersection(reflectLine, touchLine);
			CGeoPoint intersectionPoint = intersection.IntersectPoint();
			intersectionPoint.setX(intersectionPoint.x() - toIntersectionPointDist);
			GDebugEngine::Instance()->gui_debug_x(intersectionPoint, COLOR_CYAN);
			return intersectionPoint;
		}
	}

}
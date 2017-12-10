#include <VisionModule.h>
#include <RobotCapability.h>
#include <TaskMediator.h>
#include "ShootRangeList.h"
#include "PlayInterface.h"
#include "param.h"	//  [5/16/2011 zhanfei]
namespace{	
	bool IGNORE_OPP_GOALIE = false;
}
CShootRangeList::CShootRangeList(const CVisionModule* pVision, const int player, const CGeoPoint pos)
{
	if( lastCycle != pVision->Cycle() || player != lastPlayer || lastPoint.dist(pos)>5){ // ���¼���
		lastCycle = pVision->Cycle();
		lastPlayer = player;
		lastPoint = pos;

		lastShootList.clear(); // ���
		lastBothKickList.clear();
		lastChipKickList.clear();
		const CGeoPoint theirGoal(Param::Field::PITCH_LENGTH/2, 0);
		
		CGeoPoint startPos;
		if (std::abs(pos.x())>1000){
			startPos = pVision->Ball().Pos();
		}
		else{
			startPos = pos;
		}
		const double startPos2TheirGoalDist = (theirGoal - startPos).mod();
		const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(pVision->Side(), player);
		const double minShootAngleWidth = robotCap->minShootAngleWidth();
		const CGeoPoint leftPost(Param::Field::PITCH_LENGTH/2, -Param::Field::GOAL_WIDTH/2); // ������
		const CGeoPoint rightPost(Param::Field::PITCH_LENGTH/2, Param::Field::GOAL_WIDTH/2); // ������
		const double leftPostAngle = (leftPost - startPos).dir();
		const double rightPostAngle = (rightPost - startPos).dir();
		const double leftPostDist = (leftPost - startPos).mod();
		const double rightPostDist = (rightPost - startPos).mod();
		bool shootAngleIsTooNarrow = false;
		if( std::abs(startPos.y()) > Param::Field::GOAL_WIDTH/2 ){
			// ��������
			//const double minShootAngle = (std::min)(std::abs(leftPostAngle), std::abs(rightPostAngle));
			const double minShootWidth = 2*std::sin(std::abs(leftPostAngle-rightPostAngle)/2)*(std::min)(leftPostDist,rightPostDist); // --- LIUNIAN

			shootAngleIsTooNarrow = minShootWidth < 1.5*robotCap->minShootAngleWidth();
		}
		if( !shootAngleIsTooNarrow ){
			const double minChipKickObstacleDist = 26/*robotCap->minChipKickObstacleDist()*/;
			const double maxChipKickDist = (std::min)(robotCap->maxChipShootDist(), startPos2TheirGoalDist - robotCap->minChipShootExtra(startPos2TheirGoalDist) - Param::Field::MAX_PLAYER_SIZE );
			
			//lastShootList.add(CValueRange(leftPostAngle, rightPostAngle)); // ��ʼ��
			//lastBothKickList.add(CValueRange(leftPostAngle, rightPostAngle));
			lastShootList.add(CValueRange(leftPostAngle, rightPostAngle,leftPostDist, rightPostDist)); // ��ʼ��
			lastBothKickList.add(CValueRange(leftPostAngle, rightPostAngle, leftPostDist, rightPostDist));
			for( int i=1; i<=Param::Field::MAX_PLAYER; ++i ){
				if( pVision->TheirPlayer(i).Valid() && pVision->TheirPlayer(i).X() > startPos.x() ){
					const CGeoPoint& playerPos = pVision->TheirPlayer(i).Pos();

					// �Ը�CMU��������Ҫ������������Ա
					if ( IGNORE_OPP_GOALIE ){
						if ( playerPos.dist(theirGoal) < Param::Field::GOAL_WIDTH/2 + 30 )
							continue;
					}

					const CVector startPos2player = playerPos - startPos;
					const double playerDist = startPos2player.mod() - Param::Field::MAX_PLAYER_SIZE/2;
					const double playerDir = startPos2player.dir();
					const double angleRange = (playerDist <= Param::Field::MAX_PLAYER_SIZE/2 ? Param::Math::PI/2 : std::asin( Param::Field::MAX_PLAYER_SIZE/2 / playerDist)); // ��ס�ĽǶ�
					lastShootList.removeAngleRange(playerDir, angleRange, playerDist);
					if( playerDist < 0.001 || playerDist > minChipKickObstacleDist && playerDist < maxChipKickDist){
						continue; // ����ס���߷�ֹ����Խ��
					}
					lastBothKickList.removeAngleRange(playerDir, angleRange, playerDist);
				}
			}
			//for( int i=1; i<=Param::Field::MAX_PLAYER; ++i ){
			//	if( pVision->OurPlayer(i).Valid() && pVision->OurPlayer(i).X() > startPos.x() && i!=player){
			//		//�Լ���ס�ĵ�Ȼ����
			//		const CGeoPoint& playerPos = pVision->OurPlayer(i).Pos();
			//		const CVector startPos2player = playerPos - startPos;
			//		const double playerDist = startPos2player.mod() - Param::Field::MAX_PLAYER_SIZE/2;
			//		const double playerDir = startPos2player.dir();
			//		const double angleRange = (playerDist <= Param::Field::MAX_PLAYER_SIZE/2 ? Param::Math::PI/2 : std::asin( Param::Field::MAX_PLAYER_SIZE/2 / playerDist)); // ��ס�ĽǶ�
			//		lastShootList.removeAngleRange(playerDir, angleRange, playerDist);
			//		if( playerDist < 0.001 || playerDist > minChipKickObstacleDist && playerDist < maxChipKickDist){
			//			continue; // ����ס���߷�ֹ����Խ��
			//		}
			//		lastBothKickList.removeAngleRange(playerDir, angleRange, playerDist);
			//	}
			//}
		}
			lastChipKickList = lastBothKickList - lastShootList;
		}
}

CShootRangeList::CShootRangeList(const CVisionModule* pVision,const bool defence,const int player,const CGeoPoint pos)
{
	//SR_DEFENCE Ϊ���ش˺�������Ҫ��־
	if (SR_DEFENCE != defence)
	{
		cout<<"call function ERROR in shootRangeList,please check your codes "<<endl;
		return;
	}
	if( lastCycle != pVision->Cycle() || player != lastPlayer || lastPoint.dist(pos)>5){ // ���¼���
		lastCycle = pVision->Cycle();
		lastPlayer = player;
		lastPoint = pos;

		lastShootList.clear(); // ���
		lastBothKickList.clear();
		lastChipKickList.clear();
		const CGeoPoint myGoal(Param::Field::PITCH_LENGTH/2, 0);

		CGeoPoint startPos;
		if (std::abs(pos.x())>1000){
			startPos = CGeoPoint(-1*pVision->Ball().Pos().x(),-1*pVision->Ball().Pos().y());
		}
		else{
			startPos = CGeoPoint(-1*pos.x(),-1*pos.y());
		}
		const double startPos2myGoalDist = (myGoal - startPos).mod();
		const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(pVision->Side(), 1);
		const double minShootAngleWidth = robotCap->minShootAngleWidth();
		const CGeoPoint leftPost(Param::Field::PITCH_LENGTH/2, -Param::Field::GOAL_WIDTH/2); // ������
		const CGeoPoint rightPost(Param::Field::PITCH_LENGTH/2, Param::Field::GOAL_WIDTH/2); // ������
		const double leftPostAngle = (leftPost - startPos).dir();
		const double rightPostAngle = (rightPost - startPos).dir();
		const double leftPostDist = (leftPost - startPos).mod();
		const double rightPostDist = (rightPost - startPos).mod();
		bool shootAngleIsTooNarrow = false;
		if( std::abs(startPos.y()) > Param::Field::GOAL_WIDTH/2 ){
			// ��������
			//const double minShootAngle = (std::min)(std::abs(leftPostAngle), std::abs(rightPostAngle));
			const double minShootWidth = 2*std::sin(std::abs(leftPostAngle-rightPostAngle)/2)*(std::min)(leftPostDist,rightPostDist); // --- LIUNIAN

			shootAngleIsTooNarrow = minShootWidth < 1.5*robotCap->minShootAngleWidth();
		}
		if( !shootAngleIsTooNarrow ){
			const double minChipKickObstacleDist = 26/*robotCap->minChipKickObstacleDist()*/;
			const double maxChipKickDist = (std::min)(robotCap->maxChipShootDist(), startPos2myGoalDist - robotCap->minChipShootExtra(startPos2myGoalDist) - Param::Field::MAX_PLAYER_SIZE );

			//lastShootList.add(CValueRange(leftPostAngle, rightPostAngle)); // ��ʼ��
			//lastBothKickList.add(CValueRange(leftPostAngle, rightPostAngle));
			lastShootList.add(CValueRange(leftPostAngle, rightPostAngle,leftPostDist, rightPostDist)); // ��ʼ��
			lastBothKickList.add(CValueRange(leftPostAngle, rightPostAngle, leftPostDist, rightPostDist));
			for( int i=1; i<=Param::Field::MAX_PLAYER; ++i ){
				if (player == i)//�ų�������Ҫ�ų��ĳ�
				{
					continue;
				}
				if( pVision->OurPlayer(i).Valid() && pVision->OurPlayer(i).X() * -1 > startPos.x() - 10){// -10Ϊ�˷�ֹ��������ʱ�������
					const CGeoPoint& playerPos = CGeoPoint(-1*pVision->OurPlayer(i).X(),-1*pVision->OurPlayer(i).Y());

					const CVector startPos2player = playerPos - startPos;
					const double playerDist = startPos2player.mod() - Param::Field::MAX_PLAYER_SIZE/2;
					const double playerDir = startPos2player.dir();
					const double angleRange = (playerDist <= Param::Field::MAX_PLAYER_SIZE/2 ? Param::Math::PI/2 : std::asin( Param::Field::MAX_PLAYER_SIZE/2 / playerDist)); // ��ס�ĽǶ�
					lastShootList.removeAngleRange(playerDir, angleRange, playerDist);
					if( playerDist < 0.001 || playerDist > minChipKickObstacleDist && playerDist < maxChipKickDist){
						continue; // ����ס���߷�ֹ����Խ��
					}
					lastBothKickList.removeAngleRange(playerDir, angleRange, playerDist);
				}
			}
		}
		lastChipKickList = lastBothKickList - lastShootList;
	}
}

const CValueRangeList& CShootRangeList::get()
{
	return lastBothKickList;
}
const CValueRangeList& CShootRangeList::getShootRange()
{
	return lastShootList;
}

const CValueRangeList& CShootRangeList::getChipKickRange()
{
	return lastChipKickList;
}
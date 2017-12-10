#ifndef _DEFENDUTILS_H_
#define _DEFENDUTILS_H_
#include <geometry.h>
#include <param.h>
#include <string>
#include <misc_types.h>

enum posSide{
	POS_SIDE_LEFT = 0,
	POS_SIDE_RIGHT,
	POS_SIDE_MIDDLE
};

namespace DefendUtils{
	extern CGeoEllipse D_ELLIPSE;
	extern CGeoEllipse D_ELLIPSE1;

	extern int getPenaltyFronter();
	extern bool isPosInOurPenaltyArea(const CGeoPoint& pos);
     /** 
	@brief  ����ĳһ���������ֱ������ϵ����ԭ��ĶԳƵ� */
	extern CGeoPoint reversePoint(const CGeoPoint& p);
	/** 
	@brief  �����ҷ�ĳһ��Ա ��Ŀ������ҷ����ŵ����ŽǶȵ� �谭�� */
	extern double calcBlockAngle(const CGeoPoint& target,const CGeoPoint& player);
	/** 
	@brief  �������Բ�ϵĵ��Ƿ��ڽ��� */
	extern bool leftCirValid(const CGeoPoint& p);
	/** 
	@brief  �����Ұ�Բ�ϵĵ��Ƿ��ڽ��� */
	extern bool rightCirValid(const CGeoPoint& p);
	/** 
	@brief  �ҳ��з����п������ŵĶ�Ա */
	extern int getEnemyShooter();
	/** 
	@brief  ����ʵ�������ߣ����Ҹ��·���Ŀ���ͷ��س��� */
	//����ĵ�3�������Ƿ���Ŀ���R����ֵ�����ã����ĸ�����Ӧ��R�Ƕ�
	extern CGeoLine getDefenceTargetAndLine(CGeoPoint& RdefenceTarget,double& RdefendDir);

	/** 
	@brief  ����Stop״̬�º�����վλ�� */
	extern CGeoPoint calcDefenderPoint(const CGeoPoint Rtarget,const double Rdir,posSide Rside);
	/**
	@brief  ��������Ա��վλ�� */
	//����ĵ��ĸ��������ϴμ�¼������Ավλ����������꣬����������ʱ����Ա���������������򣬸���վλ������ĵ��������Ϊ����Ŀ��ĳ����Rֵ

	//��������������һ�����ķ��ؽǶ�
	extern CGeoLine getSideBackTargetAndLine(CGeoPoint& RSideTarget,double& RSideDir);
	
	//����ģʽ,ģʽ0Ϊ���Ҽ������������㷨��ģʽ1Ϊ�ߺ��������㷨
	extern CGeoPoint calcDefenderPointV2(const CGeoPoint Rtarget,const double Rdir,const posSide Rside,int mode = 0,double ratio=-1.0);

	// mode����0�������Һ����͵������㷨������1����ߺ��������㷨������3�����к����㷨
	extern CGeoPoint calcDefenderPointV3(const CGeoPoint& RTarget, double RDir, posSide RSide, int mode);

	extern CGeoPoint calcGoaliePointV2(const CGeoPoint Rtarget,const double Rdir,const posSide Rside,CGeoPoint laststoredpoint,int mode = 0);

	// mode��������Ա�Ƿ���ǰ��1Ϊһ������ǰ��0Ϊ��ȷ������Ҫ�������λ��
	extern CGeoPoint calcGoaliePointV3(const CGeoPoint& RTarget, double RDir, posSide RSide, const CGeoPoint& lastpoint, int mode);

	extern CGeoPoint calcPenaltyLinePoint(const double dir, const posSide side,double ratio);
	/*ratioȡ0-1*/

  /**
   * @brief �ж����Ƿ���������
   * @return �����ҷ��� ������֮ǰ �������ҷ������򷵻�true
   */
	extern bool isBallShotToTheGoal();

	extern bool BallIsToPenaltyArea();

	extern bool inHalfCourt(CGeoPoint target,double dir,int sideFactor);

	extern bool getBallShooted();

	extern bool getEnemyPass();

	//��ͷ��ĵ�
	extern CGeoPoint getCornerAreaPos();
	//������ĵ�
	extern CGeoPoint getIndirectDefender(double radius ,CGeoPoint leftUp ,CGeoPoint rightDown,int mode);

	//void clearKickOffDefArea();

	extern CGeoPoint getMiddleDefender(double bufferX);

	extern CGeoPoint getDefaultPos(int index);

	extern bool checkInDeadArea();

	extern void changeBasicData();

	extern double calcBalltoOurPenaty();
}
#endif

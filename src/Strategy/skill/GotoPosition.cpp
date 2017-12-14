#include "GotoPosition.h"
#include <utils.h>
#include "skill/Factory.h"
#include <CommandFactory.h>
#include <tinyxml/ParamReader.h>
#include <VisionModule.h>
#include <RobotCapability.h>
#include <sstream>
#include <TaskMediator.h>
#include <ControlModel.h>
#include <robot_power.h>
#include <DribbleStatus.h>
#include "PlayInterface.h"
#include <GDebugEngine.h>
//#include <LeavePenaltyArea.h>
#include "DynamicsSafetySearch.h"
#include "CMmotion.h"
#include <fstream>
/************************************************************************/
/*                                                                      */
/************************************************************************/
namespace{
	/// ���Կ���
	bool DRAW_TARGET = false;
	bool DSS_AVOID = true;

	/// ���ڽ������ζ�������
	const double DIST_REACH_CRITICAL = 2;	// [unit : cm]
	const double SlowFactor = 0.5;
	const double FastFactor = 1.2;

	/// �ײ��˶����Ʋ��� �� Ĭ������ƽ���Ŀ�������
	double MAX_TRANSLATION_SPEED = 400;		// [unit : cm/s]
	double MAX_TRANSLATION_ACC = 600;		// [unit : cm/s2]
	double MAX_ROTATION_SPEED = 5;			// [unit : rad/s]
	double MAX_ROTATION_ACC = 15;			// [unit : rad/s2]
	double TRANSLATION_ACC_LIMIT = 1000;
  double MAX_TRANSLATION_DEC = 650;

	/// ����Աר��
	double MAX_TRANSLATION_SPEED_GOALIE;
	double MAX_TRANSLATION_ACC_GOALIE;
	double MAX_TRANSLATION_DEC_GOALIE;

	/// ����ר��
	double MAX_TRANSLATION_SPEED_BACK;
	double MAX_TRANSLATION_ACC_BACK;
	double MAX_TRANSLATION_DEC_BACK;

	/// �ײ���Ʒ�������
	int TRAJECTORY_METHORD = 1;				// Ĭ��ʹ�� CMU �Ĺ켣����
	int TASK_TARGET_COLOR = COLOR_CYAN;
	ofstream carVisionVel("D://zjunlict//zeus2017ap//bin//play_books//carVisionVel.txt");
}
using namespace Param::Vehicle::V2;

/// ���캯�� �� ������ʼ��
CGotoPositionV2::CGotoPositionV2()
{
	DECLARE_PARAM_READER_BEGIN(CGotoPositionV2)
		// ����Ա��������ƽ������
		READ_PARAM(MAX_TRANSLATION_SPEED_GOALIE)
		READ_PARAM(MAX_TRANSLATION_ACC_GOALIE)
		READ_PARAM(MAX_TRANSLATION_DEC_GOALIE)

		// ������������ƽ������
		READ_PARAM(MAX_TRANSLATION_SPEED_BACK)
		READ_PARAM(MAX_TRANSLATION_ACC_BACK)
		READ_PARAM(MAX_TRANSLATION_DEC_BACK)

		// ��������ƽ������
		READ_PARAM(MAX_TRANSLATION_SPEED)
		READ_PARAM(MAX_TRANSLATION_ACC)
		READ_PARAM(MAX_TRANSLATION_DEC)

		READ_PARAM(MAX_ROTATION_SPEED)
		READ_PARAM(MAX_ROTATION_ACC)
		READ_PARAM(DRAW_TARGET)
		READ_PARAM(DSS_AVOID)
		READ_PARAM(TRAJECTORY_METHORD)
		READ_PARAM(TRANSLATION_ACC_LIMIT)
	DECLARE_PARAM_READER_END
}

/// ����� �� ������ʾ
void CGotoPositionV2::toStream(std::ostream& os) const
{
	os << "Going to " << task().player.pos<<" angle:"<<task().player.angle;
}

/// �滮���
void CGotoPositionV2::plan(const CVisionModule* pVision)
{
	return ;
}

/// ִ�нӿ�
CPlayerCommand* CGotoPositionV2::execute(const CVisionModule* pVision)
{
	/************************************************************************/
	/* �����������                                                         */
	/************************************************************************/
	const int vecNumber = task().executor;
	const PlayerVisionT& self = pVision->OurPlayer(vecNumber);
	const CGeoPoint& vecPos = self.Pos();							// С����λ��
	const double vecDir = self.Dir();								// С��������Ƕ�
	CGeoPoint target = task().player.pos;							// Ŀ���λ��
	const int goalieNum = PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie());
	const bool isGoalie = (vecNumber == goalieNum);
	/*cout<<"goalie= "<<goalieNum<<endl;*/
	//cout << self.RotVel() <<" "<< task().player.rotvel << endl;
	/************************************************************************/
	/* �����Ƿ�Ŀ�������                                                   */
	/************************************************************************/
	if (_isnan(target.x()) || _isnan(target.y())) {
		target = self.Pos();
		cout << "Target Pos is NaN, vecNumber is : " << vecNumber << endl;
	}
	target = Utils::MakeInField(target, -Param::Vehicle::V2::PLAYER_SIZE );
	//vector2f p,q,w;
	//p.x = 3;
	//p.y =4;
	//q.x = 6;
	//q.y = 8;
	//w = p - q;
	//while(1)
	//{
	//	cout << w.angle() / Param::Math::PI * 180<< endl;
	//}
	// �����ҷ��������ж� : ���Ž���������Ա�����ܽ������
	bool isMeInOurPenaltyArea = Utils::InOurPenaltyArea(vecPos, 0/*Param::Field::MAX_PLAYER_SIZE/2*/);
	bool isTargetInOurPenaltyArea = Utils::InOurPenaltyArea(target,0 /*Param::Field::MAX_PLAYER_SIZE/2*/);
	if (! isGoalie) {	// ���Ž���Ա�ڽ������� : �뿪����
		if ( isMeInOurPenaltyArea || isTargetInOurPenaltyArea ) {	
			if (isMeInOurPenaltyArea) {
				LeavePenaltyArea(pVision, vecNumber);
				target = reTarget();
			} else {
				double extra_out_dist = Param::Vehicle::V2::PLAYER_SIZE*2+10;
				while(extra_out_dist < 200) {
					target = Utils::MakeOutOfOurPenaltyArea(task().player.pos,extra_out_dist);

					bool checkOk = true;
					for (int teammate = 1; teammate <= Param::Field::MAX_PLAYER; teammate ++) {
						if (teammate == vecNumber) {
							continue;
						}

						if (pVision->OurPlayer(teammate).Pos().dist(target) < Param::Vehicle::V2::PLAYER_SIZE*4) {
							checkOk = false;
							break;
						}
 					}

					if (checkOk) {
						break;
					}

					extra_out_dist += 2*Param::Vehicle::V2::PLAYER_SIZE;
				}				
			}
		}
	}

	// ���г������ܽ��Է�����
	CGeoPoint vecPos4TheirPenalty = vecPos + Utils::Polar2Vector(self.Vel().mod() / Param::Vision::FRAME_RATE * 5, self.Vel().dir());
	bool isMeInTheirPenaltyArea = Utils::InTheirPenaltyArea(vecPos4TheirPenalty, 0);
	bool isTargetInTheirPenaltyArea = Utils::InTheirPenaltyArea(target, 0);
	if (isMeInTheirPenaltyArea == true) {
		LeaveTheirPenaltyArea(pVision, vecNumber);
		target = reTarget();
	} else if (isTargetInTheirPenaltyArea == true) {
		double extraOutDist = Param::Vehicle::V2::PLAYER_SIZE*2 + 10;
		while (extraOutDist < 200) {
			target = Utils::MakeOutOfTheirPenaltyArea(target, extraOutDist);
			bool checkOk = true;
			for (int teammate = 1; teammate <= Param::Field::MAX_PLAYER; ++teammate) {
				if (teammate != vecNumber) {
					if (pVision->OurPlayer(teammate).Pos().dist(target) < Param::Vehicle::V2::PLAYER_SIZE*4) {
						checkOk = false;
						break;
					}
				}
			}
			if (checkOk == true)
				break;
			extraOutDist += Param::Vehicle::V2::PLAYER_SIZE*2;
		}
	}

	// ��¼��ǰ�Ĺ滮ִ��Ŀ���
	GDebugEngine::Instance()->gui_debug_x(target, TASK_TARGET_COLOR);
	GDebugEngine::Instance()->gui_debug_line(self.Pos(), target, TASK_TARGET_COLOR);
	
	const double targetDir = task().player.angle;
	const CVector player2target = target - vecPos;
	const double dist = player2target.mod();
	const double angleDiff = Utils::Normalize(targetDir - vecDir);
	const double absAngleDiff = std::abs(angleDiff);
	const double vecSpeed = self.Vel().mod();
	const int playerFlag = task().player.flag;
	const bool dribble =  playerFlag & PlayerStatus::DRIBBLING;
	unsigned char dribblePower = 0;
	double moveDiff = Utils::Normalize(player2target.dir() - self.Dir());
	//cout << "max_acceleration : " <<  task().player.max_acceleration << endl;

	/************************************************************************/
	/* ȷ���˶����ܲ��� ȷ��ֻʹ��OmniAuto���ñ�ǩ�еĲ���                       */
	/************************************************************************/
	/// �ж���ô��
	CCommandFactory* pCmdFactory = CmdFactory::Instance();					// ָ��CommandFactoryV2��ָ��
	/// �˶����ܲ��� �� û̫�����壬һ����xml�е�����Ϊ׼ cliffyin �� TODO ��������Լ����Ҫ�Ժ����Կ���
	PlayerCapabilityT capability;
	const CRobotCapability* robotCap = RobotCapFactory::Instance()->getRobotCap(pVision->Side(), vecNumber);
	
	// Traslation ȷ��ƽ���˶�����
	if (vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())) {
		capability.maxSpeed = MAX_TRANSLATION_SPEED_GOALIE;
		capability.maxAccel = MAX_TRANSLATION_ACC_GOALIE;
		capability.maxDec = MAX_TRANSLATION_DEC_GOALIE;
	} else if (TaskMediator::Instance()->leftBack() != 0 && vecNumber == TaskMediator::Instance()->leftBack()
					|| TaskMediator::Instance()->rightBack() != 0 && vecNumber == TaskMediator::Instance()->rightBack()
					|| TaskMediator::Instance()->singleBack() != 0 && vecNumber == TaskMediator::Instance()->singleBack()
					|| TaskMediator::Instance()->sideBack() != 0 && vecNumber == TaskMediator::Instance()->sideBack()) {
		capability.maxSpeed = MAX_TRANSLATION_SPEED_BACK;
		capability.maxAccel = MAX_TRANSLATION_ACC_BACK;
		capability.maxDec = MAX_TRANSLATION_DEC_BACK;
	} else {
		capability.maxSpeed = MAX_TRANSLATION_SPEED;
		capability.maxAccel = MAX_TRANSLATION_ACC;
		capability.maxDec = MAX_TRANSLATION_DEC;
	}
	// Rotation	  ȷ��ת���˶�����
	capability.maxAngularSpeed = MAX_ROTATION_SPEED;
	capability.maxAngularAccel = MAX_ROTATION_ACC;
	capability.maxAngularDec = MAX_ROTATION_ACC;

	if (playerFlag & PlayerStatus::SLOWLY) {
		capability.maxSpeed = 140;
		capability.maxAccel *= SlowFactor;
		capability.maxDec *= SlowFactor;
		capability.maxAngularSpeed *= SlowFactor;
		capability.maxAngularAccel *= SlowFactor;
		capability.maxAngularDec *= SlowFactor;
	}
	if (playerFlag & PlayerStatus::QUICKLY 
		|| vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())) {
			capability.maxSpeed *= FastFactor;
			capability.maxAccel *= FastFactor;
			capability.maxDec *= FastFactor;
			capability.maxAngularSpeed *= FastFactor;
			capability.maxAngularAccel *= FastFactor;
			capability.maxAngularDec *= FastFactor;
	}

	if (playerFlag & PlayerStatus::QUICKLY 
		|| vecNumber == PlayInterface::Instance()->getNumbByRealIndex(TaskMediator::Instance()->goalie())) {
	}

	//if (vision->OurPlayer(vecNumber).Vel().mod() > 100){
	//	capability.maxAccel = -1*vision->OurPlayer(vecNumber).Vel().mod()+750;
	//	if(vision->OurPlayer(vecNumber).Vel().mod() > 300){
	//		capability.maxAccel = 450;
	//		capability.maxDec = 450;
	//	}
	//}
	/*capability.maxAccel = 900;
	capability.maxDec = 300;*/

	if (task().player.max_acceleration > 1) { // 2014-03-26 �޸�, ��Ϊdouble�����ܽ�������ж�
		capability.maxAccel = task().player.max_acceleration > TRANSLATION_ACC_LIMIT ? TRANSLATION_ACC_LIMIT : task().player.max_acceleration;
		capability.maxDec   = capability.maxAccel;
	}
	if (WorldModel::Instance()->CurrentRefereeMsg()=="gameStop") {
		const MobileVisionT ball = pVision->Ball();
		if (ball.Pos().x() < -240 && abs(ball.Pos().y()) > 150 ) {
			capability.maxSpeed = 100;
		}
		else {
			capability.maxSpeed = 150;
		}
	}
	
	/************************************************************************/
	/* ȷ����ĩ״̬ ��� ѡȡ�Ŀ��Ʒ�ʽ�����˶�ָ��                  */
	/************************************************************************/
	/// �趨Ŀ��״̬
	PlayerPoseT final;
	final.SetPos(target);
	final.SetDir((playerFlag & (PlayerStatus::POS_ONLY | PlayerStatus::TURN_AROUND_FRONT) ) ? self.Dir() : task().player.angle);
	final.SetVel(task().player.vel);
	final.SetRotVel(task().player.rotvel);
	/// ���ÿ��Ʒ���
	CControlModel control;		
	float usedtime = target.dist(self.Pos()) / capability.maxSpeed / 1.414;	// ��λ����
	/// ���й켣���ɲ���¼����ִ��ʱ��
	if (playerFlag & PlayerStatus::DO_NOT_STOP) {											// һ�㲻��ִ�� �� cliffyin
		 if (CMU_TRAJ == TRAJECTORY_METHORD) {		//CMU���˶����Ʒ��������Է����ٶȵ���
			final.SetVel(Utils::Polar2Vector(1.0, player2target.dir())*self.Vel().mod());
			control.makeCmTrajectory(self,final,capability);
		} else {
			control.makeFastPath(self, final, capability);
			control.makeZeroFinalVelocityTheta(self, final, capability);
		}
	} else {																						// ����ִ�в��� �� cliffyin
		 int Current_Trajectory_Method = TRAJECTORY_METHORD;
		 if (task().player.is_specify_ctrl_method) {// ָ�����˶����Ʒ�ʽ
			 Current_Trajectory_Method = task().player.specified_ctrl_method;
		 }
		 switch (Current_Trajectory_Method) {
			case CMU_TRAJ:
				control.makeCmTrajectory(self,final,capability);					// CMU �����ٵ���		
				break;
			case ZERO_FINAL:
				control.makeZeroFinalVelocityPath(self, final, capability);			// Bangbang ���ٵ���
				break;
			case ZERO_TRAP:
				control.makeTrapezoidalVelocityPath(self,final,capability);			// ZJUNlict ���ٵ��� : �������⣬�Ȳ���
				break;
			case NONE_TRAP:
				control.makeNoneTrapezoidalVelocityPath(self, final, capability);	// ZJUNlict �����ٵ���
				break;
			default:
				control.makeZeroFinalVelocityPath(self, final, capability);			// Bangbang ���ٵ���
				break;
        }
	}

	const vector< vector<double> >& fullPath = control.getFullPath();
	const double time_factor = 1.5;
	if (! fullPath.empty()) {
		usedtime = fullPath.size() / Param::Vision::FRAME_RATE;
	} else {
		//usedtime = predictedTime(self, target);
	}

	bool fullPathSafe = true;
	const int fullPathCheckIndex = 0.5*Param::Vision::FRAME_RATE;
	if (fullPath.size() > 5) {	// ���й켣���
		for (int i = 0; i < fullPath.size(); i++) {
			if (i >= fullPathCheckIndex) {
				break;
			}

			CGeoPoint realPoint = CGeoPoint(fullPath[i][1]+self.Pos().x(),fullPath[i][2]+self.Pos().y());
			GDebugEngine::Instance()->gui_debug_x(realPoint,COLOR_RED);
			if ((isGoalie == false && Utils::InOurPenaltyArea(realPoint, 0))
					|| Utils::InTheirPenaltyArea(realPoint, 0)) {
				fullPathSafe = false;
				cout << vecNumber << " : unsafe^^^^^^^^^^^^^^^^^^^^^^^" << endl;
				break;
			}
		}
	}


	/************************************************************************/
	/* ���ö�̬����ģ�飨DSS�����Թ켣����ģ���˶�ָ����б������	*/
	/************************************************************************/
	// ��ȡ�켣����ģ����ȫ������ϵ�е��ٶ�ָ��
	CVector globalVel = control.getNextStep().Vel();
	if (fullPathSafe == false) {	// ����������Ž���Ա����������ͣ��
		if (isMeInOurPenaltyArea == false) {
			globalVel = CVector(0.0, 0.0);
		//	control.makeCmTrajectory(self,final,capability);	
			cout << vecNumber << " : make zero^^^^^^^^^^^^^^^^^^^^^^^" << endl;
		}		
	}
	//CUsecTimer t1;
	//t1.start();
	int priority = 0;
	if (DSS_AVOID && ((playerFlag & PlayerStatus::ALLOW_DSS))){
		//cout << "DSS" << endl;
		CVector tempVel = DynamicSafetySearch::Instance()->SafetySearch(vecNumber, globalVel, pVision, priority, target, task().player.flag, usedtime, task().player.max_acceleration);
		if (WorldModel::Instance()->CurrentRefereeMsg()=="gameStop" && tempVel.mod() > 150) { // ���������stop��ʱ�򳵿��ܻ���ȥ
			//cout << tempVel.mod() << endl;
		}
		else {
			globalVel = tempVel;
		}
	}
	//t1.stop();
	//cout << t1.time() << endl;

	/************************************************************************/
	/* ��������ָ��                                                       */
	/************************************************************************/
	// ����ϵ�����������ڽ������ζ������� [7/2/2011 cliffyin]
	double alpha = 1.0;
	if (dist <= DIST_REACH_CRITICAL) {
		alpha *= sqrt(dist/DIST_REACH_CRITICAL);
	}


	// ����ת�� : ȫ��ת�ֲ�, �õ���Ҫ�·���С������ϵ�ٶ� <vx,vy,w>
	//cout << "global:"<<globalVel.x()<<" "<<globalVel.y()<< endl;
	CVector localVel = (globalVel*alpha).rotate(-self.Dir());		// ���Լ�����ϵ������ٶ�
	double rotVel =  control.getNextStep().RotVel();				// ��ת�ٶ�
	//if (vecNumber == 2) {
//		cout << localVel.x() << " " << localVel.y() << endl;
//		cout << rotVel << endl;
	//}
	/// add by cliffyin : angular control bug fix
	if ((fabs(Utils::Normalize(final.Dir() - self.Dir())) <= Param::Math::PI*5/180)) {
		//��ǰ�߻�С���������ú���������
		CControlModel cmu_control;
		//cmu_control.makeCmTrajectory(self,final,capability);
		rotVel = cmu_control.getNextStep().RotVel();
	}
	
	// ����
	unsigned char set_power = DribbleStatus::Instance()->getDribbleCommand(vecNumber);

	if (set_power > 0) {
		dribblePower = set_power;
	} else {
		dribblePower = DRIBBLE_DISABLED;
	}

	if (dribble) {
		dribblePower = DRIBBLE_NORAML;
	}
	
	static PlayerVisionT meLast = self;
	carVisionVel << (self.RawPos().x() - meLast.RawPos().x()) * Param::Vision::FRAME_RATE << " " << (self.RawPos().y() - meLast.RawPos().y()) * Param::Vision::FRAME_RATE << endl;
	meLast = self;
	/// ���ɲ����ؿ���ָ��
	return pCmdFactory->newCommand(CPlayerSpeedV2(vecNumber, localVel.x(), localVel.y(), rotVel, dribblePower));
}

void CGotoPositionV2::LeavePenaltyArea(const CVisionModule* pVision, const int player)
{
	const CGeoPoint& vecPos = pVision->OurPlayer(player).Pos();
	const double keepDistance = Param::Field::MAX_PLAYER_SIZE + 10;
	if( Utils::InOurPenaltyArea(vecPos, Param::Field::MAX_PLAYER_SIZE) ){
		// ���ҷ���������,�ڽ���������һЩ�㣬�Ҿ�������ĵ���ס��·��
		const CGeoPoint ourGoal(-Param::Field::PITCH_LENGTH/2, 0);
		const CVector goal2player(vecPos - ourGoal);
		const double goal2playerDir = goal2player.dir();
		CGeoPoint leaveTarget;
		if( Param::Rule::Version == 2003 ){
			leaveTarget = ourGoal +  CVector(Param::Field::PITCH_MARGIN + Param::Field::PENALTY_AREA_DEPTH + keepDistance, vecPos.y());
		}else if (Param::Rule::Version == 2004){
			leaveTarget = ourGoal + Utils::Polar2Vector(Param::Field::PENALTY_AREA_WIDTH/2 + keepDistance, goal2playerDir);
		}else if (Param::Rule::Version == 2008){
			leaveTarget = Utils::GetOutSidePenaltyPos(goal2playerDir,keepDistance,ourGoal);
		}
		if( canGoto(pVision, player,leaveTarget) ){
			return;
		}
		const double angleStep = Param::Math::PI/12 * Utils::Sign(vecPos.y());
		const double distStep = 15 * Utils::Sign(vecPos.y());
		for( int i=1; i<3; ++i ){
			if( Param::Rule::Version == 2003 ){
				leaveTarget = ourGoal + CVector(Param::Field::PITCH_MARGIN + Param::Field::PENALTY_AREA_DEPTH + keepDistance, vecPos.y() + i * distStep);
			}else if(Param::Rule::Version == 2004){
				leaveTarget = ourGoal + Utils::Polar2Vector(Param::Field::PENALTY_AREA_WIDTH/2 + keepDistance, goal2playerDir + i * angleStep);
			}else if (Param::Rule::Version == 2008){
				leaveTarget = Utils::GetOutSidePenaltyPos(goal2playerDir + i * angleStep,keepDistance,ourGoal);
			}
			if( canGoto(pVision, player, leaveTarget) ){
				return;
			}
			
			if( Param::Rule::Version == 2003 ){
				leaveTarget = ourGoal + CVector(Param::Field::PITCH_MARGIN + Param::Field::PENALTY_AREA_DEPTH + keepDistance, vecPos.y() - i * distStep);
			}else if(Param::Rule::Version == 2004){
				leaveTarget = ourGoal + Utils::Polar2Vector(Param::Field::PENALTY_AREA_WIDTH/2 + keepDistance, goal2playerDir - i * angleStep);
			}else if (Param::Rule::Version == 2008){
				leaveTarget = Utils::GetOutSidePenaltyPos(goal2playerDir - i * angleStep,keepDistance,ourGoal);
			}
			if( canGoto(pVision, player, leaveTarget) ){
				return;
			}
		}
		if( Param::Rule::Version == 2003 ){
			leaveTarget = ourGoal + CVector(Param::Field::PITCH_MARGIN + Param::Field::PENALTY_AREA_DEPTH + keepDistance, vecPos.y());
		}else if(Param::Rule::Version == 2004){
			_target = ourGoal + Utils::Polar2Vector(Param::Field::PENALTY_AREA_WIDTH/2 + keepDistance, goal2playerDir); // ֻ����ǰ��
		}else if(Param::Rule::Version == 2008){
			_target = Utils::GetOutSidePenaltyPos(goal2playerDir,keepDistance,ourGoal);
		}
	}else{
		_target = vecPos; // ����
	}
}

void CGotoPositionV2::LeaveTheirPenaltyArea(const CVisionModule* pVision, const int player) {
	const CGeoPoint& vecPos = pVision->OurPlayer(player).Pos();
	// �ڶԷ���������,�ڽ���������һЩ�㣬�Ҿ�������ĵ���ס��·��
	CGeoPoint theirGoal(Param::Field::PITCH_LENGTH/2, 0);
	CVector goal2player(vecPos - theirGoal);
	double goal2playerDir = goal2player.dir();
	double angleStep = Param::Math::PI / 12 * Utils::Sign(vecPos.y());
	double distStep = 15 * Utils::Sign(vecPos.y());
	double keepDistance = Param::Field::MAX_PLAYER_SIZE*1.5;

	for (int i = 0;  i < 3; ++i) {
		CGeoPoint leaveTarget = Utils::GetOutTheirSidePenaltyPos(goal2playerDir + i*angleStep, keepDistance, theirGoal);
		if (Utils::canGo(pVision, player, leaveTarget, 0, 0) == true
				&& leaveTarget.x() > -Param::Field::PITCH_LENGTH/2
				&& leaveTarget.x() < Param::Field::PITCH_LENGTH/2) {
			_target = leaveTarget;
			return;
		}
		leaveTarget = Utils::GetOutTheirSidePenaltyPos(goal2playerDir - i*angleStep, keepDistance, theirGoal);
		if (Utils::canGo(pVision, player, leaveTarget, 0, 0) == true
				&& leaveTarget.x() > -Param::Field::PITCH_LENGTH/2
				&& leaveTarget.x() < Param::Field::PITCH_LENGTH/2) {
			_target = leaveTarget;
			return;
		}
	}
	_target = Utils::GetOutTheirSidePenaltyPos(goal2playerDir, keepDistance, theirGoal);
}

bool CGotoPositionV2::canGoto(const CVisionModule* pVision, const int player, const CGeoPoint& target)
{
	if( target.x() < -Param::Field::PITCH_LENGTH/2 || target.x() > Param::Field::PITCH_LENGTH/2){
		return false;
	}
	bool _canGo = true;
	_canGo = Utils::canGo(pVision, player, target, 0, 0);
	if( _canGo ){
		_target = target;
		return true;
	}
	return false;
}

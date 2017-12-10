#include "Global.h"

CVisionModule*   vision;
CKickStatus*    kickStatus;
CDribbleStatus* dribbleStatus;
CGDebugEngine*  debugEngine;
CWorldModel*	world;
CKickDirection* kickDirection;
CGPUBestAlgThread* bestAlg;
CDefPos2015* defPos2015;
CIndirectDefender* indirectDefender;
CTandemPos* tandemPos;
CBestPlayer* bestPlayer;
CDefenceInfo* defenceInfo;
CChipBallJudge* chipBallJudge;
CHttpServer* httpServer;
CSupportPos* supportPos;
CAssistPoint* assistPoint;
CGetLuaData* getluadata;

void initializeSingleton()
{
	vision        = VisionModule::Instance();
	kickStatus    = KickStatus::Instance();
	dribbleStatus = DribbleStatus::Instance();
	debugEngine   = GDebugEngine::Instance();
	world		  = WorldModel::Instance();
	kickDirection = KickDirection::Instance();
	bestAlg		  = GPUBestAlgThread::Instance();
	supportPos = SupportPos::Instance();
	defPos2015 = DefPos2015::Instance();
	bestPlayer    = BestPlayer::Instance();
	defenceInfo = DefenceInfo::Instance();
	tandemPos = TandemPos::Instance();
	chipBallJudge = ChipBallJudge::Instance();
	httpServer = HttpServer::Instance();
	indirectDefender = IndirectDefender::Instance();
	assistPoint = AssistPoint::Instance();
	getluadata = GetLuaData::Instance();
}
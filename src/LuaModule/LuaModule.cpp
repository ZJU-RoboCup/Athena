#include <stdio.h>
#include <string.h>
#include <string>
#include "skill/Factory.h"
#include "LuaModule.h"
#include "geometry.h"
#include "TaskMediator.h"
#include "BufferCounter.h"
#include "Global.h"
#include "Compensate.h"
#include "KickOffDefPosV2.h"
#include "MarkingPosV2.h"
#include "PenaltyPosCleaner.h"
#include "defence/EnemyDefendTacticAnalys.h"
#include "WaitKickPos.h"
#include "TouchKickPos.h"
#include "HttpServer.h"
#include "MarkingTouchPos.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "tolua++.h"
TOLUA_API int  tolua_zeus_open (lua_State* tolua_S);

extern "C" {
	typedef struct 
	{
		const char *name;
		int (*func)(lua_State *);
	}luaDef;
}

extern luaDef GUIGlue[];

namespace{
	bool IS_SIMULATION = false;
}

CLuaModule::CLuaModule()
{
	m_pErrorHandler = NULL;

	m_pScriptContext = lua_open();
	luaL_openlibs(m_pScriptContext);
	tolua_zeus_open(m_pScriptContext);
	InitLuaGlueFunc();
//	RunScript("./lua_scripts/StartZeus.lua");
}

void CLuaModule::InitLuaGlueFunc()
{
	for(int i=0; GUIGlue[i].name; i++) {
		AddFunction(GUIGlue[i].name, GUIGlue[i].func);
	}
}

CLuaModule::~CLuaModule()
{
	if(m_pScriptContext)
		lua_close(m_pScriptContext);
}

static std::string findScript(const char *pFname)
{
	FILE *fTest;

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath( pFname, drive, dir, fname, ext );

	std::string strTestFile = (std::string) drive + dir + "Scripts\\" + fname + ".LUB";
	fTest = fopen(strTestFile.c_str(), "r");
	if(fTest == NULL)
	{
		//not that one...
		strTestFile = (std::string) drive + dir + "Scripts\\" + fname + ".LUA";
		fTest = fopen(strTestFile.c_str(), "r");
	}

	if(fTest == NULL)
	{
		//not that one...
		strTestFile = (std::string) drive + dir + fname + ".LUB";
		fTest = fopen(strTestFile.c_str(), "r");
	}

	if(fTest == NULL)
	{
		//not that one...
		//not that one...
		strTestFile = (std::string) drive + dir + fname + ".LUA";
		fTest = fopen(strTestFile.c_str(), "r");
	}

	if(fTest != NULL)
	{
		fclose(fTest);
	}

	return strTestFile;
}

bool CLuaModule::RunScript(const char *pFname)
{
	std::string strFilename = findScript(pFname);
	const char *pFilename = strFilename.c_str();

	if (0 != luaL_loadfile(m_pScriptContext, pFilename))
	{
		if(m_pErrorHandler)
		{
			char buf[256];
			sprintf(buf, "Lua Error - Script Load\nScript Name:%s\nError Message:%s\n", pFilename, luaL_checkstring(m_pScriptContext, -1));
			m_pErrorHandler(buf);
		}

		return false;
	}
	if (0 != lua_pcall(m_pScriptContext, 0, LUA_MULTRET, 0))
	{
		if(m_pErrorHandler)
		{
			char buf[256];
			sprintf(buf, "Lua Error - Script Run\nScript Name:%s\nError Message:%s\n", pFilename, luaL_checkstring(m_pScriptContext, -1));
			m_pErrorHandler(buf);
		}

		return false;
	}
	return true;

}

bool CLuaModule::RunString(const char *pCommand)
{
	if (0 != luaL_loadbuffer(m_pScriptContext, pCommand, strlen(pCommand), NULL))
	{
		if(m_pErrorHandler)
		{
			char buf[256];
			sprintf(buf, "Lua Error - String Load\nString:%s\nError Message:%s\n", pCommand, luaL_checkstring(m_pScriptContext, -1));
			m_pErrorHandler(buf);
		}

		return false;
	}
	if (0 != lua_pcall(m_pScriptContext, 0, LUA_MULTRET, 0))
	{
		if(m_pErrorHandler)
		{
			char buf[256];
			sprintf(buf, "Lua Error - String Run\nString:%s\nError Message:%s\n", pCommand, luaL_checkstring(m_pScriptContext, -1));
			m_pErrorHandler(buf);
		}

		return false;
	}
	return true;
}

const char *CLuaModule::GetErrorString(void)
{
	return luaL_checkstring(m_pScriptContext, -1);
}


bool CLuaModule::AddFunction(const char *pFunctionName, LuaFunctionType pFunction)
{
	lua_register(m_pScriptContext, pFunctionName, pFunction);
	return true;
}

const char *CLuaModule::GetStringArgument(int num, const char *pDefault)
{
	return luaL_optstring(m_pScriptContext, num, pDefault);

}

double CLuaModule::GetNumberArgument(int num, double dDefault)
{
	return luaL_optnumber(m_pScriptContext, num, dDefault);
}

bool CLuaModule::GetBoolArgument(int num)
{
	return lua_toboolean(m_pScriptContext, num);
}

CGeoPoint* CLuaModule::GetPointArgument(int num)
{
	return (CGeoPoint*)(lua_touserdata(m_pScriptContext, num));
}

void CLuaModule::PushString(const char *pString)
{
	lua_pushstring(m_pScriptContext, pString);
}

void CLuaModule::PushNumber(double value)
{
	lua_pushnumber(m_pScriptContext, value);
}

void CLuaModule::PushBool(bool value)
{
	lua_pushboolean(m_pScriptContext, value);
}

extern "C" int Skill_SmartGotoPoint(lua_State *L)
{
	TaskT playerTask;
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	playerTask.executor = runner;
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	playerTask.player.pos = CGeoPoint(x,y);
	playerTask.player.vel = CVector(0.0,0.0);
	playerTask.player.rotvel = 0;
	playerTask.player.angle = LuaModule::Instance()->GetNumberArgument(4, NULL);
	playerTask.player.flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
	playerTask.ball.Sender = LuaModule::Instance()->GetNumberArgument(6,NULL);
	playerTask.player.max_acceleration = LuaModule::Instance()->GetNumberArgument(7,NULL);

	CPlayerTask* pTask = TaskFactoryV2::Instance()->SmartGotoPosition(playerTask);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	
	return 0;
}



extern "C" int Skill_GoAvoidShootLine(lua_State *L)
{
	TaskT playerTask;
	playerTask.player.is_specify_ctrl_method = true;
	playerTask.player.specified_ctrl_method = CMU_TRAJ;
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	playerTask.executor = runner;
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	playerTask.player.pos = CGeoPoint(x,y);
	playerTask.player.angle = LuaModule::Instance()->GetNumberArgument(4, NULL);
	playerTask.player.flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
	playerTask.ball.Sender = LuaModule::Instance()->GetNumberArgument(6,NULL);

	CPlayerTask* pTask = TaskFactoryV2::Instance()->GoAvoidShootLine(playerTask);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

extern "C" int Skill_GoCmuRush(lua_State *L)
{
	TaskT playerTask;
	playerTask.player.is_specify_ctrl_method = true;
	playerTask.player.specified_ctrl_method = CMU_TRAJ;
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	playerTask.executor = runner;
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	playerTask.player.pos = CGeoPoint(x,y);
	playerTask.player.angle = LuaModule::Instance()->GetNumberArgument(4, NULL);
	playerTask.player.flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
	playerTask.ball.Sender = LuaModule::Instance()->GetNumberArgument(6,NULL);
	playerTask.player.max_acceleration = LuaModule::Instance()->GetNumberArgument(7,NULL);
	playerTask.player.needdribble = LuaModule::Instance()->GetNumberArgument(8,NULL);
	//playerTask.player.vel = vision->TheirPlayer(BestPlayer::Instance()->getTheirBestPlayer()).Vel();

	CPlayerTask* pTask = TaskFactoryV2::Instance()->SmartGotoPosition(playerTask);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

extern "C" int Skill_NoneZeroGoCmuRush(lua_State *L)
{
	TaskT playerTask;
	playerTask.player.is_specify_ctrl_method = true;
	playerTask.player.specified_ctrl_method = CMU_TRAJ;
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	playerTask.executor = runner;
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	playerTask.player.pos = CGeoPoint(x, y);
	playerTask.player.angle = LuaModule::Instance()->GetNumberArgument(4, NULL);
	playerTask.player.flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
	playerTask.ball.Sender = LuaModule::Instance()->GetNumberArgument(6, NULL);
	playerTask.player.max_acceleration = LuaModule::Instance()->GetNumberArgument(7, NULL);
	playerTask.player.needdribble = LuaModule::Instance()->GetNumberArgument(8, NULL);
	double velX = LuaModule::Instance()->GetNumberArgument(9, NULL);
	double velY = LuaModule::Instance()->GetNumberArgument(10, NULL);
	playerTask.player.vel = CVector(velX, velY);

	CPlayerTask* pTask = TaskFactoryV2::Instance()->GotoPosition(playerTask);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

extern "C" int Skill_SimpleGotoPoint(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(4, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(5, NULL);
	CPlayerTask* pTask = PlayerRole::makeItSimpleGoto(runner, CGeoPoint(x, y), angle, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

extern "C" int Skill_Stop(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	CPlayerTask* pTask = PlayerRole::makeItStop(runner);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

extern "C" int Register_Role(lua_State *L)
{
	int num = LuaModule::Instance()->GetNumberArgument(1, NULL);
	string role = LuaModule::Instance()->GetStringArgument(2, NULL);
	TaskMediator::Instance()->setRoleInLua(num, role);
	return 0;
}

extern "C" int Skill_Test(lua_State *L)
{
	cout<<lua_type(L,-1)<<endl;
	CVector* pos;
	pos = (CVector*)tolua_tousertype(L, 1, 0);
	cout<<pos->x()<<endl;
	return 0;
}

extern "C" int Skill_TouchKick(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	CPlayerTask* pTask = PlayerRole::makeItTouchKick(runner, angle);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_MarkingTouch(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double leftUpPosX = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double leftUpPosY = LuaModule::Instance()->GetNumberArgument(4,NULL);
	double rightDownPosX = LuaModule::Instance()->GetNumberArgument(5,NULL);
	double rightDownPosY = LuaModule::Instance()->GetNumberArgument(6,NULL);
	int flags= LuaModule::Instance()->GetNumberArgument(7,NULL);
	CGeoPoint leftUpPos=CGeoPoint(leftUpPosX,leftUpPosY);
	CGeoPoint rightDownPos=CGeoPoint(rightDownPosX,rightDownPosY);
	CPlayerTask* pTask = PlayerRole::makeItMarkingTouch(runner, angle,leftUpPos,rightDownPos,flags);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_MarkingFront(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	int markNum = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(3,NULL);
	int flags= LuaModule::Instance()->GetNumberArgument(4,NULL);
	CPlayerTask* pTask = PlayerRole::makeItMarkingFront(runner, markNum, angle, flags);
	cout<<"marking Front"<<endl;
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}


extern "C" int Skill_ShootBall(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double preci = LuaModule::Instance()->GetNumberArgument(3,NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(4,NULL);
	CPlayerTask* pTask = PlayerRole::makeItShootBall(runner, angle, false, preci, 800, 1200, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ShootBallV2(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	CPlayerTask* pTask = PlayerRole::makeItShootBallV2(runner, angle, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}
extern "C" int Skill_GetBall(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	CPlayerTask* pTask = PlayerRole::makeItNoneTrajGetBall(runner, angle, CVector(0,0), 0, -2, CMU_TRAJ);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_GoAndTurnKickV4(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	CPlayerTask* pTask = PlayerRole::makeItGoAndTurnKickV4(runner, angle, CVector(0,0), 0, -2, CMU_TRAJ);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_StaticGetBall(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(3,NULL); 
	CPlayerTask* pTask = PlayerRole::makeItStaticGetBall(runner, angle, CVector(0,0), flag, 12, CMU_TRAJ);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_GoAndTurnKick(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	int type = LuaModule::Instance()->GetNumberArgument(3,NULL);
	int power = LuaModule::Instance()->GetNumberArgument(4, NULL);
	
	enum METHOD
	{
	METHOD_TANGENT_LINE=1,
	METHOD_CIRCLE_PASS=2, // RB_dragon
	};
	const int METHOD = METHOD_TANGENT_LINE;
	CPlayerTask* pTask;

	if (METHOD==METHOD_TANGENT_LINE)
		pTask = PlayerRole::makeItGoAndTurnKick(runner, angle, type, power);
	else if (METHOD==METHOD_CIRCLE_PASS)
		pTask = PlayerRole::makeItCircleAndPass(runner, angle, type, power);
	
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}
extern "C" int Skill_GoAndTurn(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	CPlayerTask* pTask = PlayerRole::makeItGoAndTurn(runner, 0, 0, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}


extern "C" int Skill_CircleBall(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double preci = LuaModule::Instance()->GetNumberArgument(3,NULL);
	CPlayerTask* pTask = PlayerRole::makeItTestCircleBall(runner, angle, preci, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}
extern "C" int Skill_GoAndTurnKickV3(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double preci = LuaModule::Instance()->GetNumberArgument(3,NULL);
	int circleNum = LuaModule::Instance()->GetNumberArgument(4,NULL);
	double fixAngle = LuaModule::Instance()->GetNumberArgument(5,NULL);
	double maxAcc = LuaModule::Instance()->GetNumberArgument(6,NULL);
	int radius = LuaModule::Instance()->GetNumberArgument(7,NULL);
	int numPerCir = LuaModule::Instance()->GetNumberArgument(8,NULL);
	double gotoPre = LuaModule::Instance()->GetNumberArgument(9,NULL);
	double gotoDist = LuaModule::Instance()->GetNumberArgument(10,NULL);
	double adjustPre = LuaModule::Instance()->GetNumberArgument(11,NULL);
	CPlayerTask* pTask = PlayerRole::makeItGoAndTurnKickV3(runner, angle, circleNum, fixAngle,maxAcc,radius,numPerCir,gotoPre,gotoDist,adjustPre,preci);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_SlowGetBall(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	CPlayerTask* pTask = PlayerRole::makeItSlowGetBall(runner, angle, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ForceStartRush(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	CPlayerTask* pTask = PlayerRole::makeItForceStartRush(runner, angle, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ChaseKick(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(3,NULL);
	CPlayerTask* pTask = PlayerRole::makeItChaseKickV1(runner, angle, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ChaseKickV2(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(3,NULL);
	CPlayerTask* pTask = PlayerRole::makeItChaseKickV2(runner, angle, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ChaseKickV3(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(3, NULL);
	CPlayerTask* pTask = PlayerRole::makeItChaseKickV3(runner, angle, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_DriftKick(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	CPlayerTask* pTask = PlayerRole::makeItDriftKick(runner, angle, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_InterKick(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	bool kickway = LuaModule::Instance()->GetBoolArgument(3);
	CPlayerTask* pTask = PlayerRole::makeItInterKickV3(runner, angle, 0, kickway);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_InterKickV2(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2, NULL);
	bool kickway = LuaModule::Instance()->GetBoolArgument(3);
	CPlayerTask* pTask = PlayerRole::makeItInterKickV2(runner, angle, 0, kickway);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_InterKickV5(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2, NULL);
	bool kickway = LuaModule::Instance()->GetBoolArgument(3);
	CPlayerTask* pTask = PlayerRole::makeItInterKickV5(runner, angle, 0, kickway);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_InterKickV6(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2, NULL);
	bool kickway = LuaModule::Instance()->GetBoolArgument(3);
	CPlayerTask* pTask = PlayerRole::makeItInterKickV6(runner, angle, 0, kickway);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ProtectBall(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItProtectBall(runner, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

// 现在使用新的抢球，by dxh 2013.5.20
extern "C" int Skill_AdvanceBallV2(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	int tendemNum = LuaModule::Instance()->GetNumberArgument(3, NULL);
	CPlayerTask* pTask = PlayerRole::makeItAdvanceBallV2(runner, flag, tendemNum);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_DribbleTurn(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double pre = LuaModule::Instance()->GetNumberArgument(3, NULL);
	CPlayerTask* pTask = PlayerRole::makeItDribbleTurn(runner, angle, pre);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ProtectTurn(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double finalDir = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double turnRotVel = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double kickPower = LuaModule::Instance()->GetNumberArgument(4,NULL);
	CPlayerTask* pTask = PlayerRole::makeItProtectTurn(runner, finalDir, turnRotVel,kickPower);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_DribbleTurnKick(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double finalDir = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double turnRotVel = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double kickPower = LuaModule::Instance()->GetNumberArgument(4,NULL);
	CPlayerTask* pTask = PlayerRole::makeItDribbleTurnKick(runner, finalDir, turnRotVel,kickPower);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_AdvanceBallV1(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItAdvanceBallV1(runner, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_ReceivePass(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2,NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(3, NULL);
	CPlayerTask* pTask = PlayerRole::makeItReceivePass(runner, angle, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_Goalie2013(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItGoalie2013(runner, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_Goalie2015(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItGoalie2015(runner, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_Goalie2015V2(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItGoalie2015V2(runner, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}




extern "C" int Skill_PenaltyGoalie(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyGoalie(runner, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_PenaltyGoalieV2(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyGoalieV2(runner, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_PenaltyGoalie2017V1(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyGoalie2017V1(runner, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_PenaltyGoalie2017V2(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	int flag = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyGoalie2017V2(runner, flag);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_Speed(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double speedX = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double speedY = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double rotSpeed = LuaModule::Instance()->GetNumberArgument(4,NULL);
	CPlayerTask* pTask = PlayerRole::makeItRun(runner, speedX, speedY, rotSpeed, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_SpeedLocalVersion(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double speedX = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double speedY = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double rotSpeed = LuaModule::Instance()->GetNumberArgument(4,NULL);
	CPlayerTask* pTask = PlayerRole::makeItRunLocalVersion(runner, speedX, speedY, rotSpeed);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_Marking(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	int pri    = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double x = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double y = LuaModule::Instance()->GetNumberArgument(4,NULL);
	int flag   = LuaModule::Instance()->GetNumberArgument(5,NULL);
	bool front = LuaModule::Instance()->GetBoolArgument(6);
	double dir = LuaModule::Instance()->GetNumberArgument(7,NULL);
	int bestEnemy = DefenceInfo::Instance()->getAttackOppNumByPri(0);
	if (DefenceInfo::Instance()->getOppPlayerByNum(bestEnemy)->isTheRole("RReceiver")) {
		if (pri > 0) {
			pri -= 1;
		}		
	}
	pri = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(pri);
	CPlayerTask* pTask = PlayerRole::makeItMarkEnemy(runner, pri, front, flag, CGeoPoint(x,y),dir);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_Blocking(lua_State *L)
{
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	int pri    = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double x = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double y = LuaModule::Instance()->GetNumberArgument(4,NULL);
	int flag   = LuaModule::Instance()->GetNumberArgument(5,NULL);

	pri = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(pri);

	CPlayerTask* pTask = PlayerRole::makeItMarkEnemy(runner, pri, false,flag,CGeoPoint(x,y));
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int FUNC_GetMarkingPos(lua_State *L)
{	
	int pri = LuaModule::Instance()->GetNumberArgument(1,NULL);
	bool front = LuaModule::Instance()->GetBoolArgument(2);
	CGeoPoint p;
	int bestEnemy = DefenceInfo::Instance()->getAttackOppNumByPri(0);
	//当receiver为最高优先级的时候，这句话可以理解为场上对方是否有receiver
	if (DefenceInfo::Instance()->getOppPlayerByNum(bestEnemy)->isTheRole("RReceiver")){
		if (pri > 0) {
			pri -= 1;
		}		
	}
	int oppNum = DefenceInfo::Instance()->getSteadyAttackOppNumByPri(pri);
	const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
	static bool kickOffSide = false;
	bool checkKickOffArea = false;
	if ("theirIndirectKick" == refMsg || "theirDirectKick" == refMsg){
		if (vision->Ball().Y() >0){
			kickOffSide = false;
		}else{
			kickOffSide = true;
		}
	}
	if (kickOffSide == false){
		CGeoPoint leftUp = CGeoPoint(Param::Field::PITCH_LENGTH/2,30+20);
		CGeoPoint rightDown = CGeoPoint(80,Param::Field::PITCH_WIDTH/2);
		if (DefenceInfo::Instance()->checkInRecArea(oppNum,vision,MarkField(leftUp,rightDown))){
			checkKickOffArea = true;
		}
	}else{
		CGeoPoint leftUp = CGeoPoint(Param::Field::PITCH_LENGTH/2,-Param::Field::PITCH_WIDTH/2);
		CGeoPoint rightDown = CGeoPoint(80,-30-20);
		if (DefenceInfo::Instance()->checkInRecArea(oppNum,vision,MarkField(leftUp,rightDown))){
			checkKickOffArea = true;
		}
	}
	if (front && vision->TheirPlayer(oppNum).Pos().x()>-190 && false == checkKickOffArea) {
		double dir = (vision->Ball().Pos() - vision->TheirPlayer(oppNum).Pos()).dir();
		p = vision->TheirPlayer(oppNum).Pos() + Utils::Polar2Vector(35,dir);
	} else{
		p = MarkingPosV2::Instance()->getMarkingPos(vision, pri);
	}
	LuaModule::Instance()->PushNumber(p.x());
	LuaModule::Instance()->PushNumber(p.y());
	return 2;
}

extern "C" int FUNC_GetBlockingPos(lua_State *L)
{	
	int pri = LuaModule::Instance()->GetNumberArgument(1,NULL);
	CGeoPoint p = MarkingPosV2::Instance()->getMarkingPosByAbsolutePri(vision,pri);
	LuaModule::Instance()->PushNumber(p.x());
	LuaModule::Instance()->PushNumber(p.y());
	return 2;
}

extern "C" int FUNC_TimeOut(lua_State* L)
{
	bool cond = LuaModule::Instance()->GetBoolArgument(1);
	int buf = LuaModule::Instance()->GetNumberArgument(2, NULL);
	int cnt = LuaModule::Instance()->GetNumberArgument(3, 9999);

	if(BufferCounter::Instance()->isClear(vision->Cycle())){
		BufferCounter::Instance()->startCount(vision->Cycle(), cond, buf, cnt);
	}

	if(BufferCounter::Instance()->timeOut(vision->Cycle(), cond)){
		LuaModule::Instance()->PushNumber(1);
	} else{
		LuaModule::Instance()->PushNumber(0);
	}
	return 1;
}

extern "C" int FUNC_GetRealNum(lua_State* L)
{
	int num = LuaModule::Instance()->GetNumberArgument(1, NULL);
	LuaModule::Instance()->PushNumber(PlayInterface::Instance()->getRealIndexByNum(num));
	return 1;
}

extern "C" int FUNC_GetStrategyNum(lua_State* L)
{
	int num = LuaModule::Instance()->GetNumberArgument(1, NULL);
	LuaModule::Instance()->PushNumber(PlayInterface::Instance()->getNumbByRealIndex(num));
	return 1;
}

extern "C" int FUNC_GetIsSimulation(lua_State* L)
{
	DECLARE_PARAM_READER_BEGIN(General)
	READ_PARAM(IS_SIMULATION)
	DECLARE_PARAM_READER_END
	
	LuaModule::Instance()->PushBool(IS_SIMULATION);
	return 1;
}

extern "C" int FUNC_GetKickOffDefPos(lua_State* L)
{
	string str = LuaModule::Instance()->GetStringArgument(1,NULL);
	CGeoPoint pos;
	if ( "left" == str) {
		pos = KickOffDefPosV2::Instance()->GetLeftPos(vision);
	}

	if ( "right" == str) {
		pos = KickOffDefPosV2::Instance()->GetRightPos(vision);
	}

	if ( "middle" == str) {
		pos = KickOffDefPosV2::Instance()->GetMidPos(vision);
	}
	LuaModule::Instance()->PushNumber(pos.x());
	LuaModule::Instance()->PushNumber(pos.y());

	return 2;
}

extern "C" int FUNC_CalCompensate(lua_State*L){
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double targetX = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double targetY = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double realDir;
	realDir = Compensate::Instance()->getKickDir(runner,CGeoPoint(targetX,targetY));
	LuaModule::Instance()->PushNumber(realDir);
	return 1;
}

extern "C" int FUNC_AddPenaltyCleaner(lua_State*L){
	string role = LuaModule::Instance()->GetStringArgument(1,NULL);
	int num = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double targetX = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double targetY = LuaModule::Instance()->GetNumberArgument(4,NULL);
	PenaltyPosCleaner::Instance()->add(role,num, CGeoPoint(targetX,targetY));
	return 0;
}

extern "C" int FUNC_GetPenaltyCleaner(lua_State*L){
	string role = LuaModule::Instance()->GetStringArgument(1,NULL);
	CGeoPoint p = PenaltyPosCleaner::Instance()->get(role);
	LuaModule::Instance()->PushNumber(p.x());
	LuaModule::Instance()->PushNumber(p.y());
	return 2;
}

extern "C" int FUNC_CleanPenalty(lua_State*L){
	PenaltyPosCleaner::Instance()->clean(vision);
	return 0;
}
extern "C" int FUNC_ChipBallJudge(lua_State*L){
	double  ballMovingDir = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double ballStartPosX = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double ballStartPosY = LuaModule::Instance()->GetNumberArgument(3,NULL);
	CGeoPoint ballStartPos = CGeoPoint(ballStartPosX,ballStartPosY);
	bool ischip = ChipBallJudge::Instance()->doJudge(vision,ballMovingDir,ballStartPos);
	LuaModule::Instance()->PushBool(ischip);
	LuaModule::Instance()->PushNumber(ballMovingDir);
	LuaModule::Instance()->PushNumber(ballStartPos.x());
	LuaModule::Instance()->PushNumber(ballStartPos.y());
	return 4;
}

extern "C" int FUNC_GetResetGroup(lua_State*L){
	GroupStatus* status = PenaltyPosCleaner::Instance()->getGroupStatus();
	char str[20] = "";

	for(map<string, SGroup>::iterator iter = status->begin(); iter != status->end(); iter++){
		char tmpStr[8] = "[";
		for(int j = 0; j < iter->second._names.size(); j++){
			strcat(tmpStr, iter->second._names[j].c_str());
		}
		strcat(tmpStr, "]");
		strcat(str, tmpStr);
	}
	LuaModule::Instance()->PushString(str);
	return 1;
}

extern "C" int Skill_WaitTouch(lua_State*L){
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double dir = LuaModule::Instance()->GetNumberArgument(4, NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(5, NULL);
	//cout<<"dir "<<dir<<" angle"<<angle<<endl;

	CPlayerTask* pTask = PlayerRole::makeItWaitTouch(runner, CGeoPoint(x,y),dir,angle);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_WaitTouch2016(lua_State*L){
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double dir = LuaModule::Instance()->GetNumberArgument(4, NULL);
	//cout<<"dir "<<dir<<" angle"<<angle<<endl;
	//cout << CGeoPoint(x, y) << endl;
	CPlayerTask* pTask = PlayerRole::makeItWaitTouch2016(runner, CGeoPoint(x, y), dir);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_TouchBetweenPos(lua_State*L){
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double x1 = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y1 = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double x2 = LuaModule::Instance()->GetNumberArgument(4, NULL);
	double y2 = LuaModule::Instance()->GetNumberArgument(5, NULL);
	double dir = LuaModule::Instance()->GetNumberArgument(6,NULL);
    TaskT playerTask;
	playerTask.executor =  runner;
	double ball2me = (vision->Ball().Pos() - vision->OurPlayer(runner).Pos()).dir();
	double myDir = vision->OurPlayer(runner).Dir();
	
	if (vision->Ball().Pos().dist(vision->OurPlayer(runner).Pos())<30 && fabs(ball2me - myDir)<Param::Math::PI/4 && 
		vision->Ball().Vel().mod()<50){
		CPlayerTask* pTask = PlayerRole::makeItNoneTrajGetBall(runner, dir, CVector(0,0), 0, -2);
		TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	}else{
		WaitKickPos::Instance()->GenerateWaitKickPos(CGeoPoint(x1,y1),CGeoPoint(x2,y2),runner,dir);
		playerTask.player.pos = WaitKickPos::Instance()->getKickPos(runner);
		playerTask.player.angle = dir;
		playerTask.player.max_acceleration = 1000;
		CPlayerTask* pTask = TaskFactoryV2::Instance()->GotoPosition(playerTask);
		TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	}
return 0;
}

extern "C" int Skill_MarkingField(lua_State*L){
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double x1 = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y1 = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double x2 = LuaModule::Instance()->GetNumberArgument(4, NULL);
	double y2 = LuaModule::Instance()->GetNumberArgument(5, NULL);
	double dir = LuaModule::Instance()->GetNumberArgument(6,NULL);
	int style = LuaModule::Instance()->GetNumberArgument(7,NULL);
	
	TaskT playerTask;
	playerTask.executor =  runner;
	double ball2me = (vision->Ball().Pos() - vision->OurPlayer(runner).Pos()).dir();
	double myDir = vision->OurPlayer(runner).Dir();

	if (vision->Ball().Pos().dist(vision->OurPlayer(runner).Pos())<30 && fabs(ball2me - myDir)<Param::Math::PI/4 && 
		vision->Ball().Vel().mod()<50){
			CPlayerTask* pTask = PlayerRole::makeItNoneTrajGetBall(runner, dir, CVector(0,0), 0, -2);
			TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	}else{
		WaitKickPos::Instance()->GenerateWaitKickPos(CGeoPoint(x1,y1),CGeoPoint(x2,y2),runner,dir);
		playerTask.player.pos = WaitKickPos::Instance()->getKickPos(runner);
		playerTask.player.angle = dir;
		playerTask.player.max_acceleration = 1000;
		CPlayerTask* pTask = TaskFactoryV2::Instance()->GotoPosition(playerTask);
		TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	}
	return 0;
}

extern "C" int Skill_PenaltyKick2013(lua_State*L){
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyKick2013(runner,0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_PenaltyKick2014(lua_State*L){
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyKick2014(runner,0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_PenaltyKick2017V1(lua_State*L) {
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyKick2017V1(runner, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_PenaltyKick2017V2(lua_State*L) {
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	CPlayerTask* pTask = PlayerRole::makeItPenaltyKick2017V2(runner, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int Skill_CrazyPush(lua_State*L){
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double faceDir = LuaModule::Instance()->GetNumberArgument(2, NULL);
	CPlayerTask* pTask = PlayerRole::makeItCrazyPush(runner, faceDir);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
};

extern "C" int FUNC_CGetDefNumByRolename(lua_State* L){
	string rolename = LuaModule::Instance()->GetStringArgument(1, NULL);
	string str = EnemyDefendTacticAnalys::Instance()->doAnalys(rolename,vision);
	LuaModule::Instance()->PushString(str.c_str());
	return 1;
}

extern "C" int FUNC_CGetDefRolenameByNum(lua_State* L){
	int num = LuaModule::Instance()->GetNumberArgument(1, NULL);
	string str = EnemyDefendTacticAnalys::Instance()->doAnalys(num,vision);
	LuaModule::Instance()->PushString(str.c_str());
	return 1;
}

extern "C" int FUNC_EnemyHasReceiver(lua_State* L){
	bool istrue = false;
	for(int i = 1; i <= Param::Field::MAX_PLAYER; i++) {
		if(DefenceInfo::Instance()->getOppPlayerByNum(i)->isTheRole("RReceiver")){
			istrue = true;
		}
	}
	LuaModule::Instance()->PushBool(istrue);
	return 1;
}

extern "C" int Skill_GoAroundRobot(lua_State* L){
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double angle = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double x = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(4, NULL);
	double dir = LuaModule::Instance()->GetNumberArgument(5, NULL);
	double radius = LuaModule::Instance()->GetNumberArgument(6,NULL);
	CPlayerTask* pTask = PlayerRole::makeItGoAroundRobot(runner,angle,CGeoPoint(x,y),dir,radius,0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

extern "C" int FUNC_SetRoleAndNum(lua_State* L){
	string rolename = LuaModule::Instance()->GetStringArgument(1, NULL);
	int num = LuaModule::Instance()->GetNumberArgument(2, NULL);
	PlayInterface::Instance()->setRoleNameAndNum(rolename, num);
	return 0;
}

extern "C" int Skill_SpeedInRobot(lua_State* L){
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double speedX = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double speedY = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double rotSpeed = LuaModule::Instance()->GetNumberArgument(4,NULL);
	CVector localVel(speedX, speedY);
	CVector globalVel = localVel.rotate(vision->OurPlayer(runner).Dir());
	DribbleStatus::Instance()->setDribbleCommand(runner,3);
	CPlayerTask* pTask = PlayerRole::makeItRun(runner, globalVel.x(), globalVel.y(), rotSpeed, 0);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

//当出现receiver时，找到离球最近的receiver，并看他是否被绕前盯人
extern "C" int FUNC_IsNearestBallReceiverBeDenied(lua_State* L){
	bool bl = MarkingPosV2::Instance()->isNearestBallReceiverBeDenied(vision);
	LuaModule::Instance()->PushBool(bl);
	return 1;
}

//在NormalPlay中MarkingX值最靠前的车的匹配点
extern "C" int FUNC_MarkingXFirstPos(lua_State* L){
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	CGeoPoint pos = MarkingPosV2::Instance()->getMarkingPosByNum(vision,runner);

	LuaModule::Instance()->PushNumber(pos.x());
	LuaModule::Instance()->PushNumber(pos.y());

	return 2;
}

//在NormalPlay中Markingx值最靠前的车
extern "C" int FUNC_MarkingXFirstNum(lua_State* L){
	int runner = LuaModule::Instance()->GetNumberArgument(1,NULL);
	int enemy = LuaModule::Instance()->GetNumberArgument(2,NULL);
	
	CPlayerTask* pTask = PlayerRole::makeItMarkEnemy(runner, enemy);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);

	return 0;
}

// 从Lua中注册开球车朝向供Touch使用，防止球看不见时车会上前拿球
extern "C" int FUNC_SetPassDir(lua_State* L){
	double dir = LuaModule::Instance()->GetNumberArgument(1,NULL);
	TouchKickPos::Instance()->setPassDir(vision->Cycle(), dir);

	return 0;
}

extern "C" int FUNC_GetMarkingTouchPos(lua_State* L){
	int mAreaNum = LuaModule::Instance()->GetNumberArgument(1,NULL);
	double x1 = LuaModule::Instance()->GetNumberArgument(2,NULL);
	double y1 = LuaModule::Instance()->GetNumberArgument(3,NULL);
	double x2 = LuaModule::Instance()->GetNumberArgument(4,NULL);
	double y2 = LuaModule::Instance()->GetNumberArgument(5,NULL);
	//cout<<x1<<" "<<y1<<" "<<x2<<" "<<y2<<endl;
	int markingDirection = LuaModule::Instance()->GetNumberArgument(6,NULL);
	CGeoPoint p = CGeoPoint(0,0);
	if (markingDirection == 1){
		p = MarkingTouchPos::Instance()->caculMarkingTouchPos(mAreaNum,CGeoPoint(x1,y1),CGeoPoint(x2,y2),true);
	}else{
		p = MarkingTouchPos::Instance()->caculMarkingTouchPos(mAreaNum,CGeoPoint(x1,y1),CGeoPoint(x2,y2),false);
	}
	//GDebugEngine::Instance()->gui_debug_msg(p,"P",COLOR_WHITE);
	LuaModule::Instance()->PushNumber(p.x());
	LuaModule::Instance()->PushNumber(p.y());

	return 2;
}

extern "C" int Skill_InterceptTouch(lua_State* L) {
	int runner = LuaModule::Instance()->GetNumberArgument(1, NULL);
	double x = LuaModule::Instance()->GetNumberArgument(2, NULL);
	double y = LuaModule::Instance()->GetNumberArgument(3, NULL);
	double touchDir = LuaModule::Instance()->GetNumberArgument(4, NULL);
	double power = LuaModule::Instance()->GetNumberArgument(5, NULL);
	double buffer = LuaModule::Instance()->GetNumberArgument(6, NULL);
	int useChip = LuaModule::Instance()->GetNumberArgument(7, NULL);
	bool testMode = LuaModule::Instance()->GetBoolArgument(8);
	CPlayerTask* pTask = PlayerRole::makeItInterceptTouch(runner, CGeoPoint(x, y), touchDir, power, buffer, useChip,testMode);
	TaskMediator::Instance()->setPlayerTask(runner, pTask, 1);
	return 0;
}

luaDef GUIGlue[] = 
{
	{"SmartGotoPos",		Skill_SmartGotoPoint},
	{"SimpleGotoPos",		Skill_SimpleGotoPoint},
	{"StopRobot",			Skill_Stop},
	{"CRegisterRole",		Register_Role},
	{"TestSkill",			Skill_Test},
	{"CTouchKick",			Skill_TouchKick},
	{"CMarkingTouch",		Skill_MarkingTouch},
	{"CMarkingFront",		Skill_MarkingFront},
	{"CShootBall",			Skill_ShootBall},
	{"CShootBallV2",		Skill_ShootBallV2},
	{"CGetBall",			Skill_GetBall},
	{"CStaticGetBall",		Skill_StaticGetBall},
	{"CGoAndTurnKick",		Skill_GoAndTurnKick},
	{"CGoAndTurn",			Skill_GoAndTurn},
	{"CSlowGetBall",        Skill_SlowGetBall},
	{"CForceStartRush",		Skill_ForceStartRush},
	{"CChaseKick",			Skill_ChaseKick},
	{"CChaseKickV2",		Skill_ChaseKickV2},
	{"CChaseKickV3",		Skill_ChaseKickV3},
	{"CDriftKick",			Skill_DriftKick},
	{"CInterKick",			Skill_InterKick},
	{"CInterKickV2",		Skill_InterKickV2 },
	{"CInterKickV5",		Skill_InterKickV5 },
	{"CInterKickV6",		Skill_InterKickV6 },
	{"CGoalie2013",         Skill_Goalie2013},
	{"CGoalie2015",         Skill_Goalie2015 },
	{"CGoalie2015V2",		Skill_Goalie2015V2 },
	{"CProtectBall",		Skill_ProtectBall},
	{"CAdvanceBall",		Skill_AdvanceBallV2},
	{"CAdvanceBallOld",		Skill_AdvanceBallV1},
	{"CReceivePass",		Skill_ReceivePass},
	{"CMarking",			Skill_Marking},
	{"CTimeOut",			FUNC_TimeOut},
	{"CGetRealNum",			FUNC_GetRealNum},
	{"CGetIsSimulation",	FUNC_GetIsSimulation},
	{"CTestCircleBall",     Skill_CircleBall},
	{"CPenaltyGoalie",		Skill_PenaltyGoalie},
	{"CPenaltyGoalieV2",    Skill_PenaltyGoalieV2},
	{"CPenaltyGoalie2017V1",Skill_PenaltyGoalie2017V1},
	{"CPenaltyGoalie2017V2",Skill_PenaltyGoalie2017V2},
	{"CSpeed",				Skill_Speed},
	{"COpenSpeed",    Skill_SpeedLocalVersion},
	{"CGoAndTurnKickV3",	Skill_GoAndTurnKickV3},
	{"CCalCompensateDir",	FUNC_CalCompensate},
	{"CChipBallJudge",		FUNC_ChipBallJudge},
	{"CKickOffDefPos",		FUNC_GetKickOffDefPos},
	{"CGetMarkingPos",      FUNC_GetMarkingPos},
	{"CGetMarkingTouchPos",	FUNC_GetMarkingTouchPos},
	{"CAddPenaltyCleaner",  FUNC_AddPenaltyCleaner},
	{"CGetPenaltyCleaner",  FUNC_GetPenaltyCleaner},
	{"CCleanPenalty",       FUNC_CleanPenalty},
	{"CGetResetMatchStr",	FUNC_GetResetGroup},
	{"CSetRoleAndNum",		FUNC_SetRoleAndNum},
	{"CWaitTouch",			Skill_WaitTouch},
	{"CWaitTouch2016",      Skill_WaitTouch2016 },
	{"CPenaltyKick2013",	Skill_PenaltyKick2013},
	{"CPenaltyKick2014",	Skill_PenaltyKick2014},
	{"CPenaltyKick2017V1",	Skill_PenaltyKick2017V1},
	{"CPenaltyKick2017V2",	Skill_PenaltyKick2017V2},
	{"CGoCmuRush",			Skill_GoCmuRush},
	{"CNoneZeroGoCmuRush",	Skill_NoneZeroGoCmuRush},
	{"CGoAvoidShootLine",	Skill_GoAvoidShootLine},
	{"CTouchBetweenPos",	Skill_TouchBetweenPos},
	{"CGetDefendNumByName", FUNC_CGetDefNumByRolename},
	{"CGetDefendNameByNum", FUNC_CGetDefRolenameByNum},
	{"CEnemyHasReceiver",	FUNC_EnemyHasReceiver},
	{"CCrazyPush",			Skill_CrazyPush},
	{"CGoAroundRobot",		Skill_GoAroundRobot},
	{"CBlocking",			Skill_Blocking},
	{"CGetBlockingPos",		FUNC_GetBlockingPos},
	{"CSpeedInRobot",		Skill_SpeedInRobot},
	{"CShouldAdvanceReceiver",FUNC_IsNearestBallReceiverBeDenied},
	{"CMarkingXFirstPos",	FUNC_MarkingXFirstPos},
	{"CMarkingXFirstNum",	FUNC_MarkingXFirstNum},
	{"CSetPassDir",			FUNC_SetPassDir},
	{"CDribbleTurn",        Skill_DribbleTurn},
	{"CDribbleTurnKick",    Skill_DribbleTurnKick},
	{"CMarkingField",       Skill_MarkingField},
	{"CGetStrategyNum",     FUNC_GetStrategyNum},
	{"CGoAndTurnKickV4",	Skill_GoAndTurnKickV4},
	{"CProtectTurn",		Skill_ProtectTurn},
	{"CInterceptTouch", Skill_InterceptTouch},
	{NULL, NULL}
};
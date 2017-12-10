#define _WIN32_DCOM
#include "ServerInterface.h"
#include <weerror.h>
#include <WorldModel.h>
#include <iostream>
namespace{
	IPlayer* pPlayer = 0;
	ITeam* pTeam = 0;
	IViewer* pViewer = 0;
	IController* pController = 0;
	bool stopCommandSent[Param::Field::MAX_PLAYER]; // ÊÇ·ñ·¢ËÍÁËÍ£Ö¹ÃüÁî
}
CServerInterface::CServerInterface(const COptionModule* pOption)
{
	int side=1;
	if (pOption->MyColor()==TEAM_YELLOW){
		side=-1;
	}
	if( CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK ){
		throw WEError(__FILE__, __LINE__, "CoInitializeEx Failed");
	}
	if( CoCreateInstance(CLSID_SimServer, NULL,CLSCTX_LOCAL_SERVER, IID_IViewer, (void **)&pViewer) != S_OK ){
		throw WEError(__FILE__,__LINE__,"CreateInstance Failed");
	}
	if( true ){
		if( pViewer->QueryInterface(IID_ITeam, (void **)&pTeam) != S_OK ){
			throw WEError(__FILE__,__LINE__,"QueryInterface ITeam Failed");
		}
		if( pTeam->TeamJoinIn(side,pOption->MyColor(),pOption->MyType()) != S_OK ){
			throw WEError(__FILE__,__LINE__,"Team Join Failed");
		}
		std::cout << "Team mode (" << pOption->MySide() << ") running." << Param::Output::NewLineCharacter;
	}else{
		if( pViewer->QueryInterface(IID_IPlayer, (void **)&pPlayer) != S_OK ){
			throw WEError(__FILE__,__LINE__,"QueryInterface IPlayer Failed");
		}

		if( pPlayer->JoinIn(side,pOption->MyNumber(),pOption->MyType()) != S_OK ){
			throw WEError(__FILE__,__LINE__,"Join Failed");
		}
		std::cout << "Single player mode (" << pOption->MySide() << "," << pOption->MyNumber() << ") running." << Param::Output::NewLineCharacter;
	}
	if( pViewer->QueryInterface(IID_IController, (void **)&pController) != S_OK ){ // ¿ØÖÆserver
		throw WEError(__FILE__,__LINE__,"QueryInterface IController Failed");
	}
}
CServerInterface::~CServerInterface()
{
	if( pController ){
		pController->Release();
		pController = 0;
	}
	if( pPlayer ){
		pPlayer->Release();
		pPlayer = 0;
	}
	if( pTeam ){
		pTeam->Release();
		pTeam = 0;
	}
	if( pViewer ){
		pViewer->Release();
		pViewer = 0;
	}
	CoUninitialize();
}
bool CServerInterface::PlayerAct(int num, short type, short value1, short value2, short value3)
{
	if( type == CTStop || type == CTSpeed && value1 == 0 && value2 == 0 && value3 == 0 ){
		// ÊÇÍ£Ö¹ÃüÁî
		if( stopCommandSent[num-1] ){
			return true;
		}else{
			stopCommandSent[num-1] = true;
		}
	}else{
		stopCommandSent[num-1] = false;
	}
	if( pTeam ){
		return (pTeam->TeamAct(num, type, value1, value2, value3) == S_OK);
	}
	if( pPlayer ){
		return (pPlayer->Act(type, value1, value2, value3) == S_OK);
	}
	return false;
}
bool CServerInterface::GetVisualInfo(VisualInfoT* pinfo)
{
	return (pViewer->GetVisualInfo(pinfo) == S_OK);
}
bool CServerInterface::Pause(int status)
{
	return (pController->Pause(status) == S_OK);
}
bool CServerInterface::MoveBall(double x, double y)
{
	return (pController->MoveBall(x,y,0,0) == S_OK);
}
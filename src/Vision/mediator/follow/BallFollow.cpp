#include "BallFollow.h"
#include "message.h"
#include "GlobalData.h"
#include "munkres.h"
#include "math.h"

BallFollow::BallFollow(int cameraId1,int cameraId2)
{
	initFollowArray();
	initSmsg();
	_matrix = Matrix<double>(MaxBallNum, MaxBallNum);
	_camera1=cameraId1;_camera2=cameraId2;
}

BallFollow::~BallFollow(void)
{

}


bool BallFollow::checkSend()
{
	for(int i = 0; i < MaxBallNum; i++){
		if(followarray[i].Send){
			return false;
		}
	}
	return true;
}

void BallFollow::initSmsg()
{
	GlobalData::Instance()->smsg.Ballx = -ERRORNUM;
	GlobalData::Instance()->smsg.Bally = -ERRORNUM;
	GlobalData::Instance()->smsg.BallFound = false;
}

bool BallFollow::determinesend( int minAddFrame, int minLostFrame, bool isSendFalse)
{
	for(int i = 0; i < MaxBallNum; i++){
		if(followarray[i].Fresh == false){
			if(followarray[i].LostFrame < minLostFrame ){
				followarray[i].LostFrame++;
			} else{
				followarray[i].LostFrame = minLostFrame;
				followarray[i].AddFrame = 0;
			}
		} else{
			if(followarray[i].AddFrame < minAddFrame){
				followarray[i].AddFrame++;
			} else{
				followarray[i].AddFrame = minAddFrame;
				followarray[i].LostFrame = 0;
			}
		}
	}
	for(int i=0;i<MaxBallNum;i++){
		if (followarray[i].Fresh && followarray[i].AddFrame==minAddFrame) {
			if (checkSend()) {
				followarray[i].Send=true;
				GlobalData::Instance()->smsg.Ballx=followarray[i].PosX;
				GlobalData::Instance()->smsg.Bally=followarray[i].PosY;
				GlobalData::Instance()->smsg.BallFound=true;
				followarray[i].Fresh=false;
			}
		} else if (!followarray[i].Fresh && followarray[i].LostFrame==minLostFrame) {
			if(!followarray[i].Send || (thereIsStableOne(minAddFrame, i) && followarray[i].Send)){
				cancel(minLostFrame,i);
			}
			continue;
		}
		if(followarray[i].Send == true){
			if( !isSendFalse){
				if( followarray[i].LostFrame < minLostFrame ){
					GlobalData::Instance()->smsg.BallFound = true;
				} else{
			      GlobalData::Instance()->smsg.BallFound = false;
				}
			} else{
				GlobalData::Instance()->smsg.BallFound = followarray[i].Fresh;
			}
			GlobalData::Instance()->smsg.Ballx = followarray[i].PosX;
			GlobalData::Instance()->smsg.Bally = followarray[i].PosY;
			followarray[i].Fresh = false;
		}
	}
	//if(GlobalData::Instance()->smsg.Ballx < 0){
	//	bool tmpfollow[12];
	//	int j;
	//	for(int i=0;i<MaxBallNum;i++){
	//		tmpfollow[i]=followarray[i].Send;
	//	}
	//	bool k=GlobalData::Instance()->smsg.BallFound;
	//	double x=GlobalData::Instance()->smsg.Ballx;
	//	ReceiveVisionMessage temdfdrecMsg = GlobalData::Instance()->receiveMsg[_camera2];
	//	int cycy = GlobalData::Instance()->smsg.Cycle;
	//	j = 3;
	//}
	return false;
}

void BallFollow::startFollowSingle(int camera, int minAddFrame, int minLostFrame, float maxdist)
{
	for ( int row = 0 ; row < MaxBallNum ; row++ ) {
		for ( int col = 0 ; col <MaxBallNum  ; col++ ) {
		if(distances(followarray[row].PosX,followarray[row].PosY,
				GlobalData::Instance()->receiveMsg[camera].Ballx[col],
				GlobalData::Instance()->receiveMsg[camera].Bally[col]) < BALLCALIERROR*BALLCALIERROR){
					_matrix(row,col) = -ERRORNUM*ERRORNUM;
			} else{
				_matrix(row,col)= distances(followarray[row].PosX,followarray[row].PosY,
					GlobalData::Instance()->receiveMsg[camera].Ballx[col],GlobalData::Instance()->receiveMsg[camera].Bally[col]);
			}
		}
	}

	Munkres _munkres;
	_munkres.solve(_matrix);

	double tmpMatrix[MaxBallNum][MaxBallNum];
	for ( int row = 0 ; row < MaxBallNum ; row++ ) {
		for ( int col = 0 ; col < MaxBallNum ; col++ ) {
			tmpMatrix[row][col] = _matrix(row, col);
		}
	}

	ReceiveVisionMessage tmprecmsg=GlobalData::Instance()->receiveMsg[camera];

	for ( int row = 0 ; row < MaxBallNum ; row++ ) {
		for ( int col = 0 ; col <MaxBallNum  ; col++ ) {
			if(_matrix(row,col) != INITNUM && canBeSet(followarray[row].PosX,followarray[row].PosY,
				GlobalData::Instance()->receiveMsg[camera].Ballx[col], 
				GlobalData::Instance()->receiveMsg[camera].Bally[col])&& GlobalData::Instance()->receiveMsg[camera].BallFound[col]){
				followarray[row].PosX = GlobalData::Instance()->receiveMsg[camera].Ballx[col];
				followarray[row].PosY =GlobalData::Instance()-> receiveMsg[camera].Bally[col];
				followarray[row].Fresh = true;
			}
		}
	}
}

void BallFollow::startFollowDouble(int minAddFrame, int minLostFrame, float maxdist)
{
	//ÈÚºÏ
	ReceiveVisionMessage mixedrecMsg;
	for(int i = 0; i<MaxBallNum; i++){
		mixedrecMsg.BallFound[i] = false;
		mixedrecMsg.Ballx[i] = -ERRORNUM;
		mixedrecMsg.Bally[i] = -ERRORNUM;
	}
	
	int countNum = 0;
	for(int leftnum=0;leftnum<MaxBallNum;leftnum++){
		if(GlobalData::Instance()->receiveMsg[_camera1].BallFound[leftnum]){
			int rightnum = 0;
			for (rightnum = 0; rightnum < MaxBallNum; rightnum++){
				if(ismix(leftnum,rightnum) && countNum < MaxBallNum){
					mixedrecMsg.Ballx[countNum]=(GlobalData::Instance()->receiveMsg[_camera1].Ballx[leftnum]+GlobalData::Instance()->receiveMsg[_camera2].Ballx[rightnum])/2;
					mixedrecMsg.Bally[countNum]=(GlobalData::Instance()->receiveMsg[_camera1].Bally[leftnum]+GlobalData::Instance()->receiveMsg[_camera2].Bally[rightnum])/2;
					mixedrecMsg.BallFound[countNum]=true;
					countNum++;
					break;
				}
			}
			if(rightnum == MaxBallNum && countNum < MaxBallNum){
				mixedrecMsg.Ballx[countNum]=GlobalData::Instance()->receiveMsg[_camera1].Ballx[leftnum];
				mixedrecMsg.Bally[countNum]=GlobalData::Instance()->receiveMsg[_camera1].Bally[leftnum];
				mixedrecMsg.BallFound[countNum]=true;
				countNum++;
			}
		}
	}
	for(int rightnum = 0; rightnum < MaxBallNum; rightnum++){
		if(GlobalData::Instance()->receiveMsg[_camera2].BallFound[rightnum]){
			int leftnum = 0;
			for (leftnum = 0; leftnum < MaxBallNum; leftnum++){
				if(ismix(leftnum,rightnum) && countNum < MaxBallNum){
					break;
				}
			}
			if(leftnum == MaxBallNum && countNum < MaxBallNum){
				mixedrecMsg.Ballx[countNum]=GlobalData::Instance()->receiveMsg[_camera2].Ballx[rightnum];
				mixedrecMsg.Bally[countNum]=GlobalData::Instance()->receiveMsg[_camera2].Bally[rightnum];
				mixedrecMsg.BallFound[countNum]=true;
				countNum++;
			}
		}
	}

	//Æ¥Åä
	for(int row= 0; row< MaxBallNum; row++){
		for(int col= 0; col< MaxBallNum; col++){
			if(distances(followarray[row].PosX,followarray[row].PosY,
			  mixedrecMsg.Ballx[col],mixedrecMsg.Bally[col]) < BALLCALIERROR*BALLCALIERROR){
				_matrix(row,col) = -ERRORNUM*ERRORNUM;
			} else{
				_matrix(row,col)= sqrt(distances(followarray[row].PosX,followarray[row].PosY,
					mixedrecMsg.Ballx[col],mixedrecMsg.Bally[col]));
			}
			
		}
	}
		
	Munkres _munkres;
	_munkres.solve(_matrix);

	//¸³Öµ
	for(int row= 0; row< MaxBallNum; row++){
		for(int col= 0; col< MaxBallNum; col++){
			if(_matrix(row,col) != INITNUM){
				if (mixedrecMsg.BallFound[col] && canBeSet(followarray[row].PosX,followarray[row].PosY,
					mixedrecMsg.Ballx[col],mixedrecMsg.Bally[col])){
					/*if(distances(followarray[row].PosX,followarray[row].PosY,
						mixedrecMsg.Ballx[col],mixedrecMsg.Bally[col]) > BALLCALIERROR*BALLCALIERROR){
						int j;
						int cycy = GlobalData::Instance()->smsg.Cycle;
						j = 3;
					}*/
					followarray[row].PosX = mixedrecMsg.Ballx[col];
					followarray[row].PosY = mixedrecMsg.Bally[col];
					followarray[row].Fresh=true;
				}
			}
		}
	}
}

bool BallFollow::ismix(int leftnum,int rightnum)
{
	if (GlobalData::Instance()->receiveMsg[_camera1].BallFound[leftnum]
	   && GlobalData::Instance()->receiveMsg[_camera2].BallFound[rightnum]
	   && distances(GlobalData::Instance()->receiveMsg[_camera1].Ballx[leftnum],
		          GlobalData::Instance()->receiveMsg[_camera1].Bally[leftnum],
		          GlobalData::Instance()->receiveMsg[_camera2].Ballx[rightnum],
		          GlobalData::Instance()->receiveMsg[_camera2].Bally[rightnum])<BALLCALIERROR*BALLCALIERROR)
	{
		return true;
	}
	return false;
}

bool BallFollow::canBeSet(float preX, float preY, float nowX, float nowY)
{
	if(isPosValid(preX, preY) && isPosValid(nowX, nowY)){
		if(distances(preX, preY, nowX, nowY) < BALLCALIERROR*BALLCALIERROR){
			return true;
		} else{
			addFollow(nowX,nowY);
			return false;
		}
	} else if(isPosValid(preX, preY) && !isPosValid(nowX, nowY)){
		return false;
	}
	return true;

}

void BallFollow::addFollow(float x, float y)
{
	for(int i = 0; i<MaxBallNum; i++){
		if(followarray[i].PosX == - ERRORNUM && followarray[i].PosY == - ERRORNUM){
			followarray[i].PosX = x;
			followarray[i].PosY = y;
			followarray[i].Fresh = true;
			break;
		}
	}
}

bool BallFollow::isPosValid(float x, float y)
{
	if( x>-MaxValidLength && x<MaxValidLength && y>-MaxValidWidth && y<MaxValidWidth)
		return true;
	return false;
}

bool BallFollow::thereIsStableOne(int minAddFrame, int now)
{
	for(int i = 0; i < MaxBallNum; i++){
		if(followarray[i].AddFrame == minAddFrame && i != now){
			return true;
		}
	}
	return false;
}
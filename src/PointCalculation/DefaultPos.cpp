#include "DefaultPos.h"
#include "./defence/DefenceInfo.h"
#include "Global.h"

CDefaultPos::CDefaultPos(){
	_defenderList.clear();
	_leftDefList.clear();
	_rightDefList.clear();
}

CDefaultPos::~CDefaultPos(){
}

CGeoPoint CDefaultPos::generatePos(const CVisionModule* pVision){
	double ballPosY = pVision->Ball().Y();
	int attackNum = DefenceInfo::Instance()->getAttackNum();
	int ncnt[3] = {0,0,0};
	for (int i=0; i< attackNum; i++){
		int enemy = DefenceInfo::Instance()->getAttackOppNumByPri(i);
		if (inCourt(-1,enemy)){
			ncnt[0] = ncnt[0]+1;
		}else if (inCourt(0,enemy)){
			ncnt[1] = ncnt[1]+1;
		}else if (inCourt(1,enemy)){
			ncnt[2] = ncnt[2]+1;
		}
	}
	//cout<<"left is "<<ncnt[0]<<" middle is "<<ncnt[1]<<" right is "<<ncnt[2]<<endl;
	updateDefInfo();
	int zerocnt = 0;
	for (int i=0; i< 3;i++){
		if (ncnt[i] ==0){
			zerocnt = zerocnt +1;
		}
	}
	if (zerocnt == 3 && _defenderList.size()!= 0){
		double y = 0;
		for (DefenderList::iterator ir = _defenderList.begin();ir !=_defenderList.end();ir++){
			y = y+(*ir).pos.y();
		}
		_default[2] =CGeoPoint(-40, y/_defenderList.size());
		_default[0] = CGeoPoint(-40,y/_defenderList.size()-50);
		_default[1] = CGeoPoint(-40,y/_defenderList.size()+50);
	}else if (zerocnt == 2 && _defenderList.size()!= 0){
		if (ncnt[0]==0 && ncnt[1] == 0){
			if (_leftDefList.size()!=0 ||_middleDefList.size()!=0){
				double y = 0;
				for (DefenderList::iterator ir = _defenderList.begin();ir != _defenderList.end();ir ++){
					if (find(_leftDefList.begin(),_leftDefList.end(),(*ir).defender)!=_leftDefList.end() || find(_middleDefList.begin(),_middleDefList.end(),(*ir).defender)!= _middleDefList.end()){
						y = y+(*ir).pos.y();
						//cout<<(*ir).defender<<" ";
					}
				}
				//cout<<endl;
				//cout<<_leftDefList.size()<<" "<<_middleDefList.size()<<endl;
				_default[2] =CGeoPoint(-40, y/(_leftDefList.size()+_middleDefList.size()));
				_default[0] = CGeoPoint(-40,y/(_leftDefList.size()+_middleDefList.size())-50);
				_default[1] = CGeoPoint(-40,y/(_leftDefList.size()+_middleDefList.size())+50);
			}else{
				//cout<<"22222222"<<endl;
				_default[0] = CGeoPoint(-40,-100);
				_default[1] = CGeoPoint(-40,0);
				_default[2] = CGeoPoint(-40,-50);
			}
		}else if (ncnt[0] == 0 && ncnt[2] == 0){
			double y = 0;
			for (DefenderList::iterator ir = _defenderList.begin();ir !=_defenderList.end();ir++){
				y = y+(*ir).pos.y();
			}
			_default[2] =CGeoPoint(-40, y/_defenderList.size());
			_default[0] = CGeoPoint(-40,y/_defenderList.size()-50);
			_default[1] = CGeoPoint(-40,y/_defenderList.size()+50);
		}else if (ncnt[2]==0 && ncnt[1] == 0){
			if (_rightDefList.size()!=0 ||_middleDefList.size()!=0){
				double y = 0;
				for (DefenderList::iterator ir = _defenderList.begin();ir != _defenderList.end();ir ++){
					if (find(_middleDefList.begin(),_middleDefList.end(),(*ir).defender)!=_middleDefList.end() || find(_rightDefList.begin(),_rightDefList.end(),(*ir).defender)!= _rightDefList.end()){
						y = y+(*ir).pos.y();
					}
				}
				_default[2] =CGeoPoint(-40, y/(_middleDefList.size()+_rightDefList.size()));
				_default[0] = CGeoPoint(-40,y/(_middleDefList.size()+_rightDefList.size())-50);
				_default[1] = CGeoPoint(-40,y/(_middleDefList.size()+_rightDefList.size())+50);
			}else{
				_default[0] = CGeoPoint(-40,0);
				_default[1] = CGeoPoint(-40,100);
				_default[2] = CGeoPoint(-40,50);
			}
		}
	}else if (zerocnt == 1 && _defenderList.size() !=0){
		if (ncnt[0] == 0){
			if (_leftDefList.size() !=0 ){
				double y = 0;
				for (DefenderList::iterator ir = _defenderList.begin();ir != _defenderList.end();ir ++){
					if (find(_leftDefList.begin(),_leftDefList.end(),(*ir).defender)!=_leftDefList.end()){
						y = y+(*ir).pos.y();
					}
				}
				_default[2] =CGeoPoint(-40, y/_leftDefList.size());
				_default[0] = CGeoPoint(-40,y/_leftDefList.size()-50);
				_default[1] = CGeoPoint(-40,y/_leftDefList.size()+50);
			}else{
				_default[0] = CGeoPoint(-40,-100);
				_default[1] = CGeoPoint(-40,0);
				_default[2] = CGeoPoint(-40,-50);
			}
		}else if (ncnt[1] == 0){
			if (_middleDefList.size() !=0 ){
				double y = 0;
				for (DefenderList::iterator ir = _defenderList.begin();ir != _defenderList.end();ir ++){
					if (find(_middleDefList.begin(),_middleDefList.end(),(*ir).defender)!=_middleDefList.end()){
						y = y+(*ir).pos.y();
					}
				}
				_default[2] =CGeoPoint(-40, y/_middleDefList.size());
				_default[0] = CGeoPoint(-40,y/_middleDefList.size()-50);
				_default[1] = CGeoPoint(-40,y/_middleDefList.size()+50);
			}else{
				_default[0] = CGeoPoint(-40,-50);
				_default[1] = CGeoPoint(-40,50);
				_default[2] = CGeoPoint(-40,0);
			}
		}
	}else if (zerocnt == 0 && _defenderList.size() !=0){
		double y = 0;
		for (DefenderList::iterator ir = _defenderList.begin();ir !=_defenderList.end();ir++){
			y = y+(*ir).pos.y();
		}
		_default[2] =CGeoPoint(-40, y/_defenderList.size());
		_default[0] = CGeoPoint(-40,y/_defenderList.size()-50);
		_default[1] = CGeoPoint(-40,y/_defenderList.size()+50);
	}else{
		_default[0] = CGeoPoint(-40,-50);
		_default[1] = CGeoPoint(-40,50);
		_default[2] = CGeoPoint(-40,0);
	}

	//对60cm开球区域进行检测
	const string refMsg = WorldModel::Instance()->CurrentRefereeMsg();
	if ("theirIndirectKick" == refMsg || "theirDirectKick" == refMsg || "theirKickOff" == refMsg || "gameStop" == refMsg)
	{
		if (_default[2].dist(vision->Ball().Pos()) < 60)
		{
			CGeoCirlce ballCircle = CGeoCirlce(vision->Ball().Pos(),80);
			CGeoLine baseLine = CGeoLine(_default[0],_default[1]);
			CGeoLineCircleIntersection baseIntersect = CGeoLineCircleIntersection(baseLine,ballCircle);
			if (baseIntersect.intersectant())
			{
				CGeoPoint  p1= baseIntersect.point1();
				CGeoPoint p2 = baseIntersect.point2();
				if (p1.dist(_default[2]) < p2.dist(_default[2]))
				{
					_default[2] = p1;
				} else _default[2] = p2;
			}
		}
	}
	GDebugEngine::Instance()->gui_debug_x(_default[0],COLOR_WHITE);
	GDebugEngine::Instance()->gui_debug_x(_default[1],COLOR_WHITE);
	GDebugEngine::Instance()->gui_debug_x(_default[2],COLOR_WHITE);
	return CGeoPoint(0,0);
}

bool CDefaultPos::inCourt(int side, int enemyNum){
	bool result = false;
	if (side == -1){
		if (vision->TheirPlayer(enemyNum).Y() < -Param::Field::PITCH_WIDTH/4){
			result = true;
		}
	}else if (side == 0){
		if (vision->TheirPlayer(enemyNum).Y()>= - Param::Field::PITCH_WIDTH/4 && vision->TheirPlayer(enemyNum).Y()<= Param::Field::PITCH_WIDTH/4){
			result = true;
		}
	}else if (side == 1){
		if (vision->TheirPlayer(enemyNum).Y()>Param::Field::PITCH_WIDTH/4){
			result = true;
		}
	}
	return result;
}

void CDefaultPos::updateDefInfo(){
	_defenderList.clear();
	_leftDefList.clear();
	_rightDefList.clear();
	_middleDefList.clear();

	for(int i=0;i<Param::Field::MAX_PLAYER;i++){
		if (DefenceInfo::Instance()->getOppPlayerByNum(i)->isTheRole("RDefender")){
			Defender tmpDef;
			tmpDef.defender = DefenceInfo::Instance()->getOppPlayerByNum(i)->getNum();
			tmpDef.pos = vision->TheirPlayer(tmpDef.defender).Pos();
			if (inCourt(-1,tmpDef.defender)){
				tmpDef.side = -1;
				_leftDefList.push_back(tmpDef.defender);
			}else if (inCourt(0,tmpDef.defender)){
				tmpDef.side = 0;
				_middleDefList.push_back(tmpDef.defender);
			}else{
				tmpDef.side = 1;
				_rightDefList.push_back(tmpDef.defender);
			}
			_defenderList.push_back(tmpDef);
		}
	}
}

CGeoPoint CDefaultPos::getDefaultPosbyIndex(int index){
	if (vision->Cycle() == _lastCycle){
		return _default[index];
	} else{
		_lastCycle = vision->Cycle();
	}
	generatePos(vision);
	return _default[index];
}
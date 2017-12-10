#include "ContactChecker.h"
#include "VisionModule.h"

namespace{
	double judge_one[2]={1,1};
	double judge_two[2]={-2,-2};
	double judge_three[2]={-2,-2};
	double judge_four[2]={15,15};
}

ContactChecker::ContactChecker()
{
	_lastContactNum=0;
	_lastContactReliable=true;
	_previousNum=0;
	_outFieldCycle=0;
}
bool isPointOutField(CGeoPoint pos)
{
	if (pos.x()<Param::Field::PITCH_LENGTH/2.0&&pos.x()>-Param::Field::PITCH_LENGTH/2.0
		&&pos.y()<Param::Field::PITCH_WIDTH/2.0&&pos.y()>-Param::Field::PITCH_WIDTH/2.0){
		return false;
	}
	else {
		return true;
	}
}
void ContactChecker::OutFieldJudge(const CVisionModule* pVision)
{
	MobileVisionT last_ball=pVision->Ball(pVision->Cycle()-4);
	if (pVision->Ball().Valid()&&isPointOutField(pVision->Ball().Pos())){
		_isBallOutField=true;
	}
	if (!pVision->Ball().Valid()&&last_ball.Valid()&&!isPointOutField(last_ball.Pos()))
	{
		CGeoPoint out_pos=last_ball.Pos()+last_ball.Vel()/15.0;
		if (isPointOutField(out_pos))
		{
			_isBallOutField=true;
		}
	}
	if (_outFieldCycle==0&&_isBallOutField)
	{
		_outFieldCycle=pVision->Cycle();
	}
	/*{
		int counter=0;
		for (int i=0;i<=10;i++)
		{
			if(!isPointOutField(Ball(Cycle()-i).Pos()))
			{
				counter++;
			}
		}
		if (counter>=10&&Ball().Valid()==true&&!isPointOutField(Ball().Pos()))
		{
			_outFieldCycle=0;
		}		
		return false;
	}*/
}
void ContactChecker::prepareJudge(const CVisionModule* pVision)
{
	cycle=pVision->Cycle();
	_ball=pVision->Ball();
	_lastball=pVision->Ball(cycle-5);
	AllPlayer.clear();
	for (int i=1;i<=6;i++) {
		AllPlayer.push_back(pVision->OurPlayer(i));
	}
	for (int i=1;i<=6;i++) {
		AllPlayer.push_back(pVision->TheirPlayer(i));
	}
	_last_contact.frame=0;
	_last_contact.robotnum=0;
	_last_contact.type=0;

	ball_direction_before=pVision->Ball(cycle-3).Vel();
	ball_direction_after=pVision->Ball().Vel();
	speed_before = ball_direction_before.mod();
	speed_after = ball_direction_after.mod();
	speed_diff_percept = (pVision->Ball().Vel()-pVision->Ball(cycle-3).Vel()).mod();
	angle_to = (pVision->Ball().Vel().dir()-pVision->Ball(cycle-3).Vel().dir())/Param::Math::PI*180;

	_isJudged=false;
}

void ContactChecker::judgeOne()
{
	if (true){
		int number=0;
		int color=0;
		for (std::vector<PlayerVisionT>::iterator it = AllPlayer.begin(); it != AllPlayer.end(); it++) {
			number++;
			if (it->Valid()){
				if (number>6){color=1;}
				if (_ball.Pos().dist(it->RawPos()) <=(Param::Vehicle::V2::PLAYER_SIZE + Param::Field::BALL_SIZE+judge_one[color])) {
					_isJudged = true;
					_last_contact.frame = cycle;
					_last_contact.robotnum =number;
					_last_contact.type = 1; 
				}
			}
		}
	}
}

void ContactChecker::judgeTwo()
{
	if (speed_diff_percept > 100 || angle_to > 15 && !_isJudged) {
		int number=0;
		int color=0;
		double last_touched_dist_tmp=30;
		for (std::vector<PlayerVisionT>::iterator it = AllPlayer.begin(); it != AllPlayer.end(); it++) {
			number++;
			if (it->Valid())
			{
				if (number>6){color=1;}
				CGeoCirlce circle (it->RawPos(),Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE+judge_three[color]);
				CGeoPoint p1 = _lastball.Pos();
				CGeoPoint p2 = _lastball.Pos()+(_lastball.Vel());
				CGeoSegment ball_line(p1,p2);
				CGeoSegmentCircleIntersection intersection(ball_line,circle);
				CGeoPoint intersections[2]={intersection.point1(),intersection.point2()};
				if (intersection.intersectant()) {
					//cout<<_lastball.Pos()<<endl;
					//cout<<"insecNum"<<number<<endl;
					for (unsigned int j = 0; j < intersection.size(); ++j) {
						if (_lastball.Pos().dist(intersections[j]) < last_touched_dist_tmp&&_lastball.Pos().dist(it->Pos())<25) {
							last_touched_dist_tmp = _lastball.Pos().dist(intersections[j]);
							_isJudged = true;
							_last_contact.frame = cycle;
							_last_contact.robotnum=number;
							_last_contact.type = 2; 
						}
					}
				}
			}

		}
	}
}

void ContactChecker::judgeThree()
{
	if (speed_diff_percept >100|| angle_to > 15 && !_isJudged) {
		int number=0;
		int color=0;
		double last_touched_dist_tmp=20;
		for (std::vector<PlayerVisionT>::iterator it = AllPlayer.begin(); it != AllPlayer.end(); it++) {
			number++;
			if (it->Valid())
			{
				if (number>6){color=1;}
				CGeoCirlce circle (it->RawPos(),Param::Vehicle::V2::PLAYER_SIZE+Param::Field::BALL_SIZE+judge_three[color]);
				CGeoPoint p1 =_ball.Pos();
				CGeoPoint p2 =_ball.Pos()+(-_ball.Vel());
				CGeoSegment ball_line(p1,p2); 
				CGeoSegmentCircleIntersection intersection(ball_line,circle);
				CGeoPoint intersections[2]={intersection.point1(),intersection.point2()};
				if (intersection.intersectant()) {
					for (int j = 0; j < intersection.size(); ++j) {
						if (_ball.Pos().dist(intersections[j]) < last_touched_dist_tmp&&_ball.Pos().dist(it->Pos())<25) {
							last_touched_dist_tmp = _ball.Pos().dist(intersections[j]);
							_isJudged = true;
							_last_contact.frame = cycle;
							_last_contact.robotnum=number;
							_last_contact.type = 3; 
						}
					}
				}
			}

		}
	}
}
void ContactChecker::judgeFour()
{

	if (speed_diff_percept >200|| angle_to > 20&& _isBallMoving && !_isJudged) {
		int number=0;
		double least_dist=0;
		int color=0;
		for (std::vector<PlayerVisionT>::iterator it = AllPlayer.begin(); it != AllPlayer.end(); it++) {
			number++;
			if (it->Valid()&&_ball.Valid())
			{
				if (least_dist==0)
				{
					least_dist=_ball.Pos().dist(it->Pos());
				}
				else{
					if (_ball.Pos().dist(it->Pos())<least_dist)
					{
						least_dist=_ball.Pos().dist(it->Pos());
						_isJudged = true;
						_last_contact.frame = cycle;
						_last_contact.robotnum=number;
						_last_contact.type = 4;
					}
				}						
			}
		}
		if (_last_contact.robotnum>6){color=1;}
		if (least_dist>judge_four[color])
		{
			_last_contact.robotnum=0;
		}
	}
}
void ContactChecker::judgeFive()
{
	if (speed_after>200&& !_isJudged) {
 		int number=0;
		int color=0;
		double last_touched_dist_tmp=speed_after/15;
		for (std::vector<PlayerVisionT>::iterator it = AllPlayer.begin(); it != AllPlayer.end(); it++) {
			number++;
			if (it->Valid())
			{
				if (number>6){color=1;}
				CGeoCirlce circle (it->RawPos(),Param::Vehicle::V2::PLAYER_SIZE);
				CGeoPoint p1 = _ball.Pos();
				CGeoPoint p2 = _ball.Pos()+(_ball.Vel());
				CGeoSegment ball_line(p1,p2);
				CGeoSegmentCircleIntersection intersection(ball_line,circle);
				CGeoPoint intersections[2]={intersection.point1(),intersection.point2()};
				if (intersection.intersectant()) {
					for (unsigned int j = 0; j < intersection.size(); ++j) {
						if (_ball.Pos().dist(intersections[j]) < last_touched_dist_tmp) {
							last_touched_dist_tmp = _ball.Pos().dist(intersections[j]);
							_isJudged = true;
							_last_contact.frame = cycle;
							_last_contact.robotnum=number;
							_last_contact.type = 5; 
						}
					}
				}
			}

		}
	}
}

void ContactChecker::ballMovingJudge(const CVisionModule* pVision)
{
	_isBallMoving=false;
	{
		int couter=0;
		for(int i=0;i<=5;i++)
		{
			if (pVision->Ball(pVision->Cycle()-i).Valid()&&pVision->Ball(pVision->Cycle()-i).Vel().mod()>10)
			{
				couter++;
			}
		}
		if (couter>=5)
		{
			_isBallMoving=true;
		}
	}
}


void ContactChecker::internalReliableJudge()
{
	if (!_ball.Valid())
	{
		//_lastContactReliable=0;
	}
	//¡Ω∏ˆ÷√–≈∂»≈–∂œ	
	if (_lastContactNum != 0){

		if (_ball.Pos().dist(AllPlayer[_lastContactNum-1].Pos())<7)
		{
			setContactNum(0);
			_lastContactReliable=0;
		}

		//±‹√‚≤¡≈ˆ
		if (speed_after >200&& angle_to<5&&_lastContactNum!=_previousNum){
			if ((_lastContactNum<6&&_previousNum>=6)||(_lastContactNum>=6&&_previousNum<6))
			{
				_lastContactReliable=0;
				setContactNum(0);
			}

		}
		//±‹√‚º∑‘⁄“ª∆
		if (_last_contact.robotnum>=7&&_last_contact.robotnum<=12){
			for (std::vector<PlayerVisionT>::iterator it = AllPlayer.begin(); it <=AllPlayer.begin()+5; it++) {
				if (_ball.Pos().dist(it->Pos())<(Param::Field::BALL_SIZE+Param::Vehicle::V2::PLAYER_SIZE+5))
				{
					setContactNum(0);
					_lastContactReliable=0;break;
				}
			}
		}
		if (_last_contact.robotnum>=1&&_last_contact.robotnum<=6){
			for (std::vector<PlayerVisionT>::iterator it = AllPlayer.begin()+6; it <=AllPlayer.begin()+11; it++) {
				if (_ball.Pos().dist(it->Pos())<(Param::Field::BALL_SIZE+Param::Vehicle::V2::PLAYER_SIZE+5))
				{
					setContactNum(0);
					_lastContactReliable=0;break;
				}
			}
		}

	} 
}

void ContactChecker::ContactCheck(const CVisionModule* pVision)
{
	//cout<<pVision->Cycle()<<" speed"<<_ball.Vel().mod()<<" "<<_ball.Vel().dir()<<endl;
	//cout<<"raw"<<pVision->RawBall().X()<<endl;
	if (_ball.Valid())
	{
		judgeOne();
		judgeTwo();
		judgeThree();
		judgeFour();
		judgeFive();
	}
	//cout<<"last_contact"<<_last_contact.type<<endl;
	//cout<<"last_contact_Num"<<_last_contact.robotnum<<endl;
	_previousNum=_lastContactNum;
	if(_last_contact.robotnum != 0){
		_lastContactNum = _last_contact.robotnum;
		_lastContactReliable=true;
		/*_ballContact.push_back(_last_contact);
		if(_ballContact.size()>20) _ballContact.erase(_ballContact.begin());*/
	}
	internalReliableJudge();
}
void ContactChecker::externReliableJudge()
{	
	//if (_outFieldCycle!=0&&!_ballContact.empty()){   
	//	//±‹√‚¬©≈–
	//	vector<Last_Contact>::iterator it=_ballContact.end()-1;
	//	if (_outFieldCycle - it->frame>120){
	//		_lastContactReliable=0;
	//	}
	//}

}
void ContactChecker::refereeJudge(const CVisionModule* pVision)
{
	prepareJudge(pVision);
	OutFieldJudge(pVision);
	if (pVision->GetCurrentRefereeMsg()!="gameStop"&&pVision->GetCurrentRefereeMsg()!="gameHalt"&&pVision->RawBall().Valid()){
		ContactCheck(pVision);	
	}			
	//if ( pVision->_refereeMsg <2&&pVision->_lastRefereeMsg>=2) {
	//	if (!_isBallOutField)
	//	{
	//		//setContactReliable(false);
	//		//_lastContactReliable=false;
	//	}
	//}
	if ((pVision->GetCurrentRefereeMsg()!="gameStop"||pVision->GetCurrentRefereeMsg()!="gameHalt")
		&&(pVision->GetLastRefereeMsg()=="gameStop"||pVision->GetLastRefereeMsg()=="gameHalt"))
	{
		_outFieldCycle=0;
		_isBallOutField=false;
		setContactNum(0);
		//_lastContactNum=0;
		setContactReliable(true);
	}

	if (getContactReliable()&&getContactNum()!=0){
		_lastContactNum=getContactNum();
	}else{
		_lastContactNum=0;
	}	
}
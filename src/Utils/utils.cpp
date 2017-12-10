#include "utils.h"
#include "WorldModel.h"

namespace Utils{
	double Normalize(double angle)
	{
		if (fabs(angle) > 10 ){
			cout<<angle<<" Normalize Error!!!!!!!!!!!!!!!!!!!!"<<endl;
			return 0;
		}
		const double M_2PI = Param::Math::PI * 2;
		// ���ٴֵ���
		angle -= (int)(angle / M_2PI) * M_2PI; 
		
		// ϸ���� (-PI,PI]
		while( angle > Param::Math::PI ) {
			angle -= M_2PI;
		}

		while( angle <= -Param::Math::PI ) {
			angle += M_2PI;
		}

		return angle;
	}
	CVector Polar2Vector(double m,double angle)
	{
		return CVector(m*std::cos(angle),m*std::sin(angle));
	}
	double VectorDot(const CVector& v1, const CVector& v2)
	{
		return v1.x()*v2.x() + v1.y()*v2.y();
	}
	bool InBetween(const CGeoPoint& p,const CGeoPoint& p1,const CGeoPoint& p2)
	{
		return p.x() >= (std::min)(p1.x(),p2.x()) && p.x() <= (std::max)(p1.x(),p2.x())
			&& p.y() >= (std::min)(p1.y(),p2.y()) && p.y() <= (std::max)(p1.y(),p2.y());
	}
	bool InBetween(double v,double v1,double v2)
	{
		return (v > v1 && v < v2) || (v < v1 && v > v2);
	}
	bool InBetween(const CVector& v, const CVector& v1, const CVector& v2, double buffer)
	{
		
		double d = v.dir(), d1 = v1.dir(), d2 = v2.dir();
		return AngleBetween(d, d1, d2, buffer);
	}

	bool AngleBetween(double d, double d1, double d2, double buffer)
	{
		using namespace Param::Math;
		// d, d1, d2Ϊ����v, v1, v2�ķ��򻡶�

		// ��v��v1��v2�ĽǶ�����С,��buffer����Χ֮��ʱ,��Ϊ��������
		double error = (std::min)(std::fabs(Normalize(d-d1)), std::fabs(Normalize(d-d2)));
		if (error < buffer) {
			return true;
		}

		if (std::fabs(d1 - d2) < PI) {
			// ��ֱ���������ֵС��PIʱ, dӦ�ô���С��,С�ڴ��
			return InBetween(d, d1, d2);
		}
		else {
			// ��Ϊ�����������
			return InBetween(Normalize(d+PI), Normalize(d1+PI), Normalize(d2+PI));
		}
	}

	CGeoPoint MakeInField(const CGeoPoint& p,const double buffer)
	{
		if( std::fabs(p.y()) < Param::Field::GOAL_WIDTH/2 ){
			double y = ( std::fabs(p.y()) > Param::Field::GOAL_WIDTH/2 - buffer ) ? Sign(p.y()) * (Param::Field::GOAL_WIDTH/2 - buffer) : p.y();
			const double fLeft = FieldLeft() - Param::Field::GOAL_DEPTH + buffer;
			if( p.x() < fLeft ){
				return CGeoPoint(fLeft,y);
			}
			const double fRight = FieldRight() + Param::Field::GOAL_DEPTH - buffer;
			if( p.x() > fRight ){
				return CGeoPoint(fRight,y);
			}
			return p;
		}
		CGeoPoint result = p;
		if( result.x() < FieldLeft() + buffer ){
			result = CGeoPoint(FieldLeft() + buffer,result.y());
		}else if( result.x() > FieldRight() - buffer){
			result = CGeoPoint(FieldRight() - buffer,result.y());
		}
		if( result.y() < FieldTop() + buffer){
			result = CGeoPoint(result.x(),FieldTop() + buffer);
		}else if( result.y() > FieldBottom() - buffer){
			result = CGeoPoint(result.x(),FieldBottom() - buffer);
		}
		return result;
	}
	bool OutOfField(const CGeoPoint& p, const double buffer	)	//bufferΪ���ڳ��ڣ����ڳ���
	{
		// 2014-06-26 ע���� �����ϻᵼ�½���������ͳ������� yys
		//if( std::fabs(p.y()) < Param::Field::GOAL_WIDTH/2 ){ // ����������
		//	cout << "here here here" << endl;
		//	return p.x() < FieldLeft() - Param::Field::GOAL_DEPTH/3 
		//		|| p.x() > FieldRight() + Param::Field::GOAL_DEPTH/3;
		//}
		return p.x() < FieldLeft() + buffer || p.x() > FieldRight() - buffer 
			|| p.y() < FieldTop() + buffer || p.y() > FieldBottom() - buffer;
	}

	CGeoPoint MakeOutOfOurPenaltyArea(const CGeoPoint& p,const double buffer)
	{
		// ������2009�Ľ�������
		const CGeoPoint leftC = CGeoPoint(-Param::Field::PITCH_LENGTH/2, -Param::Field::PENALTY_AREA_L/2);
		const CGeoPoint rightC = CGeoPoint(-Param::Field::PITCH_LENGTH/2, Param::Field::PENALTY_AREA_L/2);
		const CGeoPoint cen = CGeoPoint(-Param::Field::PITCH_LENGTH/2, 0);

		if ( InOurPenaltyArea(p, buffer) ){
			if ( abs(p.y()) < Param::Field::PENALTY_AREA_L/2 ){
				return CGeoPoint(-Param::Field::PITCH_LENGTH/2+Param::Field::PENALTY_AREA_R+buffer, p.y());
			}
			else if ( p.y() < -Param::Field::PENALTY_AREA_L/2 ){
				CVector leftC2p = p - leftC;
				return leftC + Utils::Polar2Vector(Param::Field::PENALTY_AREA_R+buffer, leftC2p.dir());
			}
			else{
				CVector rightC2p = p - rightC;
				return rightC + Utils::Polar2Vector(Param::Field::PENALTY_AREA_R+buffer, rightC2p.dir());
			}
		}
		return p;
	}

	CGeoPoint MakeOutOfTheirPenaltyArea(const CGeoPoint& p, const double buffer) {
		CGeoPoint leftC = CGeoPoint(Param::Field::PITCH_LENGTH/2, -Param::Field::PENALTY_AREA_L/2);
		CGeoPoint rightC = CGeoPoint(Param::Field::PITCH_LENGTH/2, Param::Field::PENALTY_AREA_L/2);
		CGeoPoint cen = CGeoPoint(Param::Field::PITCH_LENGTH/2, 0);

		if (InTheirPenaltyArea(p, buffer) == true) {
			if (abs(p.y()) < Param::Field::PENALTY_AREA_L/2) {
				return CGeoPoint(Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_R - buffer, p.y());
			}	else if (p.y() <= -Param::Field::PENALTY_AREA_L/2) {
				CVector leftC2p = p - leftC;
				return leftC + Utils::Polar2Vector(Param::Field::PENALTY_AREA_R + buffer, leftC2p.dir());
			}	else {
				CVector rightC2p = p - rightC;
				return rightC + Utils::Polar2Vector(Param::Field::PENALTY_AREA_R + buffer, rightC2p.dir());
			}
		}
		return p;
	}

	CGeoPoint MakeOutOfCircleAndInField(const CGeoPoint& center,const double radius,const CGeoPoint& p,const double buffer)
	{
		const CVector p2c = p - center;
		const double dist = p2c.mod();
		if( dist > radius + buffer || dist < 0.01 ){ // ����Բ��
			return p;
		}
		CGeoPoint newPos(center + p2c * (radius + buffer) / dist);
		CGeoRectangle fieldRect(FieldLeft() + buffer,FieldTop() + buffer,FieldRight() - buffer,FieldBottom() - buffer);
		if( !fieldRect.HasPoint(newPos) ){ // �ڳ���,ѡ���������Ҳ���Բ�ڵĳ��ڵ�
			CGeoCirlce avoidCircle(center,radius + buffer);
			std::vector< CGeoPoint > intPoints;
			for(int i=0; i<4; ++i){
				CGeoLine fieldLine(fieldRect._point[i % 4],fieldRect._point[(i+1) % 4]);
				CGeoLineCircleIntersection fieldLineCircleInt(fieldLine,avoidCircle);
				if( fieldLineCircleInt.intersectant() ){
					intPoints.push_back(fieldLineCircleInt.point1());
					intPoints.push_back(fieldLineCircleInt.point2());
				}
			}
			double minDist = 1000.0;
			CGeoPoint minPoint = newPos;
			for( unsigned int i=0; i<intPoints.size(); ++i ){
				double cDist = p.dist(intPoints[i]);
				if( cDist < minDist ){
					minDist = cDist;
					minPoint = intPoints[i];
				}
			}
			return minPoint;
		}

		return newPos; // Բ�����p����ĵ�
	}
	bool InOurPenaltyArea(const CGeoPoint& p,const double buffer)
	{
		// ȫ���������ڵĽ�������
		const CGeoPoint leftC = CGeoPoint(-Param::Field::PITCH_LENGTH/2, -Param::Field::PENALTY_AREA_L/2);
		const CGeoPoint rightC = CGeoPoint(-Param::Field::PITCH_LENGTH/2, Param::Field::PENALTY_AREA_L/2);

		if ( std::fabs(p.y()) < Param::Field::PENALTY_AREA_L/2 )
			return (p.x() < -Param::Field::PITCH_LENGTH/2 + Param::Field::PENALTY_AREA_R + buffer) && (p.x() > -Param::Field::PITCH_LENGTH/2 - 1);// ����bug [5/26/2011 zhanfei]
		else if ( p.y() < 0 ){
			return (p.dist(leftC) < Param::Field::PENALTY_AREA_R + buffer) && (p.x() > -Param::Field::PITCH_LENGTH/2 - 1);
		}
		else
			return (p.dist(rightC) < Param::Field::PENALTY_AREA_R + buffer) && (p.x() > -Param::Field::PITCH_LENGTH/2 - 1);
	}

	bool InTheirPenaltyArea(const CGeoPoint& p,const double buffer)
	{
		// ȫ���������ڵĽ�������
		const CGeoPoint leftC = CGeoPoint(Param::Field::PITCH_LENGTH/2, -Param::Field::PENALTY_AREA_L/2);
		const CGeoPoint rightC = CGeoPoint(Param::Field::PITCH_LENGTH/2, Param::Field::PENALTY_AREA_L/2);

		if (abs(p.y()) < Param::Field::PENALTY_AREA_L/2)
			return (p.x() > Param::Field::PITCH_LENGTH/2 - Param::Field::PENALTY_AREA_R - buffer);
		else if ( p.y() < 0 )
			return p.dist(leftC) < Param::Field::PENALTY_AREA_R + buffer;
		else
			return p.dist(rightC) < Param::Field::PENALTY_AREA_R + buffer;
	}

	bool PlayerNumValid(int num)
	{
		if (num>=1 && num<=Param::Field::MAX_PLAYER){
			return true;
		}
		else{
			return false;
		}
	}
	// ����һ���������ϵĵ�, һ������(�Ƕ�), �ҳ�һ���ڽ�������ظ÷����
	// ������߽Ͻ��ĵ�
	CGeoPoint GetOutSidePenaltyPos(double dir, double delta, const CGeoPoint targetPoint)
	{
		//double delta = Param::Field::MAX_PLAYER_SIZE + 1.5;
		CGeoPoint pInter = GetInterPos(dir, targetPoint);
		CGeoPoint pDefend = pInter + Polar2Vector(delta, dir);
		return pDefend;
	}

	CGeoPoint GetOutTheirSidePenaltyPos(double dir, double delta, const CGeoPoint& targetPoint) {
		CGeoPoint pInter = GetTheirInterPos(dir, targetPoint);
		return (pInter + Polar2Vector(delta, dir));
	}

	// GetDefendPos�Ĵ���ϸ��
	// ������ͷ��������ͽ����ߵĽ���
	//���������ڽ�����
	CGeoPoint GetInterPos(double dir, const CGeoPoint targetPoint)
	{
		using namespace Param::Field;
		if (Param::Rule::Version == 2008)
		{
			// ����������Բ��,��Բ����ʾ
			CGeoCirlce c1(CGeoPoint(-PITCH_LENGTH/2,  PENALTY_AREA_L/2), PENALTY_AREA_R); 
			CGeoCirlce c2(CGeoPoint(-PITCH_LENGTH/2, -PENALTY_AREA_L/2), PENALTY_AREA_R);
			CGeoPoint targetPointInstead = targetPoint;
			if (dir >= Param::Math::PI/2 - 5/180*Param::Math::PI && dir <= Param::Math::PI)
        return CGeoPoint(-PITCH_LENGTH/2,PENALTY_AREA_L/2+PENALTY_AREA_R);
			else if (dir <= -Param::Math::PI/2 + 5/180*Param::Math::PI && dir >= -Param::Math::PI)
				return CGeoPoint(-PITCH_LENGTH/2,-PENALTY_AREA_L/2-PENALTY_AREA_R);

			// ��������Բ����ֱ��(pLine),��ֱ������ʾ
			CGeoPoint pend1(-PITCH_LENGTH/2+PENALTY_AREA_R,  PENALTY_AREA_L/2);
			CGeoPoint pend2(-PITCH_LENGTH/2+PENALTY_AREA_R, -PENALTY_AREA_L/2);
			CGeoLine pLine(pend1, pend2);
			// �������ĵ�ͷ���, ��һ��ֱ��
			CGeoLine dirLine(targetPointInstead, dir);

			// ���ֱ�ߺ�c1�Ľ���
			if (targetPoint.y() == c1.Center().y())
			{
				if (dir>=0 && dir<Param::Math::PI/2)
				{
					CGeoPoint p = c1.Center()+Polar2Vector(PENALTY_AREA_R,dir);
					return p;
				}         
			}
			else{
				CGeoLineCircleIntersection dirLine_c1_inter(dirLine, c1);
				if (dirLine_c1_inter.intersectant())
				{
					CGeoPoint p1 = dirLine_c1_inter.point1();
					CGeoPoint p2 = dirLine_c1_inter.point2();
					double dir1 = Normalize((p1-c1.Center()).dir());
					double dir2 = Normalize((p2-c1.Center()).dir());
					if (dir1>=0 && dir1<=Param::Math::PI/2)
					{ 
						return p1;

					} 
					else if (dir2>=0 && dir2<=Param::Math::PI/2)
					{
						return p2;

					}
				}
			}
			
			// ���ֱ�ߺ�c2�Ľ���
			if (targetPoint.y() == c2.Center().y())
			{
				if ( dir<=0 && dir>(-Param::Math::PI/2))
				{
					CGeoPoint p = c2.Center()+Polar2Vector(PENALTY_AREA_R,dir);
					return p;
				}	
			}
			else{
				CGeoLineCircleIntersection dirLine_c2_inter(dirLine, c2);
				if (dirLine_c2_inter.intersectant())
				{
					CGeoPoint p1 = dirLine_c2_inter.point1();
					CGeoPoint p2 = dirLine_c2_inter.point2();
					double dir1 = Normalize((p1-c2.Center()).dir());
					double dir2 = Normalize((p2-c2.Center()).dir());
					if (dir1>=(-Param::Math::PI/2) && dir1<=0)
					{
						return p1;

					} 
					else if (dir2>=(-Param::Math::PI/2) && dir2<=0)
					{
						return p2;

					}
				}
			}
			// ���ֱ�ߺ���������Բ�����߶�pLine�Ľ���
			CGeoLineLineIntersection pline_dirline_inter(pLine, dirLine);
			if (pline_dirline_inter.Intersectant())
			{
				CGeoPoint p = pline_dirline_inter.IntersectPoint();
				if (p.y() <= pend1.y() && p.y() >= pend2.y())
				{
					return p;

				}
			}
			//// ����һ��Ĭ�ϵ�,�����������е�
			std::cout<<"our default pos!!"<<std::endl;
			return CGeoPoint(-PITCH_LENGTH/2+PENALTY_AREA_R, 0);
    }
		else if (Param::Rule::Version == 2004)
		{
			CGeoCirlce penaltyCircle(CGeoPoint(-PITCH_LENGTH/2, 0),PENALTY_AREA_WIDTH/2);
			CGeoLine dirLine(targetPoint,dir);
			CGeoLineCircleIntersection dirLine_penaltyCircle(dirLine,penaltyCircle);
			if (dirLine_penaltyCircle.intersectant())
			{
				CGeoPoint p = Utils::OutOfField(dirLine_penaltyCircle.point1())?dirLine_penaltyCircle.point2():dirLine_penaltyCircle.point1();
				return p;
			}		
		}			
		return CGeoPoint(-PITCH_LENGTH/2+PENALTY_AREA_WIDTH/2, 0);
	}

	CGeoPoint GetTheirInterPos(double dir, const CGeoPoint& targetPoint) {
		using namespace Param::Field;
		// ����������Բ��,��Բ����ʾ
		CGeoCirlce c1(CGeoPoint(PITCH_LENGTH/2, PENALTY_AREA_L/2), PENALTY_AREA_R); 
		CGeoCirlce c2(CGeoPoint(PITCH_LENGTH/2, -PENALTY_AREA_L/2), PENALTY_AREA_R);
		if (dir <= Param::Math::PI/2 + 5/180*Param::Math::PI && dir >= 0)
			return CGeoPoint(PITCH_LENGTH/2, PENALTY_AREA_L/2 + PENALTY_AREA_R);
		else if	 (dir >= -Param::Math::PI - 5/180*Param::Math::PI	&& dir <= 0)
			return CGeoPoint(PITCH_LENGTH/2, -(PENALTY_AREA_L/2 + PENALTY_AREA_R));

		// ��������Բ����ֱ��(pLine),��ֱ������ʾ
		CGeoPoint pend1(PITCH_LENGTH/2 - PENALTY_AREA_R, PENALTY_AREA_L/2);
		CGeoPoint pend2(PITCH_LENGTH/2 - PENALTY_AREA_R, -PENALTY_AREA_L/2);
		CGeoLine pLine(pend1, pend2);
		// �������ĵ�ͷ���, ��һ��ֱ��
		CGeoLine dirLine(targetPoint, dir);

		// ���ֱ�ߺ�c1�Ľ���
		if (abs(targetPoint.y() - c1.Center().y()) < 0.0001) {
			if (dir > Param::Math::PI/2 && dir < Param::Math::PI) {
				CGeoPoint p = c1.Center() + Polar2Vector(PENALTY_AREA_R, dir);
				return p;
			}
		} else {
			CGeoLineCircleIntersection dirLine_c1_inter(dirLine, c1);
			if (dirLine_c1_inter.intersectant()) {
				CGeoPoint p1 = dirLine_c1_inter.point1();
				CGeoPoint p2 = dirLine_c1_inter.point2();
				double dir1 = Normalize((p1-c1.Center()).dir());
				double dir2 = Normalize((p2-c1.Center()).dir());
				if (dir1 >= Param::Math::PI / 2 && dir1 <= Param::Math::PI)
					return p1;
				else if (dir2 >= Param::Math::PI / 2 && dir2 <= Param::Math::PI)
					return p2;
			}
		}

		// ���ֱ�ߺ�c2�Ľ���
		if (abs(targetPoint.y() - c2.Center().y()) < 0.0001) {
			if (dir < -Param::Math::PI/2 && dir > -Param::Math::PI) {
				CGeoPoint p = c2.Center() + Polar2Vector(PENALTY_AREA_R, dir);
				return p;
			}
		} else {
			CGeoLineCircleIntersection dirLine_c2_inter(dirLine, c2);
			if (dirLine_c2_inter.intersectant()) {
				CGeoPoint p1 = dirLine_c2_inter.point1();
				CGeoPoint p2 = dirLine_c2_inter.point2();
				double dir1 = Normalize((p1-c2.Center()).dir());
				double dir2 = Normalize((p2-c2.Center()).dir());
				if (dir1 <= -Param::Math::PI/2 && dir1 >= -Param::Math::PI)
					return p1;
				else if (dir2 <= -Param::Math::PI/2 && dir2 >= -Param::Math::PI)
					return p2;
			}
		}

		// ���ֱ�ߺ���������Բ�����߶�pLine�Ľ���
		CGeoLineLineIntersection pline_dirline_inter(pLine, dirLine);
		if (pline_dirline_inter.Intersectant()) {
			CGeoPoint p = pline_dirline_inter.IntersectPoint();
			if (p.y() <= pend1.y() && p.y() >= pend2.y())
				return p;
		}
		//// ����һ��Ĭ�ϵ�,�����������е�
		std::cout<<"our default pos!!"<<std::endl;
		return CGeoPoint(PITCH_LENGTH / 2 - PENALTY_AREA_R, 0);
	}

	float SquareRootFloat(float number) {
		long i;
		float x, y;
		const float f = 1.5F;

		x = number * 0.5F;
		y  = number;
		i  = * ( long * ) &y;
		i  = 0x5f3759df - ( i >> 1 );
		y  = * ( float * ) &i;
		y  = y * ( f - ( x * y * y ) );
		y  = y * ( f - ( x * y * y ) );
		return number * y;
	}
	bool canGo(const CVisionModule* pVision, const int vecNumber, const CGeoPoint& target, const int flags, const double avoidBuffer)//�ж��Ƿ����ֱ�ӵ���Ŀ���
	{
		static bool _canGo = true;
		const CGeoPoint& vecPos = pVision->OurPlayer(vecNumber).Pos();
		CGeoSegment moving_seg(vecPos, target);
		const double minBlockDist2 = (Param::Field::MAX_PLAYER_SIZE/2 + avoidBuffer) * (Param::Field::MAX_PLAYER_SIZE/2 + avoidBuffer);
		for( int i=1; i<=Param::Field::MAX_PLAYER * 2; ++i ){ // ��·������û����
			if( i == vecNumber || !pVision->AllPlayer(i).Valid()){
				continue;
			}
			const CGeoPoint& obs_pos = pVision->AllPlayer(i).Pos();
			if( (obs_pos - target).mod2() < minBlockDist2 ){
				_canGo = false;
				return _canGo;
			}
			CGeoPoint prj_point = moving_seg.projection(obs_pos);
			if( moving_seg.IsPointOnLineOnSegment(prj_point) ){
				const double blockedDist2 = (obs_pos - prj_point).mod2();
				if( blockedDist2 < minBlockDist2 && blockedDist2 < (obs_pos - vecPos).mod2()){
					_canGo = false;
					return _canGo;
				}
			}
		}
		if( _canGo && (flags & PlayerStatus::DODGE_BALL) ){ // �����
			const CGeoPoint& obs_pos = pVision->Ball().Pos();
			CGeoPoint prj_point = moving_seg.projection(obs_pos);
			if( obs_pos.dist(prj_point) < avoidBuffer + Param::Field::BALL_SIZE && moving_seg.IsPointOnLineOnSegment(prj_point) ){
				_canGo = false;
				return _canGo;
			}
		}
		if( _canGo && (flags & PlayerStatus::DODGE_OUR_DEFENSE_BOX) ){ // ������뱾������
			if( Param::Rule::Version == 2003 ){	// 2003��Ĺ�������Ǿ���
				CGeoRectangle defenseBox(-Param::Field::PITCH_LENGTH/2, -Param::Field::PENALTY_AREA_WIDTH/2 - avoidBuffer, -Param::Field::PITCH_LENGTH/2 + Param::Field::PENALTY_AREA_WIDTH + avoidBuffer, Param::Field::PENALTY_AREA_WIDTH/2 + avoidBuffer);
				CGeoLineRectangleIntersection intersection(moving_seg, defenseBox);
				if( intersection.intersectant() ){
					if( moving_seg.IsPointOnLineOnSegment(intersection.point1()) || moving_seg.IsPointOnLineOnSegment(intersection.point2())){
						_canGo = false; // Ҫ��������
						return _canGo;
					}
				}
			}else{ // 2004��Ĺ�������ǰ�Բ��
				CGeoCirlce defenseBox(CGeoPoint(-Param::Field::PITCH_LENGTH/2, 0), Param::Field::PENALTY_AREA_WIDTH/2 + avoidBuffer);
				CGeoLineCircleIntersection intersection(moving_seg, defenseBox);
				if( intersection.intersectant() ){
					if( moving_seg.IsPointOnLineOnSegment(intersection.point1()) || moving_seg.IsPointOnLineOnSegment(intersection.point2())){
						_canGo = false; // Ҫ��������
						return _canGo;
					}
				}
			}
		}
		return _canGo;
	}
}


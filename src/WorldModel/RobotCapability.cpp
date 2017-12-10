#include "RobotCapability.h"
#include <tinyxml/ParamReader.h>

namespace{
	class OmniAuto{
	protected:
		OmniAuto()
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
				READ_PARAM(MAX_ROTATION_DEC)
			DECLARE_PARAM_READER_END
		}

		double maxSpeedGoalie(const double angle) const { return MAX_TRANSLATION_SPEED_GOALIE; }
		double maxAccelerationGoalie(const CVector& velocity, const double angle) const { return MAX_TRANSLATION_ACC_GOALIE; }
		double maxDecGoalie(const CVector& velocity, const double angle) const { return MAX_TRANSLATION_DEC_GOALIE; }

		double maxSpeedBack(const double angle) const { return MAX_TRANSLATION_SPEED_BACK; }
		double maxAccelerationBack(const CVector& velocity, const double angle) const { return MAX_TRANSLATION_ACC_BACK; }
		double maxDecBack(const CVector& velocity, const double angle) const { return MAX_TRANSLATION_DEC_BACK; }

		double maxSpeed(const double angle) const { return MAX_TRANSLATION_SPEED; }
		double maxAcceleration(const CVector& velocity, const double angle) const { return MAX_TRANSLATION_ACC; }
		double maxDec(const CVector& velocity, const double angle) const { return MAX_TRANSLATION_DEC; }

		double maxAngularSpeed() const { return MAX_ROTATION_SPEED; }
		double maxAngularAcceleration() const { return MAX_ROTATION_ACC; }
		double maxAngularDec() const { return MAX_ROTATION_DEC; }
		
	private:
		double MAX_TRANSLATION_SPEED;		//����ٶ�
		double MAX_TRANSLATION_ACC;	//�����ٶ�
		double MAX_TRANSLATION_DEC;	//�����ٶ�
		double MAX_ROTATION_SPEED;			//�����ٶ�
		double MAX_ROTATION_ACC;	//���Ǽ��ٶ�
		double MAX_ROTATION_DEC;	//���Ǽ��ٶ�

		/// ����Աר��
		double MAX_TRANSLATION_SPEED_GOALIE;
		double MAX_TRANSLATION_ACC_GOALIE;
		double MAX_TRANSLATION_DEC_GOALIE;

		/// ����ר��
		double MAX_TRANSLATION_SPEED_BACK;
		double MAX_TRANSLATION_ACC_BACK;
		double MAX_TRANSLATION_DEC_BACK;
	};
	
	// ���������
	class ChipKick{ // ������װ��
	protected:
		ChipKick() : ChipKickEnabled(true), MinChipKickObstacleDist(10), MaxChipKickDist(300), MaxChipShootDist(180), 
						MinChipShootExtra(20)
		{}
		double minChipKickObstacleDist() const { return ChipKickEnabled ? MinChipKickObstacleDist : 100000; }
		double maxChipKickDist() const { return MaxChipKickDist; }
		double maxChipShootDist() const { return MaxChipShootDist; }
		double minChipShootExtra(const double dist) const { return MinChipShootExtra; }
	private:
		bool ChipKickEnabled; // �Ƿ���ChipKick
		double MinChipKickObstacleDist;
		double MaxChipKickDist;
		double MaxChipShootDist;
		double MinChipShootExtra;
	};
	
	// ���������
	class KickAuto{
	protected:
		KickAuto() : MinShootAngleWidth(Param::Field::BALL_SIZE*1.5)
		{}
		double minShootAngleWidth() const { return MinShootAngleWidth; }
	private:
		double MinShootAngleWidth;
	};

	template < class MovingCap, class KickCap, class ChipKickCap>
	class ConcretRobotCapability : public CRobotCapability, MovingCap, KickCap, ChipKickCap{
	public:
		virtual double minShootAngleWidth() const { return KickCap::minShootAngleWidth(); }

		virtual double minChipKickObstacleDist() const { return ChipKickCap::minChipKickObstacleDist(); }
		virtual double maxChipKickDist() const { return ChipKickCap::maxChipKickDist(); }
		virtual double maxChipShootDist() const { return ChipKickCap::maxChipShootDist(); }
		virtual double minChipShootExtra(const double dist) const { return ChipKickCap::minChipShootExtra(dist); }
		
		virtual double maxAccelerationGoalie(const CVector& velocity, const double angle) const { return MovingCap::maxAccelerationGoalie(velocity, angle); }
		virtual double maxDecGoalie(const CVector& velocity, const double angle) const { return MovingCap::maxDecGoalie(velocity, angle);}
		virtual double maxSpeedGoalie(const double angle) const { return MovingCap::maxSpeedGoalie(angle); }

		virtual double maxAccelerationBack(const CVector& velocity, const double angle) const { return MovingCap::maxAccelerationBack(velocity, angle); }
		virtual double maxDecBack(const CVector& velocity, const double angle) const { return MovingCap::maxDecBack(velocity, angle);}
		virtual double maxSpeedBack(const double angle) const { return MovingCap::maxSpeedBack(angle); }

		virtual double maxAcceleration(const CVector& velocity, const double angle) const { return MovingCap::maxAcceleration(velocity, angle); }
		virtual double maxDec(const CVector& velocity, const double angle) const { return MovingCap::maxDec(velocity, angle);}
		virtual double maxSpeed(const double angle) const { return MovingCap::maxSpeed(angle); }

		virtual double maxAngularAcceleration() const { return MovingCap::maxAngularAcceleration(); }
		virtual double maxAngularDec() const { return MovingCap::maxAngularDec();}
		virtual double maxAngularSpeed() const { return MovingCap::maxAngularSpeed(); }
	};
}
CRobotCapFactory::CRobotCapFactory()
{
	// ��ʼ��ÿ������������ע��,ÿ�α���ǰҪ���˴�! ���Ƿ���ȷ
	typedef ConcretRobotCapability< OmniAuto, KickAuto, ChipKick > VehicleAuto;
	
	_robotCaps.push_back(new VehicleAuto); // ����0

	// С��������, 0��ʾ10ϵ��, 1��ʾ9ϵ��
	for( int i=0; i<=Param::Field::MAX_PLAYER; ++i ){
		LeftRobotType[i] = 0;
		RightRobotType[i] = 0;
	}
}


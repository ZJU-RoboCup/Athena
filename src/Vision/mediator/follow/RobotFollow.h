#ifndef ROBOT_FOLLOW
#define ROBOT_FOLLOW

#include "follow.h"
#include "matrix.h"

class RobotFollow : public Follow
{
public:
	RobotFollow(void);
	RobotFollow(int,int,int);
	~RobotFollow(void);
	virtual bool determinesend(int minAddFrame, int minLostFrame, bool isSendFalse);
	void startFollowSingle(int court, int minAddFrame, int minLostFrame, float maxdist);
	void startFollowDouble(int minAddFrame, int minLostFrame, float maxdist);
	void initSmsg();
	void compare(int minAddFrame);
	bool isPosValid(float x, float y);
	void Usort();
	bool isRecvValid(int court, int num);
	bool ismix(int leftnum,int rightnum);
	bool canBeSet(ReceiveVisionMessage, int row, int col);
	void addFollow(float x, float y, float dir, int index);
public:
	int _side;
	int _camera1,_camera2;
	Matrix<double> _matrix;
	int existNum[SendCarNum];
};

#endif

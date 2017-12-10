#ifndef BALL_FOLLOW
#define BALL_FOLLOW

#include "follow.h"
#include "matrix.h"

class BallFollow :
	public Follow
{
public:
	BallFollow(){};
	BallFollow(int,int);
	~BallFollow(void);
	virtual bool checkSend();
	virtual bool determinesend(int minAddFrame, int minLostFrame, bool isSendFalse);
	void startFollowSingle(int court, int minAddFrame, int minLostFrame, float maxdist);
	void startFollowDouble( int minAddFrame, int minLostFrame, float maxdist);
	void initSmsg();
	void compare(int minAddFrame);
	bool ismix(int leftnum,int rightnum);
	bool canBeSet(float preX, float preY, float nowX, float nowY);
	bool isPosValid(float x, float y);
	bool thereIsStableOne(int, int);
	void addFollow(float x, float y);
private:
	Matrix<double> _matrix;
	int _camera1,_camera2;
};

#endif

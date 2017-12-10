#ifndef _MARKING_H_
#define _MARKING_H_
#include <skill/PlayerTask.h>

/************************************************************************/
/*∂¢»À																    */
/*wq																	*/
/************************************************************************/
class CMarking : public CStatedTask{
public:
	CMarking();
	virtual void plan(const CVisionModule* pVision);
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	virtual bool isEmpty()const { return false; }
protected:
	virtual void toStream(std::ostream& os) const { os << "Marking Defense"; }

private:
	int _lastCycle;
	int enemyNum;

private:
	bool needToUpdateTheirKicker;
	bool isMeCloserToBall;
	bool isTheirKickerPosSafe;
	bool isBallToEnemy;
};
#endif //_MARKING_H_
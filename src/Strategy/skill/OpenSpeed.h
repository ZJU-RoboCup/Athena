#ifndef Open_Speed_h__
#define Open_Speed_h__

#include <skill/PlayerTask.h>
#include <vector>
/************************************************************************/
/*                     COpenSpeed  ֱ�ӷ������ٶ�                         */
/************************************************************************/
class COpenSpeed : public CPlayerTask {
public:
	COpenSpeed();
	virtual void plan(const CVisionModule* pVision) {}
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }
};
#endif // Speed_h__
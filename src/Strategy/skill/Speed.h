#ifndef Speed_h__
#define Speed_h__

#include <skill/PlayerTask.h>
/************************************************************************/
/*                     CSpeed  ֱ�ӷ��ٶ�ָ��,ƽ����ת��          */
/*			last edited by zc(eshock@zju.edu.cn) 2008-12-28				*/
/************************************************************************/
class CSpeed : public CPlayerTask{
public:
	CSpeed();
	virtual void plan(const CVisionModule* pVision);
	virtual CPlayerCommand* execute(const CVisionModule* pVision);
	virtual bool isEmpty() const { return false; }
protected:
};
#endif // Speed_h__
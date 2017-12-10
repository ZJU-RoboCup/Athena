#include "PlayerTask.h"
#include "TaskMediator.h"
/************************************************************************/
/*                 PlayerTask                                           */
/************************************************************************/
// �麯����ȱʡʵ��
void CPlayerTask::plan(const CVisionModule* pVision) ///<�滮
{ 
	if( subTask() ){
		subTask()->plan(pVision);
	}
} 
CPlayerCommand* CPlayerTask::execute(const CVisionModule* pVision) ///<ִ��
{ 
	if( subTask() ){
		return subTask()->execute(pVision);
	}
	return 0;
} 

bool CPlayerTask::isEmpty() const
{
	// �ú������ڱ��: ��û������subTaskʱ, isEmpty������ʾ�ϲ�skill�Ƿ�ֱ�Ӹ�ָ��directCommand.
	// ��ֱ�Ӹ�ָ��,������isEmpty����,������false; ���򲻱�����;
	if( subTask() ){
		return subTask()->isEmpty();
	}
	return true;
}

/************************************************************************/
/*                       CStatedTask                                    */
/************************************************************************/
void CStatedTask::setState(int state)
{
	const int vecNumber = task().executor;
	_state = state;
}
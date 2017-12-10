#ifndef _PLAYER_TASK_H_
#define _PLAYER_TASK_H_
/**
* ��Ա��������.
* ������ʾ��Ա�ĸ������񣬰������򣬴�����λ���������أ���λ��
* ������ִ�н���ǲ�3ϵ�л�������
*/
#include <PlayerCommand.h>
#include <misc_types.h>
#include <string>
/************************************************************************/
/*                         CPlayerTask                                  */
/************************************************************************/
class CVisionModule; // ����ֻ�õ�CVisionModule��ָ��,����include��ͷ�ļ�,��ֹ��������
class CPlayerTask{
public:
	CPlayerTask() : _pSubTask(0){ }
	virtual ~CPlayerTask() { }
	virtual void plan(const CVisionModule* pVision); ///<�滮
	virtual CPlayerCommand* execute(const CVisionModule* pVision); ///<ִ��
	virtual bool isEmpty() const; // �Ƿ������
	friend std::ostream& operator << (std::ostream& os, const CPlayerTask& task)
	{
		task.toStream(os);
		if( task.subTask() ){
			os << "-->" << *(task.subTask());
		}
		return os;
	}
	virtual void reset(const TaskT& task) { _task = task; }
protected:
	virtual void toStream(std::ostream& os) const { os << "Invalid task"; } ///<����ַ�����ʽ��Ϣ
	void setSubTask(CPlayerTask* pTask){ _pSubTask = pTask; } // ����������
	CPlayerTask* subTask() const{ return _pSubTask; }
	const TaskT& task() const { return _task; }
	TaskT _task; //������ľ�������
private:	
	CPlayerTask* _pSubTask; // ������
};
/************************************************************************/
/*                         StatedTask                                   */
/************************************************************************/
class CStatedTask : public CPlayerTask{ // ��״̬��task,ά��һ��״̬��
public:	
	CStatedTask() : _state(BEGINNING){ }
	static const int BEGINNING = 0; // ��ʼ״̬
	static const int ABORT = 98; //skillִ��ʧ������
	static const int ABOUT_TO_FINISH = 99;//�����ţ���δ���added by shizhy 2009.1.9
	static const int FINISHED = 100; // ����״̬
	int state() const { return _state; }
	void setState(int state);
	bool isAborted() const { return _state == ABORT; }
	bool isFinished() const { return _state == FINISHED; }
	int getState() const { return _state; }
private:
	int _state; // ��ǰ״̬  
};
#endif // _PLAYER_TASK_H_

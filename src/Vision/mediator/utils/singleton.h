#ifndef _SINGLETON_H_
#define _SINGLETON_H_
/************************************************************************/
/*            Meyers Singleton, �˳�ʱ�ͷ���Դ                          */
/************************************************************************/
template <class SingletonClass >
class MeyersSingleton{
public:
	static SingletonClass * Instance(){
		static SingletonClass instance;
		return &instance;
	}
	SingletonClass* operator ->() { return Instance(); }
	const SingletonClass* operator ->() const { return Instance(); }
private:
	MeyersSingleton(){ }
	~MeyersSingleton(){ }
};
/************************************************************************/
/*              ��ͨ��Singleton, �˳�ʱ���ͷ���Դ                       */
/************************************************************************/
template <class SingletonClass >
class NormalSingleton{
public:
	static SingletonClass * Instance(){
		static SingletonClass* instance = 0;
		if( !instance ){
			instance = new SingletonClass;
		}
		return instance;
	}
	SingletonClass* operator ->() { return Instance(); }
	const SingletonClass* operator ->() const { return Instance(); }
private:
	NormalSingleton(){ }
	~NormalSingleton(){ }
};
#endif

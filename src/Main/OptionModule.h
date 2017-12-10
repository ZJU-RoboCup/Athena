#ifndef _OPTION_MODULE_H_
#define _OPTION_MODULE_H_
/**
* COptionModule.
* һЩ��ʼ������
*/
class COptionModule{
public:
	COptionModule(int argc, char* argv[]);
	~COptionModule();
	int MySide() const { return _side; }
	int MyNumber() const { return _number; }
	int MyType() const { return _type; }
	int MyColor() const { return _color; }
protected:
	void ReadOption(int argc, char* argv[]);
private:
	int _side; // ���������ڵı�
	int _number; // �����˺��룬����һ����Ա��ʱ���õ�
	int _type; // ����������
	int _color; // �ҷ���ɫ,RefereeBox�õ�
};
#endif

#include <MyThread.h>
#include<iostream>
using namespace std;
int main(int argc, char *argv[]) {

	MyThread mt;
	mt.setBasicConfig();
	switch (mt.openClient()) {
	case 0:
		cout << "Error! ���ܴ�UDP�˿�" << endl;
		break;
	case 1:
		cout << "Error! ���ܽ���UDP�ಥ" << endl;
		break;
	default:
		break;
	}
	mt.setFirstStart();
	cout << "Start Transmitting" << endl;
	mt.run();
	return 0;
}
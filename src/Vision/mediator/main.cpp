#include <MyThread.h>
#include<iostream>
using namespace std;
int main(int argc, char *argv[]) {

	MyThread mt;
	mt.setBasicConfig();
	switch (mt.openClient()) {
	case 0:
		cout << "Error! 不能打开UDP端口" << endl;
		break;
	case 1:
		cout << "Error! 不能建立UDP多播" << endl;
		break;
	default:
		break;
	}
	mt.setFirstStart();
	cout << "Start Transmitting" << endl;
	mt.run();
	return 0;
}
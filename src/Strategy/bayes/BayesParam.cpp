//////////////////////////////////////////////////////////////////////////
/*
存储贝叶斯先验39个参数的类
huangqiangsheng9@gmail.com

不足之处在于
1. 不具备通用性, 应该抽象成通用模块实现
2. 缺少数据边界检查
by cliffyin
*/
//////////////////////////////////////////////////////////////////////////

#include "BayesParam.h"

//////////////////////////////////////////////////////////////////////////
// define some helper functions
string getstate(int i)
{
	switch (i)
	{
	case 0:	return "ATTACK";break;
	case 1: return "DEADLOCK";break;
	case 2: return "DEFEND";break;
	}
	return "UNKNOW";
}

// copy constructor
CBayesParam::CBayesParam(const CBayesParam& bayesparam)
{
	for (int iter1 = 0 ; iter1 <3; iter1 ++)
	{
		Bel[iter1] = bayesparam.GetInit(iter1);
		for (int iter2 = 0; iter2 <3; iter2 ++)
		{
				Pzx[iter1][iter2] = bayesparam.GetPzx(iter1,iter2);
			for (int iter3 =0; iter3 <3; iter3 ++)
			{
				Pxux[iter1][iter2][iter3]=bayesparam.GetPxux(iter1,iter2,iter3);
			}
		}
	}
}

// get state transition probability (3*3*3)
double CBayesParam::GetPxux(int last_x, int u, int x) const 
{
	return Pxux[last_x][u][x];
}

// get measurement probability (3*3)
double CBayesParam::GetPzx(int z, int x) const 
{
	return Pzx[z][x];
}

// get initial belief (3*1)
double CBayesParam::GetInit(int x) const 
{
	return Bel[x];
}

// set state transition probability (3*3*3)
void CBayesParam::InPxux(int last_x, int u, int x, double val)
{
	Pxux[last_x][u][x] = val; 
}

// set measurement probability (3*3)
void CBayesParam::InPzx(int z, int x, double val)
{
	Pzx[z][x] = val;
}

// set initial belief (3*1)
void CBayesParam::InInit(int x, double val)
{
	Bel[x] = val;
}

// debug print (optional)
void CBayesParam::printDebug()
{
	cout <<"PXUX"<<endl;
	for (int last_x =0 ; last_x < 3; last_x ++)
	{	
		for (int u = 0; u < 3; u ++)
		{
			cout<<getstate(last_x)<<" "<<getstate(u)<<endl;
			for (int x = 0; x <3; x ++)
			{
				printf("%.2f ",Pxux[last_x][u][x]);
			}
			cout << endl;
		}
	}
	cout <<"PZX"<<endl;
	for (int z = 0; z <3; z++)
	{
		cout<<getstate(z)<<endl;
		for (int x = 0; x < 3; x++)
		{
			printf("%.2f ",Pzx[z][x]);
		}
		cout<<endl;
	}
	cout <<"Init"<<endl;
	for (int x = 0; x <3; x++)
	{
		cout<<getstate(x)<<endl;
		printf("%.2f\n",Bel[x]);
	}

	return ;
}
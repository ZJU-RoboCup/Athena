#ifndef DEALROBOT_H
#define DEALROBOT_H
#include <singleton.hpp>
#include <messageformat.h>

class CDealrobot
{
public:
    CDealrobot();
    void run (bool);
    ReceiveVisionMessage mergeRobot(ReceiveVisionMessage,ReceiveVisionMessage);//
    void selectRobot();//

private:

};
typedef Singleton <CDealrobot> Dealrobot;

#endif // DEALROBOT_H

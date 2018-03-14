#ifndef DEALROBOT_H
#define DEALROBOT_H
#include <singleton.hpp>

class CDealrobot
{
public:
    CDealrobot();
    void run (bool);
};
typedef Singleton <CDealrobot> Dealrobot;

#endif // DEALROBOT_H

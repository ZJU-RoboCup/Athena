#ifndef MAINTAIN_H
#define MAINTAIN_H
#include "singleton.hpp"
#include <messageformat.h>

class CMaintain
{
public:
    CMaintain();
    void run (bool);
    void init();
    void CollisionDetection();
    void ChipPrediction();
    void StateMachine();

private:
    ReceiveVisionMessage result;

};
typedef Singleton<CMaintain> Maintain;

#endif // MAINTAIN_H

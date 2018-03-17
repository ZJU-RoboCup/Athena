#ifndef MAINTAIN_H
#define MAINTAIN_H
#include "singleton.hpp"

class CMaintain
{
public:
    CMaintain();
    void CollisionDetection();
    void ChipPrediction();
    void StateMachine();

};
typedef Singleton<CMaintain> Maintain;

#endif // MAINTAIN_H

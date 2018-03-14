#ifndef DEALBALL_H
#define DEALBALL_H

#include <singleton.hpp>


class CDealball
{
public:
    CDealball();
    void run(bool);
};
typedef Singleton<CDealball> Dealball;
#endif // DEALBALL_H

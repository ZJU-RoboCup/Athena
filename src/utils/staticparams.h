#ifndef STATICPARAMS_H
#define STATICPARAMS_H
//change param for 2018 rule
//wait for change camera number
#include <QString>
namespace PARAM{
    const bool DEBUG=false;
    const int CAMERA = 8;//4
    const int ROBOTNUM = 12;// max allow robot on the playgroung
    const int ROBOTMAXID = 12;
    const int BALLNUM = 20;
    const int BLUE=0;
    const int YELLOW=1;
    const int BALLMERGEDISTANCE=140;
    const int ROBOTMERGEDOSTANCE=500;
}
namespace ZSS{
namespace Jupyter {
    const QString UDP_ADDRESS = "233.233.233.233";
    const int UDP_SEND_PORT = 10001;
    const int UDP_RECEIVE_PORT = 10002;
}
}
#endif // STATICPARAMS_H

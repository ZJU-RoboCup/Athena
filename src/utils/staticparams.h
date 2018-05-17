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
    const int TEAMS = 2;
}
namespace ZSS{
    const QString ZSS_ADDRESS = "233.233.233.233";
    const QString SSL_ADDRESS = "224.5.23.2";
    const QString REF_ADDRESS = "224.5.23.1";
    const QString LOCAL_ADDRESS = "127.0.0.1";
namespace Jupyter {
    const int CONTROL_BACK_SEND = 10002;
    const int CONTROL_RECEIVE = 10001;
}
namespace Medusa {
    const int DEBUG_MSG_SEND = 20001;
}
namespace Sim{
    const int SIM_RECEIVE = 30001;
}
namespace Athena{
    const int CONTROL_SEND = 10001;
    const int CONTROL_BACK_RECEIVE = 10002;
    const int DEBUG_MSG_RECEIVE = 20001;
    const int SIM_SEND = 20011;
}
}
#endif // STATICPARAMS_H

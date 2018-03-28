#include "globaldata.h"
CGlobalData::CGlobalData(){
    montageMaxX = 450;
    montageMaxY = 300;
    maxLostFrame = 30;
    //camera ID can change from
    cameraMatrix[0].id=0;//0;
    cameraMatrix[1].id=1;//1;
    cameraMatrix[2].id=2;//2;
    cameraMatrix[3].id=3;//3;
    cameraMatrix[4].id=4;
    cameraMatrix[5].id=5;
    cameraMatrix[6].id=6;
    cameraMatrix[7].id=7;
    //
    cameraMatrix[0].pos.fill(-450,225);
    cameraMatrix[4].pos.fill(-450,-225);
    cameraMatrix[1].pos.fill(-150,225);
    cameraMatrix[5].pos.fill(-150,-225);
    cameraMatrix[2].pos.fill(150,225);
    cameraMatrix[6].pos.fill(-150,-225);
    cameraMatrix[3].pos.fill(450,225);
    cameraMatrix[7].pos.fill(450,-225);
}

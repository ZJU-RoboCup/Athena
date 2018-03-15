#include "globaldata.h"
CGlobalData::CGlobalData(){
    montageMaxX = 450;
    montageMaxY = 350;
    maxLostFrame = 30;
    //camera ID can change from
    cameraMatrix[0].id=0;
    cameraMatrix[1].id=1;
    cameraMatrix[2].id=2;
    cameraMatrix[3].id=3;
    // cameraMatrix[4].id=4;
    // cameraMatrix[5].id=5;
    // cameraMatrix[6].id=6;
    // cameraMatrix[7].id=7;
}

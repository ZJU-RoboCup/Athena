#include "globaldata.h"
#include "parammanager.h"
namespace  {
    auto vpm =ZSS::VParamManager::instance();
    auto zpm =ZSS::ZParamManager::instance();
}
CGlobalData::CGlobalData():ctrlC(false){

    //camera ID can change from
    cameraMatrix[0].id=0;//0;
    cameraMatrix[1].id=1;//1;
    cameraMatrix[2].id=2;//2;
    cameraMatrix[3].id=3;//3;
    cameraMatrix[4].id=4;
    cameraMatrix[5].id=5;
    cameraMatrix[6].id=6;
    cameraMatrix[7].id=7;
}
CGeoPoint CGlobalData::saoConvert(CGeoPoint originPoint){
    CGeoPoint result;
    switch (saoAction){
    case 0:
        result.setX(originPoint.x());
        result.setY(originPoint.y());
        break;
    case 1:
        result.setX(originPoint.y()+3000);
        result.setY(-originPoint.x());
        break;
    case 2:
        result.setX(originPoint.x() * 3 / 2);
        result.setY(originPoint.y() * 3 / 2);
        break;
    default:
        result.setX(originPoint.x());
        result.setY(originPoint.y());
        break;
    }
    return result;
}

void CGlobalData::saoConvertEdge(){
    switch (saoAction){
    case 0:
        for (int i=0; i<PARAM::CAMERA;i++){
            vpm->loadParam(cameraMatrix[i].leftedge.min,    "Camera"+QString::number(i)+"Leftmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].leftedge.max,    "Camera"+QString::number(i)+"Leftmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].rightedge.min,   "Camera"+QString::number(i)+"Rightmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].rightedge.max,   "Camera"+QString::number(i)+"Rightmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].upedge.min,      "Camera"+QString::number(i)+"Upmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].upedge.max,      "Camera"+QString::number(i)+"Upmax",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].downedge.min,    "Camera"+QString::number(i)+"Downmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].downedge.max,    "Camera"+QString::number(i)+"Downmax",	cameraMatrix[i].campos.y());
        }
        break;
    case 1:
        for (int i=0; i<PARAM::CAMERA;i++){
            CameraEdge leftedge,rightedge,upedge,downedge;
            vpm->loadParam(leftedge.min,    "Camera"+QString::number(i)+"Leftmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(leftedge.max,    "Camera"+QString::number(i)+"Leftmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(rightedge.min,   "Camera"+QString::number(i)+"Rightmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(rightedge.max,   "Camera"+QString::number(i)+"Rightmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(upedge.min,      "Camera"+QString::number(i)+"Upmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(upedge.max,      "Camera"+QString::number(i)+"Upmax",	cameraMatrix[i].campos.y());
            vpm->loadParam(downedge.min,    "Camera"+QString::number(i)+"Downmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(downedge.max,    "Camera"+QString::number(i)+"Downmax",	cameraMatrix[i].campos.y());
            cameraMatrix[i].leftedge.min	=	downedge.min    +3000;
            cameraMatrix[i].leftedge.max	=	downedge.max    +3000;
            cameraMatrix[i].rightedge.min	=	upedge.min      +3000;
            cameraMatrix[i].rightedge.max	=	upedge.max      +3000;
            cameraMatrix[i].upedge.min   	=	-leftedge.min        ;
            cameraMatrix[i].upedge.max   	=	-leftedge.max        ;
            cameraMatrix[i].downedge.min	=	-rightedge.min       ;
            cameraMatrix[i].downedge.max	=	-rightedge.max       ;
        }
        break;
    case 2:
        for (int i=0; i<PARAM::CAMERA;i++){
            vpm->loadParam(cameraMatrix[i].leftedge.min,    "Camera"+QString::number(i)+"Leftmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].leftedge.max,    "Camera"+QString::number(i)+"Leftmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].rightedge.min,   "Camera"+QString::number(i)+"Rightmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].rightedge.max,   "Camera"+QString::number(i)+"Rightmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].upedge.min,      "Camera"+QString::number(i)+"Upmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].upedge.max,      "Camera"+QString::number(i)+"Upmax",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].downedge.min,    "Camera"+QString::number(i)+"Downmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].downedge.max,    "Camera"+QString::number(i)+"Downmax",	cameraMatrix[i].campos.y());
            cameraMatrix[i].leftedge.min	=	cameraMatrix[i].leftedge.min  * 3 /2;
            cameraMatrix[i].leftedge.max	=	cameraMatrix[i].leftedge.max  * 3 /2;
            cameraMatrix[i].rightedge.min	=	cameraMatrix[i].rightedge.min * 3 /2;
            cameraMatrix[i].rightedge.max	=	cameraMatrix[i].rightedge.max * 3 /2;
            cameraMatrix[i].upedge.min   	=	cameraMatrix[i].upedge.min    * 3 /2;
            cameraMatrix[i].upedge.max   	=	cameraMatrix[i].upedge.max    * 3 /2;
            cameraMatrix[i].downedge.min	=	cameraMatrix[i].downedge.min  * 3 /2;
            cameraMatrix[i].downedge.max	=	cameraMatrix[i].downedge.max  * 3 /2;
        }
        break;
    }
}

void CGlobalData::setCameraMatrix(bool real){
    zpm->loadParam(saoAction,"Alert/SaoAction",0);
    if (real){
        //NEED CHANGE IN CANADA
        for (int i=0; i<PARAM::CAMERA;i++){
            double x,y;
            vpm->loadParam(x,"Camera"+QString::number(i)+"CenterX",0);
            vpm->loadParam(y,"Camera"+QString::number(i)+"CenterY",0);
            cameraMatrix[i].fillCenter(saoConvert(CGeoPoint(x,y)));
        }

//        cameraMatrix[0].fillCenter(saoConvert(CGeoPoint(2250,1500)));//(-4500,2250);
//        cameraMatrix[1].fillCenter(saoConvert(CGeoPoint(2250,-1500)));//(-1500,2250);
//        cameraMatrix[2].fillCenter(saoConvert(CGeoPoint(-2250,-1500)));//(1500,2250);
//        cameraMatrix[3].fillCenter(saoConvert(CGeoPoint(-2250,1500)));//(4500,2250);
//        cameraMatrix[4].fillCenter(saoConvert(CGeoPoint(-4500,-2250)));
//        cameraMatrix[5].fillCenter(saoConvert(CGeoPoint(-1500,-2250)));
//        cameraMatrix[6].fillCenter(saoConvert(CGeoPoint(1500,-2250)));
//        cameraMatrix[7].fillCenter(saoConvert(CGeoPoint(4500,-2250)));
        for (int i=0; i<PARAM::CAMERA;i++){
            vpm->loadParam(cameraMatrix[i].leftedge.min,    "Camera"+QString::number(i)+"Leftmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].leftedge.max,    "Camera"+QString::number(i)+"Leftmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].rightedge.min,   "Camera"+QString::number(i)+"Rightmin",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].rightedge.max,   "Camera"+QString::number(i)+"Rightmax",	cameraMatrix[i].campos.x());
            vpm->loadParam(cameraMatrix[i].upedge.min,      "Camera"+QString::number(i)+"Upmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].upedge.max,      "Camera"+QString::number(i)+"Upmax",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].downedge.min,    "Camera"+QString::number(i)+"Downmin",	cameraMatrix[i].campos.y());
            vpm->loadParam(cameraMatrix[i].downedge.max,    "Camera"+QString::number(i)+"Downmax",	cameraMatrix[i].campos.y());
        }
        saoConvertEdge();
    }
    else{               //for Sim
        cameraMatrix[0].fillCenter(saoConvert(CGeoPoint(-4500,2250)));
        cameraMatrix[1].fillCenter(saoConvert(CGeoPoint(-1500,2250)));
        cameraMatrix[2].fillCenter(saoConvert(CGeoPoint(1500,2250)));
        cameraMatrix[3].fillCenter(saoConvert(CGeoPoint(4500,2250)));
        cameraMatrix[4].fillCenter(saoConvert(CGeoPoint(-4500,-2250)));
        cameraMatrix[5].fillCenter(saoConvert(CGeoPoint(-1500,-2250)));
        cameraMatrix[6].fillCenter(saoConvert(CGeoPoint(1500,-2250)));
        cameraMatrix[7].fillCenter(saoConvert(CGeoPoint(4500,-2250)));
        for (int i=0; i<PARAM::CAMERA;i++){
            cameraMatrix[i].leftedge.min=cameraMatrix[i].campos.x()-1800;
            cameraMatrix[i].leftedge.max=cameraMatrix[i].campos.x()-1500;
            cameraMatrix[i].rightedge.min=cameraMatrix[i].campos.x()+1800;
            cameraMatrix[i].rightedge.max=cameraMatrix[i].campos.x()+1500;
            cameraMatrix[i].downedge.min=cameraMatrix[i].campos.y()-2550;
            cameraMatrix[i].downedge.max=cameraMatrix[i].campos.y()-2250;
            cameraMatrix[i].upedge.min=cameraMatrix[i].campos.y()+2550;
            cameraMatrix[i].upedge.max=cameraMatrix[i].campos.y()+2250;
        }
    }
}

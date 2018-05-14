#include "field.h"
#include "visionmodule.h"
#include "transform.h"
#include "globaldata.h"
#include "parammanager.h"
#include <QColor>
#include <QtMath>
#include <QtDebug>
namespace{
//    const static QColor COLOR_BLUE(19,49,137);//QColor(16,86,151)
//    const static QColor COLOR_TRANSBLUE(19,49,137,25);
//    const static QColor COLOR_YELLOW(241,231,36);
//    const static QColor COLOR_TRANSYELLOW(241,231,36,25);
//    const static QColor COLOR_PINK(255,63,149);
//    const static QColor COLOR_GREEN(105,255,0);
//    const static QColor COLOR_ORANGE(255,170,85);
//    const static QColor COLOR_TRANSORANGE(255,170,85,25);
//    const static QColor COLOR_DARKGREEN(Qt::darkGreen);
//    const static QColor COLOR_TRANSPARENT(Qt::transparent);
    const static QColor CAR_COLOR[2]  = {QColor(25,30,150),QColor(241,201,50)};
    const static QColor CAR_SHADOW[2] = {QColor(25,30,150,30),QColor(241,201,50,30)};
    const static QColor FONT_COLOR[2] = {Qt::white,Qt::white};
    const static QColor COLOR_ORANGE(255,0,255);
    const static QColor COLOR_TRANSORANGE(255,170,85,100);
    const static QColor COLOR_DARKGREEN(48,48,48);
    const static QColor COLOR_RED(220,53,47);
    int canvasHeight;
    int canvasWidth;
    int param_width;
    int param_height;
    int param_canvas_width;
    int param_canvas_height;
    int param_goalWidth;
    int param_goalDepth;
    int param_penaltyWidth;
    int param_penaltyLength;
    int param_centerCircleRadius;
    double x(double _x){
        return _x*canvasWidth/param_canvas_width+canvasWidth/2;
    }
    double y(double _y){
        return -_y*canvasHeight/param_canvas_height+canvasHeight/2;
    }
    double w(double _w){
        return _w*canvasWidth/param_canvas_width;
    }
    double h(double _h){
        return -_h*canvasHeight/param_canvas_height;
    }
    double a(double _a){
        return _a*16;
    }
    double r(double _r){
        return _r*16;
    }
    double rx(double _x){
        return (_x-canvasWidth/2)*param_canvas_width/canvasWidth;
    }
    double ry(double _y){
        return -(_y-canvasHeight/2)*param_canvas_height/canvasHeight;
    }
    auto zpm = ZSS::ZParamManager::instance();
    int ballDiameter;
    int shadowDiameter;
    int carDiameter;
    int carFaceWidth;
    int numberSize;
}
Field::Field(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , pixmap(nullptr)
    , pen(QColor(150,150,150),1)
    , cameraMode(true)
    , _type(-1){
    zpm->loadParam(canvasHeight,"canvas/height",720);
    zpm->loadParam(canvasWidth ,"canvas/width" ,960);
    connect(VisionModule::instance(),SIGNAL(needDraw()),this,SLOT(draw()));
    setImplicitWidth(canvasWidth);
    setImplicitHeight(canvasHeight);
    pixmap = new QPixmap(QSize(canvasWidth,canvasHeight));
    pixmapPainter.begin(pixmap);
    changeMode(cameraMode);
    zpm->loadParam(ballDiameter   ,"size/ballDiameter",100);
    zpm->loadParam(shadowDiameter ,"size/shadowDiameter",30);
    zpm->loadParam(carDiameter    ,"size/carDiameter",180);
    zpm->loadParam(carFaceWidth   ,"size/carFaceWidth",120);
    zpm->loadParam(numberSize     ,"size/numberSize",200);
}

void Field::paint(QPainter* painter){
    painter->drawPixmap(area,*pixmap);
}

void Field::changeMode(bool ifBig){
    pen.setWidth(2);
    zpm->loadParam(param_width             , "field/width"             ,12000);
    zpm->loadParam(param_height            , "field/height"            , 9000);
    zpm->loadParam(param_canvas_width      , "field/canvasWidth"       ,13200);
    zpm->loadParam(param_canvas_height     , "field/canvasHeight"      , 9900);
    zpm->loadParam(param_goalWidth         , "field/goalWidth"         , 1200);
    zpm->loadParam(param_goalDepth         , "field/goalDepth"         ,  200);
    zpm->loadParam(param_penaltyWidth      , "field/penaltyWidth"      , 1200);
    zpm->loadParam(param_penaltyLength     , "field/penaltyLength"     , 2400);
    zpm->loadParam(param_centerCircleRadius, "field/centerCircleRadius",  500);
    area = QRect(0,0,this->property("width").toReal(),this->property("height").toReal());
    painterPath = QPainterPath();
    initPainterPath();
    pixmap->fill(COLOR_DARKGREEN);
    pixmapPainter.strokePath(painterPath, pen);
    this->update(area);
}

void Field::draw(){                     //change here!!!!!!!
    pixmap->fill(COLOR_DARKGREEN);
    pixmapPainter.strokePath(painterPath, pen);
    switch(_type){
    case 1:
        drawOriginVision(0);break;
    case 2:
        drawMaintainVision(0);break;
    }
    this->update(area);
}
void Field::initPainterPath(){
    painterPath.addRect(::x(-param_width/2),::y(-param_height/2),::w(param_width),::h(param_height));
    painterPath.addRect(::x(-param_width/2),::y(-param_goalWidth/2),::w(-param_goalDepth),::h(param_goalWidth));
    painterPath.addRect(::x(param_width/2),::y(-param_goalWidth/2),::w(param_goalDepth),::h(param_goalWidth));
    painterPath.moveTo(::x(-param_width/2),::y(0));
    painterPath.lineTo(::x(param_width/2),::y(0));
    painterPath.moveTo(::x(0),::y(param_height/2));
    painterPath.lineTo(::x(0),::y(-param_height/2));
    painterPath.addEllipse(::x(-param_centerCircleRadius),::y(-param_centerCircleRadius),::w(2*param_centerCircleRadius),::h(2*param_centerCircleRadius));

    // old method
    //addQuarterCirclePath(painterPath,::x(param_width/2),::y(-param_penaltyCenterLength/2),::w(param_penaltyRadius),90);
    //addQuarterCirclePath(painterPath,::x(param_width/2),::y(param_penaltyCenterLength/2),::w(param_penaltyRadius),180);
    //addQuarterCirclePath(painterPath,::x(-param_width/2),::y(param_penaltyCenterLength/2),::w(param_penaltyRadius),-90);
    //addQuarterCirclePath(painterPath,::x(-param_width/2),::y(-param_penaltyCenterLength/2),::w(param_penaltyRadius),0);
    //painterPath.moveTo(::x(param_width/2-param_penaltyRadius),::y(param_penaltyCenterLength/2));
    //painterPath.lineTo(::x(param_width/2-param_penaltyRadius),::y(-param_penaltyCenterLength/2));
    //painterPath.moveTo(::x(-(param_width/2-param_penaltyRadius)),::y(param_penaltyCenterLength/2));
    //painterPath.lineTo(::x(-(param_width/2-param_penaltyRadius)),::y(-param_penaltyCenterLength/2));

    // new method
    painterPath.addRect(::x(-param_width/2),::y(-param_penaltyLength/2),::w(param_penaltyWidth),::h(param_penaltyLength));
    painterPath.addRect(::x(param_width/2),::y(-param_penaltyLength/2),::w(-param_penaltyWidth),::h(param_penaltyLength));

}
void Field::drawOriginVision(int index){
    for(int i=0;i<PARAM::CAMERA;i++){
        drawVision(GlobalData::instance()->camera[i][index]);
    }
}
void Field::drawTransformedVision(int index){
    for(int i=0;i<PARAM::CAMERA;i++){
        drawVision(GlobalData::instance()->transformed[i][index]);
    }
}
void Field::drawBallFixedVision(int index){
    for(int i=-99;i<0;i++){
        drawVision(GlobalData::instance()->processBall[index + i],true);
    }
    drawVision(GlobalData::instance()->processBall[index]);
}
void Field::drawRobotFixedVision(int index){
    drawVision(GlobalData::instance()->processRobot[index]);
}

void Field::drawMaintainVision(int index){
    //drawVision(GlobalData::instance()->maintain[index]);
    for(int i=-99;i<0;i++){
        drawVision(GlobalData::instance()->maintain[index + i],true);
    }
    drawVision(GlobalData::instance()->maintain[index]);
    const OriginMessage &vision = GlobalData::instance()->maintain[index];
//    if (GlobalData::instance()->lastTouch!=-1){
//        if (GlobalData::instance()->lastTouch<PARAM::ROBOTMAXID){
//            auto& robot = vision.robot[BLUE][GlobalData::instance()->lastTouch];
//            paintCar(CAR_COLOR[BLUE],robot.id,robot.pos.x(),robot.pos.y(),robot.angel,true,FONT_COLOR[BLUE],true);
//        }
//        else{
//            auto& robot = vision.robot[YELLOW][GlobalData::instance()->lastTouch-PARAM::ROBOTMAXID];
//            paintCar(CAR_COLOR[YELLOW],robot.id,robot.pos.x(),robot.pos.y(),robot.angel,true,FONT_COLOR[YELLOW],true);
//        }
//    }
    for(int color=BLUE;color<=YELLOW;color++){
        for(int j=0;j<vision.robotSize[color];j++){
            auto& robot = vision.robot[color][j];
            if (GlobalData::instance()->lastTouch==color*PARAM::ROBOTMAXID+j)
                paintCar(CAR_COLOR[color],robot.id,robot.pos.x(),robot.pos.y(),robot.angel,true,FONT_COLOR[color],true);
        }
    }
}

void Field::drawModelFixedVision(int index) {
    float x = GlobalData::instance()->montageMaxX;
    float y = GlobalData::instance()->montageMaxY;
    pixmapPainter.setPen(QPen(COLOR_RED));
    pixmapPainter.drawLine(::x(x),::y(-::param_height/2),::x(x),::y(::param_height/2));
    pixmapPainter.drawLine(::x(-x),::y(-::param_height/2),::x(-x),::y(::param_height/2));
    pixmapPainter.drawLine(::x(::param_width/2),::y(y),::x(-::param_width/2),::y(y));
    pixmapPainter.drawLine(::x(::param_width/2),::y(-y),::x(-::param_width/2),::y(-y));
    for(int i=0;i<PARAM::CAMERA;i++){
        drawVision(GlobalData::instance()->modelFixed[i][index]);
    }
}
void Field::drawProcessedVision(int index){
    drawVision(GlobalData::instance()->vision[index]);
}
void Field::paintCar(const QColor& color,quint8 num,qreal x,qreal y,qreal radian,bool ifDrawNum,const QColor& textColor,bool needCircle){
    static qreal radius = carDiameter/2;
    static qreal chordAngel = qRadiansToDegrees(qAcos(1.0*carFaceWidth/carDiameter));
    pixmapPainter.setBrush(QBrush(color));
    pixmapPainter.setPen(Qt::NoPen);
    pixmapPainter.drawChord(::x(x-radius),::y(y-radius),::w(2*radius),::h(2*radius),::a(90.0-chordAngel + 180/M_PI*radian),::r(180.0+2*chordAngel));
    if (needCircle) {
        pixmapPainter.setBrush(Qt::NoBrush);
        pixmapPainter.setPen(QPen(COLOR_RED));
        pixmapPainter.drawChord(::x(x-radius),::y(y-radius),::w(2*radius),::h(2*radius),::a(90.0-chordAngel + 180/M_PI*radian),::r(180.0+2*chordAngel));
    }
    if (ifDrawNum) {
        pixmapPainter.setBrush(Qt::NoBrush);
        pixmapPainter.setPen(QPen(textColor));
        QFont font;
        int fontSize = ::h(-numberSize);
        font.setPixelSize(fontSize);
        pixmapPainter.setFont(font);
        if (num >= 10) {
            pixmapPainter.drawText(::x(x-numberSize*0.5),::y(y+carDiameter*0.4),QString::number(num));//
        } else {
            pixmapPainter.drawText(::x(x-numberSize),::y(y+carDiameter*0.4),QString::number(num));
        }
    }
}
void Field::paintBall(const QColor& color,qreal x, qreal y){
    pixmapPainter.setBrush(QBrush(color));
    pixmapPainter.setPen(Qt::NoPen);
    pixmapPainter.drawEllipse(::x(x-ballDiameter/2),::y(y-ballDiameter/2),::w(ballDiameter),::h(ballDiameter));
}
void Field::paintShadow(const QColor& color,qreal x,qreal y){
    pixmapPainter.setBrush(QBrush(color));
    pixmapPainter.setPen(Qt::NoPen);
    pixmapPainter.drawEllipse(::x(x-shadowDiameter/2),::y(y-shadowDiameter/2),::w(shadowDiameter),::h(shadowDiameter));
}
void Field::drawVision(const OriginMessage &vision,bool shadow){
    for(int color=BLUE;color<=YELLOW;color++){
        for(int j=0;j<vision.robotSize[color];j++){
            auto& robot = vision.robot[color][j];
            if(!shadow){
                paintCar(CAR_COLOR[color],robot.id,robot.pos.x(),robot.pos.y(),robot.angel,true,FONT_COLOR[color]);
            }
            else{
                paintShadow(CAR_SHADOW[color],robot.pos.x(),robot.pos.y());
            }
        }
    }
    for(int j=0;j<vision.ballSize;j++){
        auto& ball = vision.ball[j];
        if(!shadow){
            paintBall(COLOR_ORANGE,ball.pos.x(),ball.pos.y());
        }else{
            paintShadow(COLOR_TRANSORANGE,ball.pos.x(),ball.pos.y());
        }
    }
}
float Field::minimumX = -999999;
float Field::minimumY = -999999;
float Field::maximumX =  999999;
float Field::maximumY =  999999;
void Field::setArea(int sx,int ex,int sy,int ey){
    minimumX = ::rx(sx);
    minimumY = ::ry(sy);
    maximumX = ::rx(ex);
    maximumY = ::ry(ey);
}
bool Field::inChosenArea(float x, float y){
    return (x >= minimumX && x <= maximumX && y >= minimumY && y <= maximumY);
}
bool Field::inChosenArea(CGeoPoint point){
    return (point.x() >= minimumX && point.x() <= maximumX && point.y() >= minimumY && point.y() <= maximumY);
}

#include "field.h"
#include "singleparams.h"
#include "visionmodule.h"
#include "transform.h"
#include "globaldata.h"
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
    const static QColor COLOR_DARKGREEN(150,150,150);
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
}
Field::Field(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , pixmap(nullptr)
    , pen(QColor(255,255,255),1)
    , cameraMode(true)
    , _type(-1){
    canvasHeight = SingleParams::instance()->_("canvas.height");
    canvasWidth  = SingleParams::instance()->_("canvas.width");
    connect(VisionModule::instance(),SIGNAL(needDraw()),this,SLOT(draw()));
    setImplicitWidth(canvasWidth);
    setImplicitHeight(canvasHeight);
    pixmap = new QPixmap(QSize(canvasWidth,canvasHeight));
    pixmapPainter.begin(pixmap);
    changeMode(cameraMode);
}

void Field::paint(QPainter* painter){
    painter->drawPixmap(area,*pixmap);
}

void Field::changeMode(bool ifBig){
    std::string prefix = ifBig ? "bigField" : "smallField";
    pen.setWidth(ifBig ? 2 : 1);
    param_width                 = SingleParams::instance()->_(prefix+".field.width");
    param_height                = SingleParams::instance()->_(prefix+".field.height");
    param_canvas_width          = SingleParams::instance()->_(prefix+".canvas_width");
    param_canvas_height         = SingleParams::instance()->_(prefix+".canvas_height");
    param_goalWidth             = SingleParams::instance()->_(prefix+".field.goalWidth");
    param_goalDepth             = SingleParams::instance()->_(prefix+".field.goalDepth");
    param_penaltyWidth          = SingleParams::instance()->_(prefix+".field.penaltyWidth");
    param_penaltyLength         = SingleParams::instance()->_(prefix+".field.penaltyLength");
    param_centerCircleRadius    = SingleParams::instance()->_(prefix+".field.centerCircleRadius");
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
    case 3:
        drawImmortalsVision();break;
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
    if (GlobalData::instance()->lastTouch!=-1){
        if (GlobalData::instance()->lastTouch<PARAM::ROBOTMAXID){
            auto& robot = vision.robot[BLUE][GlobalData::instance()->lastTouch];
            paintCar(CAR_COLOR[BLUE],robot.id,robot.pos.x(),robot.pos.y(),robot.angel,true,FONT_COLOR[BLUE],true);
        }
        else{
            auto& robot = vision.robot[YELLOW][GlobalData::instance()->lastTouch-PARAM::ROBOTMAXID];
            paintCar(CAR_COLOR[YELLOW],robot.id,robot.pos.x(),robot.pos.y(),robot.angel,true,FONT_COLOR[YELLOW],true);
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
    static float diameter = SingleParams::instance()->_("car.diameter");
    static float faceWidth = SingleParams::instance()->_("car.faceWidth");
    static float numberSize = SingleParams::instance()->_("numberSize");
    static qreal radius = diameter/2;
    static qreal chordAngel = qRadiansToDegrees(qAcos(faceWidth/diameter));
    pixmapPainter.setBrush(QBrush(color));
    pixmapPainter.setPen(Qt::NoPen);
    pixmapPainter.drawChord(::x(x-radius),::y(y-radius),::w(diameter),::h(diameter),::a(90.0-chordAngel + 180/M_PI*radian),::r(180.0+2*chordAngel));
    if (needCircle) {
        pixmapPainter.setBrush(Qt::NoBrush);
        pixmapPainter.setPen(QPen(COLOR_RED));
        pixmapPainter.drawChord(::x(x-radius),::y(y-radius),::w(diameter),::h(diameter),::a(90.0-chordAngel + 180/M_PI*radian),::r(180.0+2*chordAngel));
    }
    if (ifDrawNum) {
        pixmapPainter.setBrush(Qt::NoBrush);
        pixmapPainter.setPen(QPen(textColor));
        QFont font;
        int fontSize = ::h(-numberSize);
        font.setPixelSize(fontSize);
        pixmapPainter.setFont(font);
        if (num >= 10) {
            pixmapPainter.drawText(::x(x-numberSize*0.5),::y(y+diameter*0.4),QString::number(num));
        } else {
            pixmapPainter.drawText(::x(x-numberSize),::y(y+diameter*0.4),QString::number(num));
        }
    }
}
void Field::paintBall(const QColor& color,qreal x, qreal y){
    static float radius = float(SingleParams::instance()->_("ball.diameter"))/2;
    pixmapPainter.setBrush(QBrush(color));
    pixmapPainter.setPen(Qt::NoPen);
    pixmapPainter.drawEllipse(::x(x-radius),::y(y-radius),::w(2*radius),::h(2*radius));
}
void Field::paintShadow(const QColor& color,qreal x,qreal y){
    static float radius = float(SingleParams::instance()->_("shadow.diameter"))/2;
    pixmapPainter.setBrush(QBrush(color));
    pixmapPainter.setPen(Qt::NoPen);
    pixmapPainter.drawEllipse(::x(x-radius),::y(y-radius),::w(2*radius),::h(2*radius));
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
void Field::drawImmortalsVision(){
    auto& state = GlobalData::instance()->immortalsVisionState;
    if(state.has_ball)
        paintBall(COLOR_ORANGE,state.ball.Position.X,state.ball.Position.Y);
    for ( int i = 0 ; i < MAX_ROBOTS ; i ++ ){
        if ( state.OwnRobot[i].seenState != CompletelyOut ){
            auto& robot = state.OwnRobot[i];
            paintCar(CAR_COLOR[BLUE],robot.vision_id,robot.Position.X,robot.Position.Y,robot.Angle/180.0*3.1415926,true,FONT_COLOR[BLUE]);
        }
        if ( state.OppRobot[i].seenState != CompletelyOut ){
            auto& robot = state.OppRobot[i];
            paintCar(CAR_COLOR[YELLOW],robot.vision_id,robot.Position.X,robot.Position.Y,(robot.Angle)/180.0*3.1415926,true,FONT_COLOR[YELLOW]);
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

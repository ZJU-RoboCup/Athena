#include "field.h"
#include "visionmodule.h"
#include "transform.h"
#include "globaldata.h"
#include "parammanager.h"
#include "globalsettings.h"
#include "simulator.h"
#include "zss_debug.pb.h"
#include <QColor>
#include <QtMath>
#include <QtDebug>
namespace{
    const static QColor CAR_COLOR[2]  = {QColor(25,30,150),QColor(241,201,50)};
    const static QColor CAR_SHADOW[2] = {QColor(25,30,150,30),QColor(241,201,50,30)};
    const static QColor CAR_DIR[2] = {Qt::white,Qt::white};
    const static QColor FONT_COLOR[2] = {Qt::white,Qt::white};
    const static QColor DEBUG_COLOR[10] = {
        Qt::white,
        Qt::red,
        QColor(255,100,0),
        Qt::yellow,
        Qt::green,
        Qt::cyan,
        Qt::blue,
        QColor(128,0,255),
        Qt::gray,
        Qt::black
    };
    const static QColor DEBUG_BRUSH_COLOR = QColor(255,255,255,20);
    const static QColor COLOR_ORANGE(255,0,255);
    const static QColor COLOR_TRANSORANGE(255,170,85,100);
    const static QColor COLOR_DARKGREEN(48,48,48);
    const static QColor COLOR_RED(220,53,47);
    const static QColor COLOR_LIGHTWHITE(255,255,255,20);
    const static qreal zoomStep = 0.05;
    const int ballRatio = 2;
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

    auto zpm = ZSS::CParamManager::instance();
    auto vpm = ZSS::VParamManager::instance();
    bool isSimulation;
    int ballDiameter;
    int shadowDiameter;
    int carDiameter;
    int carFaceWidth;
    int numberSize;
    qreal zoomRatio;
    QPoint zoomStart;
    QRect area;
    QSize size;
    double x(double _x){
        return _x*canvasWidth/param_canvas_width+canvasWidth/2;
    }
    double y(double _y){
        return -_y*canvasHeight/param_canvas_height+canvasHeight/2;
    }
    QPointF p(QPointF& _p){
        return QPointF(x(_p.x()),y(_p.y()));
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
    double orx(double _x){
        return (_x-canvasWidth/2)*param_canvas_width/canvasWidth;
    }
    double ory(double _y){
        return -(_y-canvasHeight/2)*param_canvas_height/canvasHeight;
    }
    double orw(double _w){
        return (_w)*param_canvas_width/canvasWidth;
    }
    double orh(double _h){
        return -(_h)*param_canvas_height/canvasHeight;
    }
    double rx(double x){
        return ::orx(zoomStart.x() + x*zoomRatio);
    }
    double ry(double y){
        return ::ory(zoomStart.y() + y*zoomRatio);
    }
    QPointF rp(const QPointF& p){
        return QPointF(rx(p.x()),ry(p.y()));
    }
    double distance2(double dx,double dy){
        return dx*dx + dy*dy;
    }
    template<typename T>
    T limitRange(T value,T minValue,T maxValue){
        return value > maxValue ? maxValue : (value < minValue) ? minValue : value;
    }
    auto GS = GlobalSettings::instance();
}
Field::Field(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , pixmap(nullptr)
    , pen(QColor(150,150,150),1)
    , cameraMode(true)
    , _type(-1){
    setFillColor(COLOR_DARKGREEN);
    zpm->loadParam(canvasHeight,"canvas/height",960);
    zpm->loadParam(canvasWidth ,"canvas/width" ,1280);
    connect(VisionModule::instance(),SIGNAL(needDraw()),this,SLOT(draw()));
    setImplicitWidth(canvasWidth);
    setImplicitHeight(canvasHeight);
    pixmap = new QPixmap(QSize(canvasWidth,canvasHeight));
    pixmapPainter.begin(pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing, true);
    pixmapPainter.setRenderHint(QPainter::TextAntialiasing, true);
    pixmapPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    init();
    zpm->loadParam(isSimulation   ,"Alert/IsSimulation",false);
    zpm->loadParam(ballDiameter   ,"size/ballDiameter",100);
    zpm->loadParam(shadowDiameter ,"size/shadowDiameter",30);
    zpm->loadParam(carDiameter    ,"size/carDiameter",180);
    zpm->loadParam(carFaceWidth   ,"size/carFaceWidth",120);
    zpm->loadParam(numberSize     ,"size/numberSize",200);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    //setAcceptHoverEvents(true);
    connect(GS,SIGNAL(needRepaint()),this,SLOT(repaint()));
    resetAfterMouseEvent();
}

void Field::paint(QPainter* painter){
    painter->drawPixmap(area,*pixmap,QRect(zoomStart,::size*zoomRatio));
}

void Field::mousePressEvent(QMouseEvent *e)
{
    pressed = e->buttons();
    checkClosestRobot(rx(e->x()),ry(e->y()));
    start = end = rp(e->pos());
    switch(pressed){
    case Qt::LeftButton:
        leftPressEvent(e);
        break;
    case Qt::RightButton:
        rightPressEvent(e);
        break;
    default:
        break;
    }
}
void Field::mouseMoveEvent(QMouseEvent *e){
    end = rp(e->pos());
    switch(pressed){
    case Qt::LeftButton:
        leftMoveEvent(e);
        break;
    case Qt::RightButton:
        rightMoveEvent(e);
        break;
    default:
        break;
    }
}
void Field::mouseReleaseEvent(QMouseEvent *e){
    switch(pressed){
    case Qt::LeftButton:
        leftReleaseEvent(e);
        break;
    case Qt::RightButton:
        rightReleaseEvent(e);
        break;
    default:
        break;
    }
    resetAfterMouseEvent();
    //Simulator::instance()->setBall(rx(e->x())/1000.0,ry(e->y())/1000.0);
}
void Field::resetAfterMouseEvent(){
    pressed = 0;
    pressedRobot = false;
    start = end = QPoint(-9999,-9999);
}
void Field::checkClosestRobot(double x,double y){
    double limit = carDiameter*carDiameter/4;
    auto& vision = GlobalData::instance()->maintain[0];
    for(int color=BLUE;color<=YELLOW;color++){
        for(int j=0;j<vision.robotSize[color];j++){
            auto& robot = vision.robot[color][j];
            if(distance2(robot.pos.x()-x,robot.pos.y()-y)<limit){
                robotID = robot.id;
                robotTeam = color;
                pressedRobot = true;
                return;
            }
        }
    }
    pressedRobot = false;
}
void Field::leftMoveEvent(QMouseEvent *e){
    if(pressedRobot){
        Simulator::instance()->setRobot(rx(e->x())/1000.0,ry(e->y())/1000.0,robotID,robotTeam == YELLOW);
    }else{
        Simulator::instance()->setBall(rx(e->x())/1000.0,ry(e->y())/1000.0);
    }
}
void Field::leftPressEvent(QMouseEvent *e){
    if(pressedRobot){
        Simulator::instance()->setRobot(rx(e->x())/1000.0,ry(e->y())/1000.0,robotID,robotTeam == YELLOW);
    }else{
        Simulator::instance()->setBall(rx(e->x())/1000.0,ry(e->y())/1000.0);
    }
}
void Field::leftReleaseEvent(QMouseEvent *e){

}
void Field::rightMoveEvent(QMouseEvent *e){
    QLineF line(start,end);
    if(pressedRobot){
        displayData = -line.angle();
        if(displayData<-180) displayData += 360;
        Simulator::instance()->setRobot(start.x()/1000.0,start.y()/1000.0,robotID,robotTeam == YELLOW,displayData);
    }else{
        displayData = ballRatio * line.length()/1000.0;
    }
}
void Field::rightPressEvent(QMouseEvent *e){

}
void Field::rightReleaseEvent(QMouseEvent *e){
    QLineF line(start,end);
    if(!pressedRobot){
        Simulator::instance()->setBall(start.x()/1000.0,start.y()/1000.0,ballRatio * line.dx()/1000.0,ballRatio * line.dy()/1000.0);
    }
}
#if QT_CONFIG(wheelevent)
void Field::wheelEvent (QWheelEvent *e)
{
    qreal oldRatio = zoomRatio;
    zoomRatio += (e->delta() < 0 ? zoomStep : -zoomStep);
    zoomRatio = limitRange(zoomRatio,zoomStep,1.0);
    zoomStart -= e->pos()*(zoomRatio-oldRatio);
    zoomStart.setX(limitRange(zoomStart.x(),0,int(area.width()*(1-zoomRatio))));
    zoomStart.setY(limitRange(zoomStart.y(),0,int(area.height()*(1-zoomRatio))));
    this->update(area);
}
#endif
void Field::moveField(int dx,int dy){
    zoomStart -= QPoint(dx,dy)*zoomRatio;
    zoomStart.setX(limitRange(zoomStart.x(),0,int(area.width()*(1-zoomRatio))));
    zoomStart.setY(limitRange(zoomStart.y(),0,int(area.height()*(1-zoomRatio))));
}
void Field::setSize(int width,int height){
    canvasWidth = width;
    canvasHeight = height;
    zpm->changeParam("canvas/height",canvasHeight);
    zpm->changeParam("canvas/width" ,canvasWidth);
}
void Field::init(){
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
    ::area = QRect(0,0,this->property("width").toReal(),this->property("height").toReal());
    ::size = QSize(this->property("width").toReal(),this->property("height").toReal());
    ::zoomStart = QPoint(0,0);
    ::zoomRatio = 1;
    pressedRobot = false;
    painterPath = QPainterPath();
    initPainterPath();
    repaint();
}
void Field::repaint(){
    paintInit();
    this->update(area);
}
void Field::paintInit(){
    pixmap->fill(COLOR_DARKGREEN);
    pixmapPainter.strokePath(painterPath, pen);
    pixmapPainter.setBrush(QBrush(COLOR_LIGHTWHITE));
    pixmapPainter.setPen(Qt::NoPen);
    pixmapPainter.drawRect(QRect(QPoint(::x(GS->minimumX),::y(GS->minimumY)),QPoint(::x(GS->maximumX),::y(GS->maximumY))));

}
void Field::draw(){                     //change here!!!!!!!
    paintInit();
    switch(_type){
    case 1:
        drawOriginVision(0);break;
    case 2:
        drawMaintainVision(0);
        drawDebugMessages();
        break;
    }
    drawBallLine();
    drawCtrlC();
    this->update(area);
}
void Field::drawBallLine(){
    if(pressed == Qt::RightButton){
        pixmapPainter.setBrush(QBrush(FONT_COLOR[0]));
        pixmapPainter.setPen(QPen(FONT_COLOR[0],2,Qt::DashLine));
        pixmapPainter.drawLine(p(start),p(end));
        pixmapPainter.drawText(p(end),QString::fromStdString(std::to_string(displayData)));
    }
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
    painterPath.addRect(::x(-param_width/2),::y(-param_penaltyLength/2),::w(param_penaltyWidth),::h(param_penaltyLength));
    painterPath.addRect(::x(param_width/2),::y(-param_penaltyLength/2),::w(-param_penaltyWidth),::h(param_penaltyLength));



}
void Field::drawOriginVision(int index){
    for(int i=0;i<PARAM::CAMERA;i++){
        if (GlobalData::instance()->cameraControl[i]==true)
            drawVision(GlobalData::instance()->camera[i][index]);
    }
    for (int i=0;i<PARAM::CAMERA;i++){
        if (GlobalData::instance()->cameraControl[i]==true){
            pixmapPainter.setBrush( Qt::NoBrush);
            pixmapPainter.setPen(COLOR_DARKGREEN);
            pixmapPainter.drawRect(QRect(QPoint(::x(GlobalData::instance()->cameraMatrix[i].leftedge.min),::y(GlobalData::instance()->cameraMatrix[i].downedge.min)),QPoint(::x(GlobalData::instance()->cameraMatrix[i].rightedge.min),::y(GlobalData::instance()->cameraMatrix[i].upedge.min))));
            pixmapPainter.setPen(COLOR_RED);
            pixmapPainter.drawRect(QRect(QPoint(::x(GlobalData::instance()->cameraMatrix[i].leftedge.max),::y(GlobalData::instance()->cameraMatrix[i].downedge.max)),QPoint(::x(GlobalData::instance()->cameraMatrix[i].rightedge.max),::y(GlobalData::instance()->cameraMatrix[i].upedge.max))));
            pixmapPainter.setFont(QFont("Helvetica [Cronyx]", 13,QFont::Bold));
            pixmapPainter.drawText(::x(GlobalData::instance()->cameraMatrix[i].campos.x()-100),::y(GlobalData::instance()->cameraMatrix[i].campos.y()),"Camera"+QString::number(i));
        }
    }
}
void Field::drawMaintainVision(int index){
    for(int i=-99;i<0;i++){
        drawVision(GlobalData::instance()->maintain[index + i],true);
    }
    drawVision(GlobalData::instance()->maintain[index]);
    const OriginMessage &vision = GlobalData::instance()->maintain[index];
    if (GlobalData::instance()->lastTouch!=-1){
        if (GlobalData::instance()->lastTouch<PARAM::ROBOTMAXID)
            for (index=0;index<vision.robotSize[BLUE];index++){
                auto& robot = vision.robot[BLUE][index];
                if (robot.id==GlobalData::instance()->lastTouch) paintCar(CAR_COLOR[BLUE],robot.id,robot.pos.x(),robot.pos.y(),robot.angel,true,FONT_COLOR[BLUE],true);
            }
        else
            for (index=0;index<vision.robotSize[YELLOW];index++){
                auto& robot = vision.robot[YELLOW][index];
                if (robot.id==GlobalData::instance()->lastTouch-PARAM::ROBOTMAXID) paintCar(CAR_COLOR[YELLOW],robot.id,robot.pos.x(),robot.pos.y(),robot.angel,true,FONT_COLOR[YELLOW],true);
            }
    }
}
void Field::paintCar(const QColor& color,quint8 num,qreal x,qreal y,qreal radian,bool ifDrawNum,const QColor& textColor,bool needCircle){
    static qreal radius = carDiameter/2;
    static qreal chordAngel = qRadiansToDegrees(qAcos(1.0*carFaceWidth/carDiameter));
    pixmapPainter.setBrush(QBrush(color));
    pixmapPainter.setPen(QPen(COLOR_RED,3,Qt::DotLine));
    pixmapPainter.drawLine(QPointF(::x(x),::y(y)),QPointF(::x(x),::y(y))+QPointF(30*qSin(radian+M_PI_2),30*qCos(radian+M_PI_2)));
    pixmapPainter.setPen(Qt::NoPen);
    pixmapPainter.drawChord(::x(x-radius),::y(y-radius),::w(2*radius),::h(2*radius),::a(90.0-chordAngel + 180/M_PI*radian),::r(180.0+2*chordAngel));
    if (needCircle) {
        pixmapPainter.setBrush(Qt::NoBrush);
        pixmapPainter.setPen(QPen(COLOR_RED,3));
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
            pixmapPainter.drawText(::x(x-numberSize*0.5),::y(y+carDiameter*0.4),QString::number(num));
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
void Field::drawCtrlC(){
    pixmapPainter.setFont(QFont("Helvetica [Cronyx]", 300,QFont::Bold));
    GlobalData::instance()->ctrlCMutex.lock();
    bool ctrlC = GlobalData::instance()->ctrlC;
    GlobalData::instance()->ctrlCMutex.unlock();
    if(ctrlC){
        pixmapPainter.drawText(zoomStart+QPointF(0,300),"CTRL_C");
    }
    pixmapPainter.setFont(QFont("Helvetica [Cronyx]", 13,QFont::Bold));
}
void Field::drawDebugMessages(){
    static ZSS::Protocol::Debug_Msgs msgs;
    GlobalData::instance()->debugMutex.lock();
    msgs.ParseFromArray(GlobalData::instance()->debugMessages.data(),GlobalData::instance()->debugMessages.size());
    GlobalData::instance()->debugMutex.unlock();
    pixmapPainter.setFont(QFont("Helvetica [Cronyx]", 13,QFont::Bold));
    pixmapPainter.setBrush(QBrush(DEBUG_BRUSH_COLOR));
    for(int i=0;i<msgs.msgs_size();i++){
        auto& msg = msgs.msgs(i);
        pixmapPainter.setPen(DEBUG_COLOR[msg.color()]);
        switch(msg.type()){
        case ZSS::Protocol::Debug_Msg_Debug_Type_ARC:
            //pixmapPainter.drawArc(QRectF(QPointF(msg.arc().rectangle().point1().x()*10,-msg.arc().rectangle().point1().y()*10),QPointF(msg.arc().rectangle().point2().x()*10,-msg.arc().rectangle().point2().y()*10)),msg.arc().start(),msg.arc().end()-msg.arc().start());
            break;
        case ZSS::Protocol::Debug_Msg_Debug_Type_LINE:
            pixmapPainter.drawLine(::x(msg.line().start().x()*10),::y(-msg.line().start().y()*10),::x(msg.line().end().x()*10),::y(-msg.line().end().y()*10));
            break;
        case ZSS::Protocol::Debug_Msg_Debug_Type_TEXT:
            pixmapPainter.drawText(QPointF(::x(msg.text().pos().x()*10),::y(-msg.text().pos().y()*10)),QString::fromStdString(msg.text().text()));
            break;
        case ZSS::Protocol::Debug_Msg_Debug_Type_ROBOT:
            pixmapPainter.drawEllipse(::x((msg.robot().pos().x())*10-carDiameter/2),::y(-(msg.robot().pos().y())*10+carDiameter/2),::w((carDiameter)),::h(-(carDiameter)));
            break;
        //case ZSS::Protocol::Debug_Msg_Debug_Type_CURVE:
        //case ZSS::Protocol::Debug_Msg_Debug_Type_POLYGON:
        default:
            qDebug() << "debug message type not support!";
        }
    }
}
float Field::fieldXFromCoordinate(int x){
    return ::rx(x);
}
float Field::fieldYFromCoordinate(int y){
    return ::ry(y);
}

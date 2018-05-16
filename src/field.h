#ifndef __FIELD_H__
#define __FIELD_H__

#include <QQuickPaintedItem>
#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include "messageformat.h"
#include "staticparams.h"
class Field : public QQuickPaintedItem{
    Q_OBJECT
    Q_PROPERTY(int type READ type WRITE setType)
public:
    void paint(QPainter* painter) override;
    Field(QQuickItem *parent = 0);
    inline int type() { return this->_type; }
    inline void setType(int t) { this->_type = t; }
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    static float fieldXFromCoordinate(int);
    static float fieldYFromCoordinate(int);
    static void moveField(int,int);
//    void mouseDoubleClickEvent(QMouseEvent *event) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent * event) override;
#endif
    virtual ~Field() {}
public slots:
    void draw();
private slots:
    void repaint();
private:
    void init();
    void initPainterPath();
    void initField();
    void paintInit();
    void paintCar(const QColor& color,quint8 num,qreal x,qreal y,qreal radian
                  ,bool ifDrawNum = true,const QColor& textColor = Qt::white,bool needCircle = false);
    void paintBall(const QColor& color,qreal x,qreal y);
    void paintShadow(const QColor& color,qreal x,qreal y);
    void drawOriginVision(int);
    void drawMaintainVision(int);
//    void drawBallFixedVision(int);
//    void drawRobotFixedVision(int);
//    void drawTransformedVision(int);
//    void drawModelFixedVision(int);
//    void drawProcessedVision(int);
    void drawVision(const OriginMessage&,bool shadow = false);
    void leftMoveEvent(QMouseEvent *);
    void leftPressEvent(QMouseEvent *);
    void leftReleaseEvent(QMouseEvent *);
    void rightMoveEvent(QMouseEvent *);
    void rightPressEvent(QMouseEvent *);
    void rightReleaseEvent(QMouseEvent *);
    QPixmap *pixmap;
    QPainter pixmapPainter;
    QPainterPath painterPath;
    QPen pen;
    bool cameraMode;
    int _type;

private:
    int pressed;
    bool pressedRobot;
    int robotID;
    int robotTeam;
    QPointF start,end;
    double displayData;
private:
    void checkClosestRobot(double,double);
    void resetAfterMouseEvent();
    void drawBallLine();
};

#endif // __FIELD_H__

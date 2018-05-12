#ifndef __FIELD_H__
#define __FIELD_H__

#include <QQuickPaintedItem>
#include <QPainter>
#include <QPainterPath>
#include <QImage>
#include "messageformat.h"
class Field : public QQuickPaintedItem{
    Q_OBJECT
    Q_PROPERTY(int type READ type WRITE setType)
public:
    void paint(QPainter* painter) override;
    Field(QQuickItem *parent = 0);
    void changeMode(bool ifBig);
    inline int type() { return this->_type; }
    inline void setType(int t) { this->_type = t; }
    static bool inChosenArea(float,float);
    static bool inChosenArea(CGeoPoint);
    static void setArea(int,int,int,int);
    virtual ~Field() {};
signals:

public slots:
    void draw();
private:
    void initPainterPath();
    void initField();
    void paintCar(const QColor& color,quint8 num,qreal x,qreal y,qreal radian
                  ,bool ifDrawNum = true,const QColor& textColor = Qt::white,bool needCircle = false);
    void paintBall(const QColor& color,qreal x,qreal y);
    void paintShadow(const QColor& color,qreal x,qreal y);
    void drawOriginVision(int);
    void drawBallFixedVision(int);
    void drawRobotFixedVision(int);
    void drawMaintainVision(int);
    void drawTransformedVision(int);
    void drawModelFixedVision(int);
    void drawProcessedVision(int);
    void drawVision(const OriginMessage&,bool shadow = false);
    QPixmap *pixmap;
    QPainter pixmapPainter;
    QPainterPath painterPath;
    QPen pen;
    bool cameraMode;
    int _type;
    QRect area;
    static float minimumX,maximumX,minimumY,maximumY;
};

#endif // __FIELD_H__

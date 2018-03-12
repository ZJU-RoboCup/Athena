#ifndef VISIONBALL_H
#define VISIONBALL_H

#include <singleton.hpp>
#include <QObject>

class CVisionball : public QObject
{
    Q_OBJECT
public:
    CVisionball();
    void run(bool);

signals:

public slots:
};

typedef Singleton<CVisionball> Visionball;
#endif // VISIONBALL_H

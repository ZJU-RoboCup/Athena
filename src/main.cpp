#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "visionmodule.h"
#include "interaction.h"
#include "field.h"
#include "graph/graph.h"
#include <parammanager.h>
void qmlRegister(){
    qmlRegisterType<Field>("Client.Component", 1, 0, "Field");
    qmlRegisterType<Interaction>("Client.Component", 1, 0, "Interaction");
    qmlRegisterType<Graph>("Graph", 1, 0, "Graph");
}

int main(int argc, char *argv[]){
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    qmlRegister();
    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/src/qml/main.qml")));

    return app.exec();
}

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "visionmodule.h"
#include "interaction.h"
#include "interaction4field.h"
#include "field.h"
#include "graph/graph.h"
#include "parammanager.h"
#include "translator.hpp"
#include "paraminterface.h"
#include "simulator.h"
#include "test.h"
void qmlRegister(){
    qmlRegisterType<Field>("ZSS", 1, 0, "Field");
    qmlRegisterType<Interaction>("ZSS", 1, 0, "Interaction");
    qmlRegisterType<Interaction4Field>("ZSS", 1, 0, "Interaction4Field");
    qmlRegisterType<Graph>("Graph", 1, 0, "Graph");
    qmlRegisterType<Translator>("ZSS", 1, 0, "Translator");
    qmlRegisterType<Interaction>("ZSS", 1, 0, "Interaction");
    //qmlRegisterType<Test>("ZSS", 1, 0, "Test");
    qmlRegisterType<ParamInterface>("ZSS", 1, 0, "ParamModel");
    Simulator::instance();
}

int main(int argc, char *argv[]){
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    qmlRegister();
    app.setFont(QFont("Microsoft YaHei",9));
    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/src/qml/main.qml")));

    return app.exec();
}

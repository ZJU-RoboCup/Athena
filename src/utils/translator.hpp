#ifndef TRANSLATION_H
#define TRANSLATION_H
#include <QTranslator>
#include <QCoreApplication>
#include <QtDebug>
class Translator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString n READ getN NOTIFY tlChanged)

public:
    Translator() {
        translator = new QTranslator(this);
    }
    ~Translator() {
        delete translator;
    }
    QString getN() {
        return "";
    }
    Q_INVOKABLE void selectLanguage(QString language) {
        if(language == QString("zh")) {
            if(!translator->load("t1_zh", QCoreApplication::applicationDirPath())){
                qWarning("Miss Language Package t1_zh.qm!");
            }
            QCoreApplication::instance()->installTranslator(translator);
        }
        if(language == QString("en")) {
            QCoreApplication::instance()->removeTranslator(translator);
        }
        emit tlChanged();
    }
signals:
    void tlChanged();
private:
    QTranslator *translator;
};

#endif // TRANSLATION_H

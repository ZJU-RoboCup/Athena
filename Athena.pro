TEMPLATE = app

CONFIG += c++11

QT += qml quick serialport

CONFIG(debug, debug|release) {
    TARGET = AthenaD
    DESTDIR = $$PWD/../ZJUNlictSoftwareD
    MOC_DIR = ./temp
    OBJECTS_DIR = ./temp
}
CONFIG(release, debug|release) {
    TARGET = Athena
    DESTDIR = $$PWD/../ZJUNlict
    MOC_DIR = ./temp
    OBJECTS_DIR = ./temp
}

SOURCES += \
    src/main.cpp \
    src/field.cpp \
    src/interaction.cpp \
    src/utils/singleparams.cpp \
    src/utils/globaldata.cpp \
    src/utils/geometry.cpp \
    src/proto/cpp/messages_robocup_ssl_detection.pb.cc \
    src/proto/cpp/messages_robocup_ssl_geometry_legacy.pb.cc \
    src/proto/cpp/messages_robocup_ssl_geometry.pb.cc \
    src/proto/cpp/messages_robocup_ssl_refbox_log.pb.cc \
    src/proto/cpp/messages_robocup_ssl_wrapper_legacy.pb.cc \
    src/proto/cpp/messages_robocup_ssl_wrapper.pb.cc \
    src/proto/cpp/vision_detection.pb.cc \
    src/vision/transform.cpp \
    src/vision/dealball.cpp \
    src/vision/dealrobot.cpp \
    src/vision/maintain.cpp \
    src/vision/collisiondetect.cpp \
    src/vision/visionmodule.cpp \
    src/vision/kalmanfilter.cpp \
    src/utils/matrix2d.cpp \
    graph/graph.cpp \
    graph/gridnode.cpp \
    graph/linenode.cpp \
    src/utils/parammanager.cpp \
    src/test.cpp \
    src/communicator.cpp \
    src/proto/cpp/zss_cmd.pb.cc \
    src/radio/actionmodule.cpp \
    src/radio/crc.cpp \
    src/interaction4field.cpp

RESOURCES += \
    Athena.qrc

HEADERS += \
    src/field.h \
    src/vision/visionmodule.h \
    src/utils/singleton.hpp \
    src/utils/dataqueue.hpp \
    src/utils/globaldata.h \
    src/utils/singleparams.h \
    src/interaction.h \
    src/vision/messageformat.h \
    src/utils/staticparams.h \
    src/proto/cpp/messages_robocup_ssl_detection.pb.h \
    src/proto/cpp/messages_robocup_ssl_geometry_legacy.pb.h \
    src/proto/cpp/messages_robocup_ssl_geometry.pb.h \
    src/proto/cpp/messages_robocup_ssl_refbox_log.pb.h \
    src/proto/cpp/messages_robocup_ssl_wrapper_legacy.pb.h \
    src/proto/cpp/messages_robocup_ssl_wrapper.pb.h \
    src/proto/cpp/vision_detection.pb.h \
    src/vision/dealball.h \
    src/vision/dealrobot.h \
    src/vision/maintain.h \
    src/vision/collisiondetect.h \
    src/utils/geometry.h \
    src/vision/kalmanfilter.h \
    src/utils/matrix2d.h \
    graph/graph.h \
    graph/gridnode.h \
    graph/linenode.h \
    src/utils/parammanager.h \
    src/test.h \
    src/utils/translator.hpp \
    src/paraminterface.h \
    src/communicator.h \
    src/proto/cpp/zss_cmd.pb.h \
    src/radio/actionmodule.h \
    src/radio/crc.h \
    src/interaction4field.h

INCLUDEPATH += \
    $$PWD/src/utils \
    $$PWD/src/vision \
    $$PWD/src/proto/cpp \
    $$PWD/src/radio \
    $$PWD/src
macx {
    PROTOBUF_INCLUDE_DIR = /usr/local/include
    LIBPROTOBUF = /usr/local/lib/libprotobuf.a
}

win32 {
    PROTOBUF_INCLUDE_DIR = C:\usr\local\protobuf\2.6.1\include
    CONFIG(release,debug|release){
        LIBPROTOBUF = $$PWD\lib\libprotobuf.lib
    }
    CONFIG(debug,debug|release){
        LIBPROTOBUF = $$PWD\lib\libprotobufD.lib
    }
}
unix:!macx{
    PROTOBUF_INCLUDE_DIR = /usr/local/include
    LIBPROTOBUF = /usr/local/lib/libprotobuf.so
}

defineTest(copyToDestdir) {
    files = $$1
    for(FILE, files) {
        macx {
            DDIR = $${DESTDIR}/$${TARGET}.app/Contents/MacOS
        }else {
            DDIR = $$DESTDIR
        }
        # Replace slashes in paths with backslashes for Windows
        win32:FILE ~= s,/,\\,g
        win32:DDIR ~= s,/,\\,g
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)
    }
    export(QMAKE_POST_LINK)
}
LIBS += $$LIBPROTOBUF
INCLUDEPATH += $$PROTOBUF_INCLUDE_DIR

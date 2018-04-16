TEMPLATE = app

CONFIG += c++11

QT += qml quick

TARGET = Athena_V02

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
    immortals/immortalsvision.cpp \
    immortals/Source/Reality/Vision/Kalman/FilteredObject.cpp \
    immortals/Source/Common/MATHS_REGRESSION_PARABOLIC.cpp \
    immortals/Source/Reality/Vision/Protobuf/ImmortalsProtocols.pb.cc \
    immortals/Source/Reality/Vision/Protobuf/aidebug.pb.cc \
    immortals/Source/Reality/Vision/VisionBall.cpp \
    immortals/Source/Reality/Vision/VisionRobot.cpp \
    src/vision/transform.cpp \
    src/vision/dealball.cpp \
    src/vision/dealrobot.cpp \
    src/vision/maintain.cpp \
    src/vision/collisiondetect.cpp \
    src/vision/visionmodule.cpp

DISTFILES += \
    opt/params.json

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
    immortals/immortalsvision.h \
    immortals/Source/Common/MedianFilter.h \
    immortals/Source/Reality/Vision/Kalman/FilteredObject.h \
    immortals/Source/Common/MATHS_REGRESSION_PARABOLIC.h \
    immortals/Source/Reality/Vision/WorldState.h \
    immortals/Source/Reality/Vision/Protobuf/ImmortalsProtocols.pb.h \
    immortals/Source/Reality/Vision/Protobuf/aidebug.pb.h \
    src/vision/dealball.h \
    src/vision/dealrobot.h \
    src/vision/maintain.h \
    src/vision/collisiondetect.h \
    src/utils/geometry.h

INCLUDEPATH += \
    $$PWD/src/utils \
    $$PWD/src/vision \
    $$PWD/src/proto/cpp \
    $$PWD/src
macx {
    PROTOBUF_INCLUDE_DIR = /usr/local/include
    LIBPROTOBUF = /usr/local/lib/libprotobuf.dylib
}

win32 {
    PROTOBUF_INCLUDE_DIR = C:\usr\local\protobuf\2.6.1\include
    CONFIG(release,debug|release){
        LIBPROTOBUF = C:\usr\local\protobuf\2.6.1\lib\vs14.0\libprotobuf.lib
    }
    CONFIG(debug,debug|release){
        LIBPROTOBUF = C:\usr\local\protobuf\2.6.1\lib\vs14.0\libprotobufd.lib
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

DESTDIR = ./bin
MOC_DIR = .
OBJECTS_DIR = .
LIBS += $$LIBPROTOBUF
INCLUDEPATH += $$PROTOBUF_INCLUDE_DIR

copyToDestdir($$PWD/opt/params.json $$PWD/immortals/Data/ballFilterFast.txt $$PWD/immortals/Data/ballFilterSlow.txt)


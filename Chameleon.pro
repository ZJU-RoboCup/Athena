macx {
    system("/usr/local/bin/protoc --cpp_out=$$PWD/proto --proto_path=$$PWD/proto $$PWD/proto/*.proto")
}
TEMPLATE = app

CONFIG += c++11

QT += qml quick

TARGET = Chameleon

SOURCES += \
    src/main.cpp \
    src/utils/globaldata.cpp \
    src/field.cpp \
    src/utils/singleparams.cpp \
    src/interaction.cpp \
    Owl/src/visionmodule.cpp \
    proto/messages_robocup_ssl_detection.pb.cc \
    proto/messages_robocup_ssl_geometry_legacy.pb.cc \
    proto/messages_robocup_ssl_geometry.pb.cc \
    proto/messages_robocup_ssl_refbox_log.pb.cc \
    proto/messages_robocup_ssl_wrapper_legacy.pb.cc \
    proto/messages_robocup_ssl_wrapper.pb.cc \
    Owl/src/transform.cpp \
    Owl/src/modelfix.cpp \
    Owl/src/montage.cpp \
    Owl/src/modelfix/modelfixsingle.cpp \
    immortals/immortalsvision.cpp \
    immortals/Source/Reality/Vision/Kalman/FilteredObject.cpp \
    immortals/Source/Common/MATHS_REGRESSION_PARABOLIC.cpp \
    immortals/Source/Reality/Vision/Protobuf/ImmortalsProtocols.pb.cc \
    immortals/Source/Reality/Vision/Protobuf/aidebug.pb.cc \
    immortals/Source/Reality/Vision/VisionBall.cpp \
    immortals/Source/Reality/Vision/VisionRobot.cpp \
    Owl/src/transform.cpp \
    Owl/src/dealball.cpp \
    Owl/src/dealrobot.cpp \
    Owl/src/maintain.cpp

DISTFILES += \
    opt/params.json

RESOURCES += \
    Chameleon.qrc

HEADERS += \
    Owl/src/visionmodule.h \
    src/utils/singleton.hpp \
    src/utils/dataqueue.hpp \
    src/utils/globaldata.h \
    src/field.h \
    src/utils/singleparams.h \
    src/interaction.h \
    Owl/src/messageformat.h \
    src/utils/staticparams.h \
    proto/messages_robocup_ssl_detection.pb.h \
    proto/messages_robocup_ssl_geometry_legacy.pb.h \
    proto/messages_robocup_ssl_geometry.pb.h \
    proto/messages_robocup_ssl_refbox_log.pb.h \
    proto/messages_robocup_ssl_wrapper_legacy.pb.h \
    proto/messages_robocup_ssl_wrapper.pb.h \
    Owl/src/transform.h \
    Owl/src/modelfix.h \
    Owl/src/montage.h \
    Owl/src/modelfix/modelfixsingle.h \
    immortals/immortalsvision.h \
    immortals/Source/Common/MedianFilter.h \
    immortals/Source/Reality/Vision/Kalman/FilteredObject.h \
    immortals/Source/Common/MATHS_REGRESSION_PARABOLIC.h \
    immortals/Source/Reality/Vision/WorldState.h \
    immortals/Source/Reality/Vision/Protobuf/ImmortalsProtocols.pb.h \
    immortals/Source/Reality/Vision/Protobuf/aidebug.pb.h \
    Owl/src/dealball.h \
    Owl/src/dealrobot.h \
    Owl/src/maintain.h

INCLUDEPATH += \
    $$PWD/src/utils \
    $$PWD/Owl/src
macx {
    PROTOBUF_INCLUDE_DIR = /usr/local/include
    LIBPROTOBUF = /usr/local/lib/libprotobuf.dylib
}

win32 {
    PROTOBUF_INCLUDE_DIR = $$PWD\protobuf\include
    LIBPROTOBUF = $$PWD\protobuf\lib\libprotobuf.lib
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


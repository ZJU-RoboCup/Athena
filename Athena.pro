TEMPLATE = app

CONFIG += c++11

QT += qml quick serialport

RC_ICONS = Athena.ico
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
    ../Medusa/share/geometry.cpp \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_detection.pb.cc \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_geometry_legacy.pb.cc \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_geometry.pb.cc \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_refbox_log.pb.cc \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_wrapper_legacy.pb.cc \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_wrapper.pb.cc \
    ../Medusa/share/proto/cpp/vision_detection.pb.cc \
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
    ../Medusa/share/parammanager.cpp \
    src/test.cpp \
    src/communicator.cpp \
    src/radio/actionmodule.cpp \
    src/radio/crc.cpp \
    src/interaction4field.cpp \
    src/utils/globalsettings.cpp \
    src/simulator.cpp \
    ../Medusa/share/proto/cpp/grSim_Commands.pb.cc \
    ../Medusa/share/proto/cpp/grSim_Packet.pb.cc \
    ../Medusa/share/proto/cpp/grSim_Replacement.pb.cc \
    ../Medusa/share/proto/cpp/zss_cmd.pb.cc \
    ../Medusa/share/proto/cpp/zss_debug.pb.cc \
    src/refereebox.cpp \
    ../Medusa/share/proto/cpp/game_event.pb.cc \
    ../Medusa/share/proto/cpp/referee.pb.cc \
    src/paraminterface.cpp \
    src/debugger.cpp \
    src/documenthandler.cpp \
    src/vision/kalmanfilterdir.cpp

RESOURCES += \
    Athena.qrc

HEADERS += \
    src/field.h \
    src/vision/visionmodule.h \
    ../Medusa/share/singleton.hpp \
    ../Medusa/share/dataqueue.hpp \
    src/utils/globaldata.h \
    src/utils/singleparams.h \
    src/interaction.h \
    src/vision/messageformat.h \
    src/utils/staticparams.h \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_detection.pb.h \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_geometry_legacy.pb.h \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_geometry.pb.h \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_refbox_log.pb.h \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_wrapper_legacy.pb.h \
    ../Medusa/share/proto/cpp/messages_robocup_ssl_wrapper.pb.h \
    ../Medusa/share/proto/cpp/vision_detection.pb.h \
    src/vision/dealball.h \
    src/vision/dealrobot.h \
    src/vision/maintain.h \
    src/vision/collisiondetect.h \
    ../Medusa/share/geometry.h \
    src/vision/kalmanfilter.h \
    src/utils/matrix2d.h \
    graph/graph.h \
    graph/gridnode.h \
    graph/linenode.h \
    ../Medusa/share/parammanager.h \
    src/test.h \
    src/utils/translator.hpp \
    src/paraminterface.h \
    src/communicator.h \
    src/radio/actionmodule.h \
    src/radio/crc.h \
    src/interaction4field.h \
    src/utils/globalsettings.h \
    src/simulator.h \
    ../Medusa/share/proto/cpp/grSim_Commands.pb.h \
    ../Medusa/share/proto/cpp/grSim_Packet.pb.h \
    ../Medusa/share/proto/cpp/grSim_Replacement.pb.h \
    ../Medusa/share/proto/cpp/zss_cmd.pb.h \
    ../Medusa/share/proto/cpp/zss_debug.pb.h \
    src/refereebox.h \
    ../Medusa/share/proto/cpp/game_event.pb.h \
    ../Medusa/share/proto/cpp/referee.pb.h \
    src/debugger.h \
    src/documenthandler.h \
    src/vision/kalmanfilterdir.h

INCLUDEPATH += \
    $$PWD/src/utils \
    $$PWD/src/vision \
    $$PWD/../Medusa/share/proto/cpp \
    $$PWD/src/radio \
    $$PWD/src \
    ../Medusa/share

macx {
    PROTOBUF_INCLUDE_DIR = /usr/local/Cellar/protobuf/2.6.1/include
    LIBPROTOBUF = /usr/local/Cellar/protobuf/2.6.1/lib/libprotobuf.a
}

win32 {
    PROTOBUF_INCLUDE_DIR = C:\usr\local\protobuf\2.6.1\include
    CONFIG(release,debug|release){
        LIBPROTOBUF = C:\usr\local\protobuf\2.6.1\lib\vs14.0\libprotobuf.lib
    }
    CONFIG(debug,debug|release){
        LIBPROTOBUF = C:\usr\local\protobuf\2.6.1\lib\vs14.0\libprotobufD.lib
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
QMAKE_LFLAGS += -Wl,-rpath,"'$$ORIGIN'"

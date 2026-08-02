QT       += core gui websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    vps.cpp \
    vps_helpers.cpp \
    vps_script_testing.cpp \
    vps_scripts_base.cpp

HEADERS += \
    vps.h \
    vps_helpers.h \
    vps_script_testing.h \
    vps_scripts_base.h

FORMS += \
    vps.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    OBSPacketsExamples.txt

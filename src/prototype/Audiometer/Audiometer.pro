QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../auxiliary/CommandLineParser.cpp \
    ../../data/HearingMeasurement.cpp \
    ../../data/HearingTest.cpp \
    ../../data/MeasurementBase.cpp \
    ../../managers/ManagerBase.cpp \
    ../../managers/SerialPortManager.cpp \
    ../../managers/AudiometerManager.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    ../../auxiliary/CommandLineParser.h \
    ../../data/HearingTest.h \
    ../../data/MeasurementBase.h \
    ../../data/HearingMeasurement.h \
    ../../data/TestBase.h \
    ../../managers/ManagerBase.h \
    ../../managers/SerialPortManager.h \
    ../../managers/AudiometerManager.h \
    MainWindow.h

FORMS += \
    MainWindow.ui

TRANSLATIONS += \
    Audiometer_ec_CA.ts

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../auxiliary/CommandLineParser.cpp \
    ../../data/BodyCompositionMeasurement.cpp \
    ../../data/BodyCompositionTest.cpp \
    ../../data/MeasurementBase.cpp \
    ../../managers/BodyCompositionAnalyzerManager.cpp \
    ../../managers/ManagerBase.cpp \
    ../../managers/SerialPortManager.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    ../../auxiliary/CommandLineParser.h \
    ../../data/BodyCompositionMeasurement.h \
    ../../data/BodyCompositionTest.h \
    ../../data/MeasurementBase.h \
    ../../data/TestBase.h \
    ../../managers/BodyCompositionAnalyzerManager.h \
    ../../managers/ManagerBase.h \
    ../../managers/SerialPortManager.h \
    MainWindow.h

FORMS += \
    MainWindow.ui

TRANSLATIONS += \
    BodyComposition_en_CA.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

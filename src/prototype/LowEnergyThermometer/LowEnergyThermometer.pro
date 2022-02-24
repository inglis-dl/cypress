# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------
win32 {
TEMPLATE = app
TARGET = LowEnergyThermometer
DESTDIR = ../../../build/cypress/thermometer
CONFIG += debug
LIBS += -L"."
DEPENDPATH += .
MOC_DIR += .
OBJECTS_DIR += debug
UI_DIR += .
#RCC_DIR += GeneratedFiles
}

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets bluetooth

CONFIG += c++11 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../auxiliary/CommandLineParser.cpp \
    ../../auxiliary/CypressConstants.cpp \
    ../../auxiliary/BluetoothUtil.cpp \
    ../../data/MeasurementBase.cpp \
    ../../data/TemperatureMeasurement.cpp \
    ../../data/TemperatureTest.cpp \
    ../../managers/ManagerBase.cpp \
    ../../managers/BluetoothLEManager.cpp \
    ../../widgets/BarcodeWidget.cpp \
    MainWindow.cpp \
    main.cpp

HEADERS += \
    ../../auxiliary/CommandLineParser.h \
    ../../auxiliary/CypressConstants.h \
    ../../auxiliary/BluetoothUtil.h \
    ../../data/MeasurementBase.h \
    ../../data/TemperatureMeasurement.h \
    ../../data/TemperatureTest.h \
    ../../data/TestBase.h \
    ../../managers/ManagerBase.h \
    ../../managers/BluetoothLEManager.h \
    ../../widgets/BarcodeWidget.h \
    MainWindow.h

FORMS += \
    ../../widgets/barcodewidget.ui \
    MainWindow.ui

TRANSLATIONS += \
    LowEnergyThermometer_en_CA.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

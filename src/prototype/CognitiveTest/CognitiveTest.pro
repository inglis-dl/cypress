QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../auxiliary/CommandLineParser.cpp \
    ../../data/MeasurementBase.cpp \
    ../../data/ChoiceReactionMeasurement.cpp \
    ../../data/ChoiceReactionTest.cpp \
    ../../managers/ManagerBase.cpp \
    CognitiveTestManager.cpp \
    MainWindow.cpp \
    main.cpp

HEADERS += \
    ../../auxiliary/CommandLineParser.h \
    ../../data/MeasurementBase.h \
    ../../data/ChoiceReactionMeasurement.h \
    ../../data/ChoiceReactionTest.h \
    ../../data/TestBase.h \
    ../../managers/ManagerBase.h \
    CognitiveTestManager.h \
    MainWindow.h

FORMS += \
    MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

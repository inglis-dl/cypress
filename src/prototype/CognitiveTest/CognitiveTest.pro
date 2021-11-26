# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

TEMPLATE = app
TARGET = CognitiveTest
DESTDIR = ../../../../CypressBuilds/CognitiveTest
CONFIG += debug
DEFINES += _WINDOWS _UNICODE _ENABLE_EXTENDED_ALIGNED_STORAGE WIN64
LIBS += -L"../../../../../../../../../openssl/lib" \
    -L"../../../../../../../../../Utils/my_sql/mysql-5.7.25-winx64/lib" \
    -L"../../../../../../../../../Utils/postgresql/pgsql/lib" \
    -lshell32
DEPENDPATH += .
MOC_DIR += .
OBJECTS_DIR += debug
UI_DIR += .
RCC_DIR += GeneratedFiles
HEADERS += ../../data/ChoiceReactionMeasurement.h \
    ../../data/ChoiceReactionTest.h \
    ../../data/MeasurementBase.h \
    ../../data/TestBase.h \
    ./CognitiveTestManager.h \
    ../../auxiliary/CommandLineParser.h \
    ./MainWindow.h \
    ../../managers/ManagerBase.h
SOURCES += ../../data/ChoiceReactionMeasurement.cpp \
    ../../data/ChoiceReactionTest.cpp \
    ./CognitiveTestManager.cpp \
    ../../auxiliary/CommandLineParser.cpp \
    ./MainWindow.cpp \
    ../../managers/ManagerBase.cpp \
    ../../data/MeasurementBase.cpp \
    ./main.cpp
FORMS += ./MainWindow.ui

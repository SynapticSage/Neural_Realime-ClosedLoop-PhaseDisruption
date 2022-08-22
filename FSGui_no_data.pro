include(../module_defaults.pri)

QT += opengl widgets xml multimedia multimediawidgets

#CONFIG += debug

cache()

TARGET = FSGui
TEMPLATE = app

# Use Qt Resource System for images.
RESOURCES += \
    $$PWD/../../Resources/Images/buttons.qrc

CONFIG += c++11

INCLUDEPATH += ../../Trodes/src-config
INCLUDEPATH += ../../Trodes/src-main

SOURCES += main.cpp\
            fsgui.cpp \
            fsConfigureAOut.cpp \
            fsConfigureStimulators.cpp \
            fsStimForm.cpp \
            ../../Trodes/src-config/configuration.cpp \
            ../../Trodes/src-main/trodesSocket.cpp \
            ../../Trodes/src-main/trodesdatastructures.cpp \
            ../../Trodes/src-main/eventHandler.cpp \
    fsFeedbackTab.cpp \
    fsLatencyTab.cpp

HEADERS  += fsGUI.h \
            fsConfigureStimulators.h \
            fsConfigureAOut.h \
            fsStimForm.h \
#            fsLatencyGui.h \
            laser.h \
            fsSharedStimControlDefines.h \
            ../../Trodes/src-config/configuration.h \
            ../../Trodes/src-main/trodesSocket.h \
            ../../Trodes/src-main/trodesSocketDefines.h \
            ../../Trodes/src-main/trodesdatastructures.h \
            ../../Trodes/src-main/eventHandler.h \
    fsFeedbackTab.h \
    fsLatencyTab.h


include(../../module_defaults.pri)

QT       += core

QT       -= gui

TARGET = FSData
CONFIG   += console
CONFIG   -= app_bundle
QMAKE_CXXFLAGS += -std=gnu++11

TEMPLATE = app

INCLUDEPATH += ../../../Trodes/src-main/


## Primary FSDATA
SOURCES += \
    fsDataMain.cpp \
    fsSockets.cpp \
    fsProcessData.cpp \
    fsRTFilter.cpp \
    fsPhaseFilter.cpp

HEADERS += \
    fsDefines.h \
    stimControlGlobalVar.h \
    fsSocketDefines.h \
    ../../../Trodes/src-main/trodesSocketDefines.h \
    ../fsSharedStimControlDefines.h

## DSP Library
SOURCES += ../DSPFilters/Bessel.cpp \
    ../DSPFilters/Biquad.cpp \
    ../DSPFilters/Butterworth.cpp \
    ../DSPFilters/Cascade.cpp \
    ../DSPFilters/ChebyshevI.cpp \
    ../DSPFilters/ChebyshevII.cpp \
    ../DSPFilters/Custom.cpp \
    ../DSPFilters/Design.cpp \
    ../DSPFilters/Documentation.cpp \
    ../DSPFilters/Elliptic.cpp \
    ../DSPFilters/Filter.cpp \
    ../DSPFilters/Legendre.cpp \
    ../DSPFilters/Param.cpp \
    ../DSPFilters/PoleFilter.cpp \
    ../DSPFilters/RASTA.cpp \
    ../DSPFilters/RBJ.cpp \
    ../DSPFilters/RootFinder.cpp \
    ../DSPFilters/State.cpp
HEADERS += ../DSPFilters/Bessel.h \
    ../DSPFilters/Biquad.h \
    ../DSPFilters/Butterworth.h \
    ../DSPFilters/Cascade.h \
    ../DSPFilters/ChebyshevI.h \
    ../DSPFilters/ChebyshevII.h \
    ../DSPFilters/Common.h \
    ../DSPFilters/Custom.h \
    ../DSPFilters/Design.h \
    ../DSPFilters/Dsp.h \
    ../DSPFilters/Elliptic.h \
    ../DSPFilters/Filter.h \
    ../DSPFilters/Layout.h \
    ../DSPFilters/Legendre.h \
    ../DSPFilters/MathSupplement.h \
    ../DSPFilters/Params.h \
    ../DSPFilters/PoleFilter.h \
    ../DSPFilters/RASTA.h \
    ../DSPFilters/RBJ.h \
    ../DSPFilters/RootFinder.h \
    ../DSPFilters/SmoothedFilter.h \
    ../DSPFilters/State.h \
    ../DSPFilters/Types.h \
    ../DSPFilters/Utilities.h

DISTFILES += \
    phase

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    consumidor.cpp \
    fcfs.cpp \
    ganttchart.cpp \
    main.cpp \
    mainwindow.cpp \
    memorywindow.cpp \
    productor.cpp \
    sjf.cpp

HEADERS += \
    MemorySimulator.h \
    consumidor.h \
    fcfs.h \
    ganttchart.h \
    mainwindow.h \
    memorywindow.h \
    process.h \
    productor.h \
    sjf.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    ProyectoFinalSO_es_GT.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    accesstheme.cpp \
    cintaopciones.cpp \
    consultawidget.cpp \
    main.cpp \
    mainwindow.cpp \
    panelobjetos.cpp \
    pestanatabla.cpp \
    relacioneswidget.cpp \
    ribbongroup.cpp \
    ventanaprincipal.cpp \
    vistadisenio.cpp \
    vistahojadatos.cpp

HEADERS += \
    accesstheme.h \
    cintaopciones.h \
    consultawidget.h \
    mainwindow.h \
    panelobjetos.h \
    pestanatabla.h \
    relacioneswidget.h \
    ribbongroup.h \
    ventanaprincipal.h \
    vistadisenio.h \
    vistahojadatos.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    accesstheme.cpp \
    availlist.cpp \
    cintaopciones.cpp \
    consultawidget.cpp \
    main.cpp \
    mainwindow.cpp \
    packedrow.cpp \
    panelobjetos.cpp \
    pestanatabla.cpp \
    relacioneswidget.cpp \
    relationitem.cpp \
    ribbongroup.cpp \
    schema.cpp \
    tableitem.cpp \
    ventanaprincipal.cpp \
    vistadisenio.cpp \
    vistahojadatos.cpp

HEADERS += \
    accesstheme.h \
    availlist.h \
    cintaopciones.h \
    consultawidget.h \
    mainwindow.h \
    packedrow.h \
    panelobjetos.h \
    pestanatabla.h \
    relacioneswidget.h \
    relationitem.h \
    ribbongroup.h \
    schema.h \
    tableitem.h \
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

TEMPLATE = lib
TARGET = anthyplugin
QT += qml
CONFIG += plugin

# Anthy-unicode library
INCLUDEPATH += /usr/include/anthy-unicode-1.0
LIBS += -lanthy-unicode

# Plugin installation
uri = jp.anthy
TARGETPATH = $$[QT_INSTALL_QML]/$$replace(uri, \., /)

target.path = $$TARGETPATH
qmldir.files = qmldir
qmldir.path = $$TARGETPATH

INSTALLS += target qmldir

HEADERS += \
    anthyplugin.h

SOURCES += \
    anthyplugin.cpp

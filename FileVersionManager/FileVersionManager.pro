QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/filehasher.cpp \
    core/filewatcher.cpp \
    core/metadatamanager.cpp \
    core/versioninfo.cpp \
    core/versionmanager.cpp \
    main.cpp \
    storage/filestorage.cpp \
    storage/storage.cpp \
    ui/mainwindow.cpp \
    ui/versiontreemodel.cpp \
    utils/logger.cpp

HEADERS += \
    core/filehasher.h \
    core/filewatcher.h \
    core/metadatamanager.h \
    core/versioninfo.h \
    core/versionmanager.h \
    storage/filestorage.h \
    storage/storage.h \
    ui/mainwindow.h \
    ui/versiontreemodel.h \
    utils/logger.h

FORMS += \
    ui/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    renamedialog.cpp \
    selecteddirsdialog.cpp \
    settingdialog.cpp \
    transcoder.cpp \
    transcodeworker.cpp \
    utils/httpclient.cpp

HEADERS += \
    renamedialog.h \
    selecteddirsdialog.h \
    settingdialog.h \
    transcoder.h \
    transcodeworker.h \
    utils/httpclient.h

FORMS += \
    renamedialog.ui \
    selecteddirsdialog.ui \
    settingdialog.ui \
    transcoder.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

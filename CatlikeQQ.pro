QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += third_party/color_octree/\
    third_party/facile_menu/\
    third_party/interactive_buttons/\
    third_party/utils/\

SOURCES += \
    third_party/color_octree/coloroctree.cpp \
    third_party/color_octree/imageutil.cpp \
    third_party/facile_menu/facilemenu.cpp \
    third_party/facile_menu/facilemenuitem.cpp \
    global.cpp \
    third_party/interactive_buttons/interactivebuttonbase.cpp \
    main.cpp \
    mainwindow.cpp \
    notificationbubble.cpp \
    notificationcard.cpp \
    runtime.cpp \
    usersettings.cpp \
    third_party/utils/fileutil.cpp \
    third_party/utils/stringutil.cpp \
    third_party/utils/textinputdialog.cpp

HEADERS += \
    third_party/color_octree/coloroctree.h \
    third_party/third_party/color_octree/imageutil.h \
    third_party/facile_menu/facilemenu.h \
    third_party/facile_menu/facilemenuitem.h \
    global.h \
    third_party/interactive_buttons/interactivebuttonbase.h \
    mainwindow.h \
    notificationbubble.h \
    notificationcard.h \
    runtime.h \
    usersettings.h \
    third_party/utils/dlog.h \
    third_party/utils/fileutil.h \
    third_party/utils/myjson.h \
    third_party/utils/netutil.h \
    third_party/utils/pinyinutil.h \
    third_party/utils/stringutil.h \
    third_party/utils/textinputdialog.h \
    widgets/customtabstyle.h \
    widgets/lefttabwidget.h

FORMS += \
    mainwindow.ui \
    notificationbubble.ui \
    notificationcard.ui \
    third_party/utils/textinputdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    qxtglobalshortcut5/qxt.pri \
    resources/icons/account.png \
    resources/icons/application.png \
    resources/icons/auxiliary.png \
    resources/icons/port.png \
    resources/icons/settings.png

RESOURCES += \
    resources/resources.qrc

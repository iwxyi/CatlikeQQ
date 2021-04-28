QT       += core gui network websockets

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

DEFINES += ENABLE_TRAY

INCLUDEPATH += third_party/color_octree/\
    third_party/facile_menu/\
    third_party/interactive_buttons/\
    third_party/utils/\
    global/\
    notification/\
    web_service/

SOURCES += \
    global/usettings.cpp \
    notification/messageedit.cpp \
    web_service/cqhttpservice.cpp \
    third_party/color_octree/coloroctree.cpp \
    third_party/color_octree/imageutil.cpp \
    third_party/facile_menu/facilemenu.cpp \
    third_party/facile_menu/facilemenuitem.cpp \
    global/global.cpp \
    third_party/interactive_buttons/interactivebuttonbase.cpp \
    main.cpp \
    mainwindow.cpp \
    notification/notificationbubble.cpp \
    notification/notificationcard.cpp \
    global/runtime.cpp \
    third_party/utils/fileutil.cpp \
    third_party/utils/stringutil.cpp \
    third_party/utils/textinputdialog.cpp \
    widgets/settings/accountwidget.cpp \
    widgets/settings/bannerwidget.cpp \
    widgets/settings/debugwidget.cpp \
    widgets/settings/groupwidget.cpp

HEADERS += \
    global/accountinfo.h \
    global/signaltransfer.h \
    global/usettings.h \
    notification/messageedit.h \
    notification/replyedit.h \
    third_party/utils/mysettings.h \
    third_party/utils/netimageutil.h \
    web_service/cqhttpservice.h \
    third_party/color_octree/coloroctree.h \
    third_party/third_party/color_octree/imageutil.h \
    third_party/facile_menu/facilemenu.h \
    third_party/facile_menu/facilemenuitem.h \
    global/global.h \
    third_party/interactive_buttons/interactivebuttonbase.h \
    mainwindow.h \
    notification/notificationbubble.h \
    notification/notificationcard.h \
    global/runtime.h \
    third_party/utils/dlog.h \
    third_party/utils/fileutil.h \
    third_party/utils/myjson.h \
    third_party/utils/netutil.h \
    third_party/utils/pinyinutil.h \
    third_party/utils/stringutil.h \
    third_party/utils/textinputdialog.h \
    web_service/msgbean.h \
    widgets/customtabstyle.h \
    widgets/lefttabwidget.h \
    widgets/settings/accountwidget.h \
    widgets/settings/bannerwidget.h \
    widgets/settings/debugwidget.h \
    widgets/settings/groupwidget.h

FORMS += \
    mainwindow.ui \
    notification/notificationbubble.ui \
    notification/notificationcard.ui \
    third_party/utils/textinputdialog.ui \
    widgets/settings/accountwidget.ui \
    widgets/settings/bannerwidget.ui \
    widgets/settings/debugwidget.ui \
    widgets/settings/groupwidget.ui

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

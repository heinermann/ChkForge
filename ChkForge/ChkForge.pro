QT       += core gui

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

SOURCES += \
    doodadpalette.cpp \
    exportimage.cpp \
    exportsections.cpp \
    filemanager.cpp \
    importsections.cpp \
    itempalette.cpp \
    itemtree.cpp \
    main.cpp \
    mainwindow.cpp \
    mapview.cpp \
    minimap.cpp \
    newmap.cpp \
    outputwindow.cpp \
    previewpainter.cpp \
    recenttiles.cpp \
    savemap.cpp \
    scenariosettings.cpp \
    stringmanager.cpp \
    terrainbrush.cpp \
    tilepalette.cpp

HEADERS += \
    dockwidgetwrapper.h \
    doodadpalette.h \
    exportimage.h \
    exportsections.h \
    filemanager.h \
    importsections.h \
    itempalette.h \
    itemtree.h \
    mainwindow.h \
    mapview.h \
    minimap.h \
    newmap.h \
    outputwindow.h \
    previewpainter.h \
    recenttiles.h \
    savemap.h \
    scenariosettings.h \
    stringmanager.h \
    terrainbrush.h \
    tilepalette.h

FORMS += \
    doodadpalette.ui \
    exportimage.ui \
    exportsections.ui \
    filemanager.ui \
    importsections.ui \
    itempalette.ui \
    itemtree.ui \
    mainwindow.ui \
    mapview.ui \
    minimap.ui \
    newmap.ui \
    outputwindow.ui \
    previewpainter.ui \
    recenttiles.ui \
    savemap.ui \
    scenariosettings.ui \
    stringmanager.ui \
    terrainbrush.ui \
    tilepalette.ui

TRANSLATIONS += \
    ChkForge_en_CA.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../ads/lib/ -lqtadvanceddocking
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../ads/lib/ -lqtadvanceddockingd
else:unix: LIBS += -L$$OUT_PWD/../ads/lib/ -lqtadvanceddocking

INCLUDEPATH += $$PWD/../ads/src
DEPENDPATH += $$PWD/../ads/src

DISTFILES +=

RESOURCES += \
    ChkForge.qrc \
    Icons.qrc

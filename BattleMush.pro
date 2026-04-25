QT       += core gui

RC_ICONS = assets/images/icons/abc.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#保护版本
DEFINES += PROTECT=TRUE

OPENCV_PATH = D:/CodeRepos/3th/opencv/           #opencv目录

INCLUDEPATH +=  $${OPENCV_PATH}/include

CONFIG(debug, debug|release) {
    LIBS += $${OPENCV_PATH}/x64/vc16/lib/opencv_core460d.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_imgproc460d.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_imgcodecs460d.lib \
}

# Release 模式下的配置
CONFIG(release, debug|release) {
    LIBS += $${OPENCV_PATH}/x64/vc16/lib/opencv_core460.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_imgproc460.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_imgcodecs460.lib \
}

CONFIG(debug, debug|release) {
    LIBS += $$PWD/lib/debug/MSDriverPRO.lib
    DEFINES += ISDEBUG= TRUE
} else {
    LIBS += $$PWD/lib/MSDriverPRO.lib
}

LIBS += $$PWD/lib/vmp/VMProtectSDK64.lib
LIBS += Advapi32.lib
LIBS += -lSetupAPI
LIBS += -lUser32 -lKernel32

SOURCES += \
    driver/mem.cpp \
    global/controller.cpp \
    global/globalconfig.cpp \
    global/globalplayer.cpp \
    driver/driverload.cpp \
    helper/rand.cpp \
    keyboard/nativeeventfilter.cpp \
    logviewer.cpp \
    main.cpp \
    mainwindow.cpp \
    pathfinding/moveplatform.cpp \
    pathfinding/walkbase.cpp \
    process/process.cpp \
    threads/healthmonitor.cpp \
    threads/iworker.cpp \
    threads/killmonster.cpp \
    threads/monitortooltest.cpp \
    threads/threadmanager.cpp \
    threads/updategamedata.cpp \
    utils/config.cpp \
    utils/data.cpp \
    utils/gamestruct.cpp \
    utils/helper.cpp \
    utils/mapserializer.cpp \
    utils/matchtemp.cpp \
    widgets/alertwindow.cpp \
    widgets/hotkeyedit.cpp \
    widgets/noticewidget.cpp \
    widgets/taskeditdialog.cpp

HEADERS += \
    driver/mem.h \
    driver/mskeycode.h \
    global/controller.h \
    global/gameoffset.h \
    global/globalconfig.h \
    global/globalplayer.h \
    driver/driverload.h \
    driver/msdriverpro.h \
    driver/msstruct.h \
    global/struct.h \
    helper/rand.h \
    keyboard/nativeeventfilter.h \
    logviewer.h \
    mainwindow.h \
    pathfinding/moveplatform.h \
    pathfinding/walkbase.h \
    process/process.h \
    threads/healthmonitor.h \
    threads/iworker.h \
    threads/killmonster.h \
    threads/monitortooltest.h \
    threads/threadmanager.h \
    threads/updategamedata.h \
    utils/config.h \
    utils/data.h \
    utils/gamestruct.h \
    utils/helper.h \
    utils/mapserializer.h \
    utils/matchtemp.h \
    vmp/VMProtectSDK.h \
    widgets/alertwindow.h \
    widgets/hotkeyedit.h \
    widgets/noticewidget.h \
    widgets/taskeditdialog.h

FORMS += \
    TaskEditDialog.ui \
    mainwindow.ui


# 复制DLL文件
message($$OUT_PWD)
CONFIG(debug, debug|release) {
    COM_DLL_FILES = $$files($$PWD/dll/*.dll)
    com_dll_files.files = $$COM_DLL_FILES
    com_dll_files.path = $$OUT_PWD/debug/
    COPIES += com_dll_files

    DEBUG_DLL_FILES = $$files($$PWD/dll/debug/*.dll)
    debug_dll_files.files = $$DEBUG_DLL_FILES
    debug_dll_files.path = $$OUT_PWD/debug/
    COPIES += debug_dll_files

}else{
    COM_DLL_FILES = $$files($$PWD/dll/*.dll)
    com_dll_files.files = $$COM_DLL_FILES
    com_dll_files.path = $$OUT_PWD/release/
    COPIES += com_dll_files

    RELEASE_DLL_FILES = $$files($$PWD/dll/release/*.dll)
    release_dll_files.files = $$RELEASE_DLL_FILES
    release_dll_files.path = $$OUT_PWD/release/
    COPIES += release_dll_files
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

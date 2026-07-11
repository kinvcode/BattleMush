QT       += core gui network concurrent

RC_ICONS = assets/images/icons/abc.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#保护版本
# DEFINES += PROTECT=TRUE

# DEFINES += VERIFY=TRUE

DEFINES += APP_VERSION=4

CONFIG(debug, debug|release) {
    DEFINES += API_URL=\"http://www.battle.test/api/\"
    DEFINES += ISDEBUG= TRUE
} else {
    DEFINES += API_URL=\"http://battle.kinvcode.cn/api/\"
}

OPENCV_PATH = D:/CodeRepos/3th/opencv/           #opencv目录
ONNXRUNTIME_DIR = D:/CodeRepos/3th/onnxruntime-win-x64-1.23.2/ #onnxruntime v1.23.2 https://github.com/microsoft/onnxruntime/releases

INCLUDEPATH +=  $${OPENCV_PATH}/include \
                $$ONNXRUNTIME_DIR/include \
                $$PYTHON_BASE/include \
                $$PWD/include \

CONFIG(debug, debug|release) {
    LIBS += $${OPENCV_PATH}/x64/vc16/lib/opencv_core460d.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_imgproc460d.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_imgcodecs460d.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_videoio460d.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_dnn460d.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_video460d.lib \
            -L$$ONNXRUNTIME_DIR/lib -lonnxruntime
}

# Release 模式下的配置
CONFIG(release, debug|release) {
    LIBS += $${OPENCV_PATH}/x64/vc16/lib/opencv_core460.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_imgproc460.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_imgcodecs460.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_videoio460.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_dnn460.lib \
            $${OPENCV_PATH}/x64/vc16/lib/opencv_video460.lib \
            -L$$ONNXRUNTIME_DIR/lib -lonnxruntime
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
LIBS += $$PWD/lib/msdk/msdkLib.lib
LIBS += $$PWD/lib/msdk/hid.lib
LIBS += $$PWD/lib/openssl/libssl.lib
LIBS += $$PWD/lib/openssl/libcrypto.lib

LIBS += setupapi.lib \
        gdi32.lib \
        winspool.lib \
        comdlg32.lib \
        shell32.lib \
        ole32.lib \
        oleaut32.lib \
        uuid.lib \
        odbc32.lib \
        odbccp32.lib \
        legacy_stdio_definitions.lib \
        dbghelp.lib

SOURCES += \
    driver/mem.cpp \
    driver/msdk/msdkDriver.cpp \
    global/controller.cpp \
    global/globalconfig.cpp \
    global/globalplayer.cpp \
    driver/driverload.cpp \
    helper/rand.cpp \
    http/apirequest.cpp \
    http/request.cpp \
    http/requestsync.cpp \
    keyboard/nativeeventfilter.cpp \
    login.cpp \
    logviewer.cpp \
    main.cpp \
    mainwindow.cpp \
    pathfinding/moveplatform.cpp \
    pathfinding/walkbase.cpp \
    process/exception.cpp \
    process/process.cpp \
    threads/healthmonitor.cpp \
    threads/iworker.cpp \
    threads/killmonster.cpp \
    threads/monitortooltest.cpp \
    threads/mousemovehandle.cpp \
    threads/threadmanager.cpp \
    threads/updategamedata.cpp \
    utils/config.cpp \
    utils/data.cpp \
    utils/decrypt.cpp \
    utils/gamestruct.cpp \
    utils/hardware.cpp \
    utils/helper.cpp \
    utils/mapserializer.cpp \
    utils/matchtemp.cpp \
    utils/mousetrack.cpp \
    utils/realtracker.cpp \
    widgets/alertwindow.cpp \
    widgets/hotkeyedit.cpp \
    widgets/noticewidget.cpp \
    widgets/taskeditdialog.cpp

HEADERS += \
    driver/mem.h \
    driver/msdk/msdk.h \
    driver/msdk/msdkDriver.h \
    driver/msdk/usbhidkeycode.h \
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
    http/apirequest.h \
    http/request.h \
    http/requestsync.h \
    keyboard/nativeeventfilter.h \
    login.h \
    logviewer.h \
    mainwindow.h \
    pathfinding/moveplatform.h \
    pathfinding/walkbase.h \
    process/exception.h \
    process/process.h \
    threads/healthmonitor.h \
    threads/iworker.h \
    threads/killmonster.h \
    threads/monitortooltest.h \
    threads/mousemovehandle.h \
    threads/threadmanager.h \
    threads/updategamedata.h \
    utils/config.h \
    utils/data.h \
    utils/decrypt.h \
    utils/gamestruct.h \
    utils/hardware.h \
    utils/helper.h \
    utils/mapserializer.h \
    utils/matchtemp.h \
    utils/mousetrack.h \
    utils/realtracker.h \
    vmp/VMProtectSDK.h \
    widgets/alertwindow.h \
    widgets/hotkeyedit.h \
    widgets/noticewidget.h \
    widgets/taskeditdialog.h

FORMS += \
    TaskEditDialog.ui \
    login.ui \
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

QT       += core gui

RC_ICONS = assets/images/icons/abc.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

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
    driver/driverload.cpp \
    helper/rand.cpp \
    logviewer.cpp \
    main.cpp \
    mainwindow.cpp \
    noticewidget.cpp \
    process/process.cpp

HEADERS += \
    driver/driverload.h \
    driver/msdriverpro.h \
    driver/msstruct.h \
    helper/rand.h \
    logviewer.h \
    mainwindow.h \
    noticewidget.h \
    process/process.h \
    vmp/VMProtectSDK.h

FORMS += \
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

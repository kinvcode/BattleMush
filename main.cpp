#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QSharedMemory>
#include "keyboard/nativeeventfilter.h"
#include <windows.h>
#include <QMessageBox>
#include <QDebug>
#include "global/controller.h"
#include "process/process.h"
#include "driver/msdk/msdkDriver.h"
#include "driver/PeaceDriverAPI.h"
#include "driver/peacedriverload.h"
#include "vmp/VMProtectSDK.h"
#include <dbghelp.h>
#include "process/exception.h"
#include "login.h"

void setStyle(const QString &style);
void loadDriver();
bool loadPeaceDRV();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 使用QSharedMemory来实现互斥锁，禁止重复运行
    QSharedMemory sharedMemory;
    sharedMemory.setKey("BattleMushKey");
    if (!sharedMemory.create(1)) {
        return 1;
    }

    SetUnhandledExceptionFilter(BattleCrashHandler);

    QString rand_tile = "";
#ifdef PROTECT
    // 自身程序重命名
    ProcessHelper::renameSelf();

    // 删除自身
    ProcessHelper::deleteSelf();
#endif

    // 设置全局样式表
    setStyle(":assets/qss/com.qss");

    // 初始化地图列表
    Controller::initTaskList();

    // 初始化地图节点
    Controller::initRoomNodes();

    // 主窗口
    MainWindow* mainWindow = MainWindow::getInstance();

    NativeEventFilter filter(mainWindow);
    a.installNativeEventFilter(&filter);

// #ifdef VERIFY
    // 加载窗口
    Login login_pannel;
    login_pannel.show();

    QObject::connect(&login_pannel, &Login::processingFinished, [&](){
        login_pannel.close();

        // 初始化组件
        mainWindow->initAll();
        mainWindow->show();

        RegisterHotKey(NULL, VK_F1, NULL, VK_F1);
        RegisterHotKey(NULL, VK_F2, NULL, VK_F2);
        RegisterHotKey(NULL, VK_F3, NULL, VK_F3);
    });
// #else
//     mainWindow->initAll();
//     mainWindow->show();
//     // 注册热键
//     RegisterHotKey(NULL, VK_F1, NULL, VK_F1);
//     RegisterHotKey(NULL, VK_F2, NULL, VK_F2);
//     RegisterHotKey(NULL, VK_F3, NULL, VK_F3);
// #endif

    // 组件更新
    QObject::connect(&filter, &NativeEventFilter::sendControleStatus, mainWindow, &MainWindow::updateCompoment);

    // 连接 aboutToQuit 信号到槽函数
    QObject::connect(&a, &QApplication::aboutToQuit, [mainWindow]() {
        mainWindow->onApplicationQuit();
    });

    // 启动驱动
    loadDriver();

    return a.exec();
}

void setStyle(const QString &style) {
    QFile qss(style);
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();
}

void loadDriver()
{
    VMProtectBegin(__FUNCSIG__);

    // 检测驱动是否已开启
    if(!PeaceDriver::pingREG()){
        // 初始化驱动
        bool driver_success = loadPeaceDRV();
        if(!driver_success)
        {
            qApp->quit();
            return;
        }
    }
    // 保护自身
    PeaceDriver::protectPid((ULONG64)GetCurrentProcessId());

// 伪装进程
#ifdef PROTECT
    // 隐藏进程
    int csrss_pid = ProcessHelper::getProcessPID("csrss.exe");
    PeaceDriver::fakeProcess((ULONG64)GetCurrentProcessId(), csrss_pid);
#endif

    qDebug() << "驱动初始化完毕！";

    VMProtectEnd();
}

bool loadPeaceDRV()
{
    PeaceDriverLoad drv;
    if (!drv.driverIsLoaded()) {
        // 加载驱动
        PeaceDriverLoad::LoadDriverResult load_result = drv.loadPeaceDriver();
        if (!load_result.success) {
            qDebug() << "VT驱动安装失败";
            qDebug() << (QString::fromWCharArray(load_result.message));
            QMessageBox::information(NULL, "BattleMush", QString::fromWCharArray(load_result.message));
            return false;
        }
    }

    if (drv.driverIsLoaded())
        return true;

    return false;
}

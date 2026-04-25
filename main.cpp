#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include "keyboard/nativeeventfilter.h"
#include <windows.h>
#include "global/controller.h"

void setStyle(const QString &style);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    // 设置全局样式表
    setStyle(":assets/qss/com.qss");

    // 初始化地图列表
    Controller::initTaskList();

    // 初始化地图节点
    Controller::initRoomNodes();

    // 主窗口
    MainWindow* mainWindow =MainWindow::getInstance();
    mainWindow->show();

    // 注册热键
    NativeEventFilter filter(mainWindow);
    a.installNativeEventFilter(&filter);
    RegisterHotKey(NULL, VK_F1, NULL, VK_F1);
    RegisterHotKey(NULL, VK_F2, NULL, VK_F2);

    // 连接 aboutToQuit 信号到槽函数
    QObject::connect(&a, &QApplication::aboutToQuit, [mainWindow]() {
        mainWindow->onApplicationQuit();
    });

    // 组件更新
    QObject::connect(&filter, &NativeEventFilter::sendControleStatus, mainWindow, &MainWindow::updateCompoment);

    return a.exec();
}

void setStyle(const QString &style) {
    QFile qss(style);
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();
}


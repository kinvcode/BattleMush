#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include "keyboard/nativeeventfilter.h"
#include <windows.h>

void setStyle(const QString &style);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    // 设置全局样式表
    setStyle(":assets/qss/com.qss");

    // 主窗口
    MainWindow mainWindow;
    mainWindow.show();

    // 注册热键
    NativeEventFilter filter(&mainWindow);
    a.installNativeEventFilter(&filter);
    RegisterHotKey(NULL, VK_F1, NULL, VK_F1);

    return a.exec();
}

void setStyle(const QString &style) {
    QFile qss(style);
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();
}


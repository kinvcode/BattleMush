#include "mainwindow.h"

#include <QApplication>
#include <QFile>

void setStyle(const QString &style);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    // 设置全局样式表
    setStyle(":assets/qss/com.qss");

    MainWindow w;
    w.show();


    return a.exec();
}

void setStyle(const QString &style) {
    QFile qss(style);
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();
}


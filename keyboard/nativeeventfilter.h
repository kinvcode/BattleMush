#pragma once
#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QAbstractNativeEventFilter>
#include <QDebug>
#include <QObject>
#include "mainwindow.h"
#include "vmp/VMProtectSDK.h"

#define HOT_KEY_HOME 36

class NativeEventFilter: public QObject,public QAbstractNativeEventFilter
{
    Q_OBJECT
private:
    MainWindow* main_win;
    // 脚本是否暂停
    bool m_is_stop = true;
public:
    explicit NativeEventFilter(MainWindow* window);
    bool nativeEventFilter(const QByteArray& eventType, void* message, long long* result);
    //
signals:
    void sendMsg(QString msg);
};

#endif // NATIVEEVENTFILTER_H

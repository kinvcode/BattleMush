// ThreadManager.h
#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include "threads/iworker.h"

class ThreadManager : public QObject
{
    Q_OBJECT
public:
    struct ThreadInfo {
        QThread* thread;
        IWorker* worker;
        bool isRunning;
        QString workerType;
    };

    explicit ThreadManager(QObject *parent = nullptr);
    ~ThreadManager();

    // 添加不同类型的线程
    bool addThread(int threadId, const QString& workerType);
    bool addThread(int threadId, IWorker* worker);

    // 移除线程
    bool removeThread(int threadId);

    // 启动所有线程
    void startAll();

    // 停止所有线程
    void stopAll();

    // 暂停所有线程
    void pauseAll();

    // 恢复所有线程
    void resumeAll();

    // 启动指定线程
    bool startThread(int threadId);

    // 停止指定线程
    bool stopThread(int threadId);

    // 暂停/恢复指定线程
    bool pauseThread(int threadId);
    bool resumeThread(int threadId);

    // 查询状态
    bool isThreadRunning(int threadId) const;
    int threadCount() const { return m_threads.size(); }
    QStringList getRunningThreads() const;

signals:
    // 线程事件信号
    void threadStarted(int threadId, const QString& workerType);
    void threadStopped(int threadId);
    void threadFinished(int threadId);
    void threadProgress(int threadId, int value);
    void threadError(int threadId, const QString& message);

    // 批量事件信号
    void allThreadsStarted();
    void allThreadsStopped();
    void allThreadsFinished();

private slots:
    void onWorkerProgress(int workerId, int value);
    void onWorkerFinished(int workerId);
    void onWorkerError(int workerId, const QString& message);
    void onThreadFinished();

private:
    IWorker* createWorker(const QString& workerType, int id);
    void cleanupThread(int threadId);

    QMap<int, ThreadInfo> m_threads;
    QAtomicInt m_runningCount;
    mutable QRecursiveMutex m_mutex;
};

#endif // THREADMANAGER_H

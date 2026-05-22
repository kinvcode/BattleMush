// ThreadManager.cpp
#include "ThreadManager.h"
#include <QCoreApplication>
#include "threads/healthmonitor.h"
#include "threads/updategamedata.h"
#include "threads/monitortooltest.h"
#include "threads/killmonster.h"
#include "threads/mousemovehandle.h"

ThreadManager::ThreadManager(QObject *parent)
    : QObject(parent)
    , m_runningCount(0)
{
    qDebug() << "ThreadManager created";
}

ThreadManager::~ThreadManager()
{
    qDebug() << "ThreadManager destructor - stopping all threads";
    stopAll();

    QMutexLocker locker(&m_mutex);
    for (auto it = m_threads.begin(); it != m_threads.end(); ++it) {
        cleanupThread(it.key());
    }
    m_threads.clear();
}

bool ThreadManager::addThread(int threadId, const QString& workerType)
{
    QMutexLocker locker(&m_mutex);

    if (m_threads.contains(threadId)) {
        qWarning() << "Thread" << threadId << "already exists";
        return false;
    }

    IWorker* worker = createWorker(workerType, threadId);
    if (!worker) {
        qWarning() << "Failed to create worker of type:" << workerType;
        return false;
    }

    return addThread(threadId, worker);
}

bool ThreadManager::addThread(int threadId, IWorker* worker)
{
    QMutexLocker locker(&m_mutex);

    if (m_threads.contains(threadId)) {
        qWarning() << "Thread" << threadId << "already exists";
        delete worker;
        return false;
    }

    // 创建线程
    QThread* thread = new QThread(this);

    // 移动工作对象到线程
    worker->moveToThread(thread);

    // 连接信号槽
    connect(thread, &QThread::started, worker, &IWorker::start);
    connect(worker, &IWorker::progress, this, &ThreadManager::onWorkerProgress);
    connect(worker, &IWorker::finished, this, &ThreadManager::onWorkerFinished);
    connect(worker, &IWorker::error, this, &ThreadManager::onWorkerError);
    connect(worker, &IWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, &ThreadManager::onThreadFinished);

    // 存储信息
    ThreadInfo info;
    info.thread = thread;
    info.worker = worker;
    info.isRunning = false;
    info.workerType = worker->metaObject()->className();

    m_threads[threadId] = info;

    qDebug() << "Thread" << threadId << "added with worker type:" << info.workerType;
    return true;
}

IWorker* ThreadManager::createWorker(const QString& workerType, int id)
{
    if (workerType == "HealthMonitor") {
        return new HealthMonitor(id);
    } else if (workerType == "UpdateGameData") {
        return new UpdateGameData(id);
    } else if(workerType == "MonitorToolTest"){
        return new MonitorToolTest(id);
    } else if(workerType == "KillMonster"){
        return new KillMonster(id);
    } else if(workerType == "MouseMoveHandle"){
        return new MouseMoveHandle(id);
    }
    return nullptr;
}

bool ThreadManager::removeThread(int threadId)
{
    QMutexLocker locker(&m_mutex);

    if (!m_threads.contains(threadId)) {
        qWarning() << "Thread" << threadId << "not found";
        return false;
    }

    cleanupThread(threadId);
    m_threads.remove(threadId);

    qDebug() << "Thread" << threadId << "removed";
    return true;
}

void ThreadManager::cleanupThread(int threadId)
{
    ThreadInfo& info = m_threads[threadId];

    if (info.isRunning) {
        if (info.worker) {
            info.worker->stop();
        }
        if (info.thread) {
            info.thread->quit();
            info.thread->wait(3000);
        }
    }

    if (info.thread) {
        delete info.thread;
        info.thread = nullptr;
    }
}

void ThreadManager::startAll()
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "Starting all threads...";
    for (auto it = m_threads.begin(); it != m_threads.end(); ++it) {
        startThread(it.key());
    }
}

void ThreadManager::stopAll()
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "Stopping all threads...";
    for (auto it = m_threads.begin(); it != m_threads.end(); ++it) {
        stopThread(it.key());
    }
}

void ThreadManager::pauseAll()
{
    QMutexLocker locker(&m_mutex);
    for (auto it = m_threads.begin(); it != m_threads.end(); ++it) {
        if (!m_threads.contains(it.key())) {
            continue;
        }

        ThreadInfo& info = m_threads[it.key()];
        if (info.worker && info.isRunning) {
            info.worker->pause();
            continue;
        }
    }
}

void ThreadManager::resumeAll()
{
    QMutexLocker locker(&m_mutex);
    for (auto it = m_threads.begin(); it != m_threads.end(); ++it) {
        if (!m_threads.contains(it.key())) {
            continue;
        }

        ThreadInfo& info = m_threads[it.key()];
        if (info.worker && info.isRunning) {
            info.worker->resume();
            continue;
        }
    }
}

bool ThreadManager::startThread(int threadId)
{
    QMutexLocker locker(&m_mutex);

    if (!m_threads.contains(threadId)) {
        qWarning() << "Thread" << threadId << "not found";
        return false;
    }

    ThreadInfo& info = m_threads[threadId];

    if (info.isRunning) {
        qWarning() << "Thread" << threadId << "already running";
        return false;
    }

    if (!info.thread) {
        qWarning() << "Thread" << threadId << "has no thread object";
        return false;
    }

    info.isRunning = true;
    m_runningCount.ref();
    info.thread->start();

    emit threadStarted(threadId, info.workerType);
    qDebug() << "Thread" << threadId << "(" << info.workerType << ") started";

    return true;
}

bool ThreadManager::stopThread(int threadId)
{
    QMutexLocker locker(&m_mutex);

    if (!m_threads.contains(threadId)) {
        qWarning() << "Thread" << threadId << "not found";
        return false;
    }

    ThreadInfo& info = m_threads[threadId];

    if (!info.isRunning) {
        qWarning() << "Thread" << threadId << "not running";
        return false;
    }

    if (info.worker) {
        info.worker->stop();
    }

    return true;
}

bool ThreadManager::pauseThread(int threadId)
{
    QMutexLocker locker(&m_mutex);

    if (!m_threads.contains(threadId)) {
        return false;
    }

    ThreadInfo& info = m_threads[threadId];
    if (info.worker && info.isRunning) {
        info.worker->pause();
        return true;
    }

    return false;
}

bool ThreadManager::resumeThread(int threadId)
{
    QMutexLocker locker(&m_mutex);

    if (!m_threads.contains(threadId)) {
        return false;
    }

    ThreadInfo& info = m_threads[threadId];
    if (info.worker && info.isRunning) {
        info.worker->resume();
        return true;
    }

    return false;
}

bool ThreadManager::isThreadRunning(int threadId) const
{
    QMutexLocker locker(&m_mutex);

    if (!m_threads.contains(threadId)) {
        return false;
    }

    return m_threads[threadId].isRunning;
}

QStringList ThreadManager::getRunningThreads() const
{
    QMutexLocker locker(&m_mutex);

    QStringList running;
    for (auto it = m_threads.begin(); it != m_threads.end(); ++it) {
        if (it.value().isRunning) {
            running << QString("Thread %1 (%2)").arg(it.key()).arg(it.value().workerType);
        }
    }
    return running;
}

void ThreadManager::onWorkerProgress(int workerId, int value)
{
    emit threadProgress(workerId, value);
    qDebug() << "Thread" << workerId << "progress:" << value << "%";
}

void ThreadManager::onWorkerFinished(int workerId)
{
    QMutexLocker locker(&m_mutex);

    if (!m_threads.contains(workerId)) {
        return;
    }

    ThreadInfo& info = m_threads[workerId];
    info.isRunning = false;
    m_runningCount.deref();

    emit threadFinished(workerId);
    qDebug() << "Thread" << workerId << "finished";

    // if (m_runningCount.load() == 0) {
    //     emit allThreadsFinished();
    //     qDebug() << "All threads finished";
    // }
}

void ThreadManager::onWorkerError(int workerId, const QString& message)
{
    emit threadError(workerId, message);
    qDebug() << "Thread" << workerId << "error:" << message;
}

void ThreadManager::onThreadFinished()
{
    qDebug() << "Thread finished";
}

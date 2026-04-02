#include "updategamedata.h"
#include "driver/msdriverpro.h"
#include "global/globalplayer.h"
#include <QDebug>
#include "process/process.h"
#include "utils/helper.h"

UpdateGameData::UpdateGameData(int id, QObject *parent)
    : IWorker(id, parent)
    , m_running(false)
    , m_paused(false)
    , m_counter(0)
{
    qDebug() << "UpdateGameData" << m_id;
}

UpdateGameData::~UpdateGameData()
{
    qDebug() << "UpdateGameData" << m_id;
}

void UpdateGameData::start()
{
    qDebug() << "UpdateGameData" << m_id << "started in thread:" << QThread::currentThread();
    m_running = true;
    m_paused = false;
    m_counter = 0;
    process();
}

void UpdateGameData::stop()
{
    qDebug() << "UpdateGameData" << m_id << "stop requested";
    m_running = false;
    m_paused = false;
}

void UpdateGameData::pause()
{
    qDebug() << "UpdateGameData" << m_id << "paused";
    m_paused = true;
}

void UpdateGameData::resume()
{
    qDebug() << "UpdateGameData" << m_id << "resumed";
    m_paused = false;
}

void UpdateGameData::process()
{
    while (m_running) {
        // 处理暂停状态
        while (m_paused && m_running) {
            QThread::msleep(10);  // 等待恢复
        }

        if (!m_running) break;


        // 检测游戏是否存在
        Gplayer.p_gamepid = MSDriverPRO::getPid("msw.exe");

        if(Gplayer.p_gamepid == 0){
            // qDebug << "";
            DebugOutput::out("游戏进程不存在");

            ProcessHelper::programDelay(3000);
            QApplication::processEvents();
            continue;
        }

        // DebugOutput::out("游戏进程:" + QString::number(Gplayer.p_gamepid));

        ProcessHelper::programDelay(3000);
        QApplication::processEvents();
    }

    qDebug() << "UpdateGameData" << m_id << "finished";
}

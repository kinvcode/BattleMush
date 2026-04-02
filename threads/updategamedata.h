#ifndef UPDATEGAMEDATA_H
#define UPDATEGAMEDATA_H
#include "threads/iworker.h"

#include <QThread>

class UpdateGameData : public IWorker
{
    Q_OBJECT
public:
    explicit UpdateGameData(int id, QObject *parent = nullptr);
    virtual ~UpdateGameData();
public slots:
    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
private:
    void process();

    bool m_running;
    bool m_paused;
    int m_counter;
};

#endif // UPDATEGAMEDATA_H

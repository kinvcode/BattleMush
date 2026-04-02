#ifndef IWORKER_H
#define IWORKER_H

#include <QObject>

class IWorker : public QObject
{
    Q_OBJECT
public:
    explicit IWorker(int id, QObject *parent = nullptr) : QObject(parent), m_id(id) {}
    virtual ~IWorker() {}

    int id() const { return m_id; }

public slots:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

signals:
    void progress(int workerId, int value);
    void finished(int workerId);
    void error(int workerId, const QString& message);

protected:
    int m_id;
};

#endif // IWORKER_H

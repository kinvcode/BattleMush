#ifndef GAMESTRUCT_H
#define GAMESTRUCT_H

#include "global/struct.h"
#include "global/gameoffset.h"
#include "driver/mem.h"

class GameStruct
{
public:
    static QString readEntityName(quint64 entity);

    static QString readEntityID(quint64 entity);

    static QString readEntityText(quint64 entity);

    static void printEntityChild(quint64 entity, int depth = 0);

    static QJsonObject buildJSONEntityChild(quint64 entity, std::unordered_set<quint64> &entity_set, int depth = 0);
};

#endif // GAMESTRUCT_H

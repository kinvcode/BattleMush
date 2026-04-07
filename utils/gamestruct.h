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
};

#endif // GAMESTRUCT_H

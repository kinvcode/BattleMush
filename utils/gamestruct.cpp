#include "gamestruct.h"

using namespace Mem;

QString GameStruct::readEntityName(quint64 entity)
{
    QString name = "";
    quint64 name_p = readLong(entity + BASE::MODEntity::name);
    if(!name_p){
        return name;
    }
    return readString(name_p + BASE::String::text, 100);
}

QString GameStruct::readEntityID(quint64 entity)
{
    QString name = "";
    quint64 name_p = readLong(entity + BASE::MODEntity::_entityId);
    if(!name_p){
        return name;
    }
    return readString(name_p + BASE::String::text, 100);
}

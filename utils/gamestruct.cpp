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

QString GameStruct::readEntityText(quint64 entity)
{
    QString text = "";
    quint64 text_component = readLong(entity + BASE::MODEntity::TextComponent);
    if(!text_component){
        return text;
    }

    quint64 text_p = readLong(text_component + BASE::TextComponent::_text);
    if(!text_p){
        return text;
    }

    return readString(text_p + BASE::String::text, 100);
}

void GameStruct::printEntityChild(quint64 entity, int depth)
{
    QString indent = QString(" ").repeated(2 * depth);

    // 子组件列表地址
    uintptr_t listAddress = readLong(entity + BASE::MODEntity::children);
    if (!listAddress) {
        qDebug().noquote() << indent << "List: NULL";
        return;
    }

    qDebug().noquote() << QString("%1[Entity 0x%2]")
                              .arg(indent)
                              .arg(entity, 0, 16);

    uintptr_t myArrayAddress = readLong(listAddress + BASE::List::InternalArray);
    if (!myArrayAddress) {
        qDebug().noquote() << indent << "  myArray: NULL";
        return;
    }

    int32_t count = readInt(listAddress + BASE::List::size);
    if (count <= 0 || count > 1000) {
        qDebug().noquote() << indent << "  count invalid";
        return;
    }

    // 数据区起始地址
    uintptr_t arrayStartAddress = myArrayAddress + BASE::InternalArray::Head;

    qDebug().noquote() << QString("%1  子组件数量: %2")
                              .arg(indent)
                              .arg(count);

    for (int i = 0; i < count; ++i) {
        // 子组件指针
        uintptr_t compPtr = readLong(arrayStartAddress + i * 8);

        if (compPtr && compPtr != 0) {
            // 读取名称
            QString name = readEntityName(compPtr);

            qDebug().noquote() << QString("%1  ├─[%2] 0x%3  %4")
                                      .arg(indent)
                                      .arg(i)
                                      .arg(compPtr, 0, 16)
                                      .arg(name);

            // 递归遍历下一层
            printEntityChild(compPtr, depth + 1);
        } else {
            qDebug().noquote() << QString("%1  ├─[%2] NULL")
                                      .arg(indent)
                                      .arg(i);
        }
    }
}

#include "gamestruct.h"
#include "utils/helper.h"

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

            QString name_str = QString("%1  ├─[%2] 0x%3  %4")
                                   .arg(indent)
                                   .arg(i)
                                   .arg(compPtr, 0, 16)
                                   .arg(name);
            // qDebug().noquote() << QString("%1  ├─[%2] 0x%3  %4")
            //                           .arg(indent)
            //                           .arg(i)
            //                           .arg(compPtr, 0, 16)
            //                           .arg(name);
            Helper::console(name_str);
            qDebug() << name_str;
            // 递归遍历下一层
            printEntityChild(compPtr, depth + 1);
        } else {
            // qDebug().noquote() << QString("%1  ├─[%2] NULL")
            //                           .arg(indent)
            //                           .arg(i);
            QString name_str =   QString("%1  ├─[%2] NULL")
                                   .arg(indent)
                                   .arg(i);
            Helper::console(name_str);
            qDebug() << name_str;
        }
    }
}

// 构建 JSON 对象的递归函数
QJsonObject GameStruct::buildJSONEntityChild(quint64 entity, std::unordered_set<quint64> &entity_set, int depth)
{
    QJsonObject result;

    bool visited = entity_set.find(entity) != entity_set.end();
    if(visited) return result;

    // 标记当前实体已访问
    entity_set.insert(entity);

    QString name = readEntityName(entity);

    // 实体地址
    result["entity"] = QString("0x%1").arg(entity, 0, 16);
    result["depth"] = depth;
    result["name"] = name;
    result["children_count"] = 0;
    result["childrens"] = QJsonValue::Null;

    // 子组件列表地址
    uintptr_t listAddress = readLong(entity + BASE::MODEntity::children);
    if (!listAddress) {
        result["childrens"] = QJsonValue::Null;
        return result;
    }

    uintptr_t myArrayAddress = readLong(listAddress + BASE::List::InternalArray);
    if (!myArrayAddress) {
        result["childrens"] = QJsonValue::Null;
        return result;
    }

    int32_t count = readInt(listAddress + BASE::List::size);
    if (count <= 0 || count > 10000) {
        result["childrens"] = QJsonValue::Null;
        return result;
    }

    result["children_count"] = count;

    if(count <= 0){
        return result;
    }

    uintptr_t arrayStartAddress = myArrayAddress + BASE::InternalArray::Head;

    // 构建子节点数组
    QJsonArray childrenArray;

    for (int i = 0; i < count; ++i) {
        uintptr_t compPtr = readLong(arrayStartAddress + i * 8);

        if (compPtr && compPtr != 0) {
            QString name = readEntityName(compPtr);
            QString ptrStr = QString("0x%1").arg(compPtr, 0, 16);

            // 检查是否已访问（循环引用检测）
            // bool visited = entity_set.find(compPtr) != entity_set.end();

            QJsonObject child;
            child["depth"] = depth + 1;
            child["entity"] = ptrStr;
            child["name"] = name;
            child["children_count"] = 0;
            child["childrens"] = QJsonValue::Null;

            uintptr_t child_listAddress = readLong(compPtr + BASE::MODEntity::children);
            if (child_listAddress) {
                uintptr_t child_ArrayAddress = readLong(child_listAddress + BASE::List::InternalArray);
                if (child_ArrayAddress) {
                    int32_t child_count = readInt(child_listAddress + BASE::List::size);
                    if (child_count > 0 && child_count < 10000) {
                        child["children_count"] = child_count;
                        child["childrens"] = buildJSONEntityChild(compPtr, entity_set, depth + 1);
                    }
                }
            }
            childrenArray.append(child);
        } else {
            childrenArray.append(QJsonValue::Null);
        }
    }

    result["childrens"] = childrenArray;

    return result;
}

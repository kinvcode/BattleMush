#include "helper.h"
#include "driver/mem.h"

using namespace Mem;

void DebugOutput::out(const QString& message){
    QString prefix = "[myprint] ";
    QString formattedMsg = prefix + message;
    QString output = formattedMsg + "\n";

    // 输出到 DebugView
    OutputDebugStringW(output.toStdWString().c_str());
}



quint64 Helper::findChild(quint64 parent, QString name)
{
    quint64 child_entity = 0;

    quint64 list = Mem::readLong(parent + 0x3A8);
    int list_num = Mem::readLong(list + 0x18);
    if(list_num == 0){
        qDebug() << "不存在子模块";
        return child_entity;
    }

    quint64 list_head = Mem::readLong(list + 0x10);
    int max = Mem::readInt(list_head + 0x18);
    list_head = list_head + 0x20;
    for(int i = 0; i < max; i++){
        quint64 cur_entity = Mem::readLong(list_head);
        if(cur_entity == 0){
            break;
        }

        QString entity_name = readString(readLong(cur_entity + 0x270) + 0x14, 100);
        // qDebug() << "当前模块:" << entity_name;
        if(entity_name == name){
            return cur_entity;
        }

        list_head += 0x8;
    }

    return child_entity;
}

QString Helper::getEntityTextComponentString(quint64 entity)
{
    QString name = "";
    name = readString(readLong(readLong(entity + 0x1F8) + 0xF0) + 0x14, 100);
    return name;
}

bool Helper::parseHPString(const QString &Text, int &left, int &right) {
    QStringList parts = Text.split("/");
    if (parts.size() == 2) {
        left = parts[0].trimmed().toInt();
        right = parts[1].trimmed().toInt();
        return true;
    }
    return false;
}

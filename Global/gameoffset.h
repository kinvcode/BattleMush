#ifndef GAMEOFFSET_H
#define GAMEOFFSET_H

#define C_Entity_Name 0x270; // Entity名称
#define C_Entity_Child 0x3A8; // Entity子组件

namespace BASE {
    namespace List{
        static int size = 0x18;
        static int InternalArray = 0x10;

    };

    namespace InternalArray{
        static int max_size = 0x18;
        static int Head = 0x20;
    };

    namespace MODEntity{
        static int MapComponent = 0x70;
    };

    namespace MapComponent{
        static int FootholdComponent = 0x108;
    };

    namespace FootholdComponent{
        static int FootholdImpl = 0x120;
    };

    namespace FootholdImpl{
        static int footHoldList = 0x20;
    };

    namespace Foothold{
        static int ID = 0x10;
        static int StartPoint = 0x14;
        static int EndPoint = 0x1C;
        static int length = 0x40;
        static int groupID = 0x4C;
    };
};

#endif // GAMEOFFSET_H

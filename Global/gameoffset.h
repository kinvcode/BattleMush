#ifndef GAMEOFFSET_H
#define GAMEOFFSET_H

#define C_Entity_Name 0x270; // Entity名称
#define C_Entity_Child 0x3A8; // Entity子组件

namespace BASE {

    static int ClientCoreThreadManager_base = 0xA230ED8;
    static int ClientCoreThreadManager_offset_lv1 = 0xB8;
    static int ClientCoreThreadManager_offset_lv2 = 0x0;
    static int ClientCoreThreadManager_GameDictionary = 0x10;

    namespace Dictionary{
        static int InternalArray_entries = 0x18;
        static int count = 0x20;
    };

    namespace DynamicTree{
        static int _nodes = 0x10;
        static int _nodeCount = 0x20;
    };

    namespace List{
        static int size = 0x18;
        static int InternalArray = 0x10;
    };

    namespace InternalArray{
        static int max_size = 0x18;
        static int Head = 0x20;
    };

    namespace String{
        static int size = 0x10;
        static int text = 0x14;
    };

    namespace CollisionService{
        static int simulatorByMap = 0x38;
    };

    namespace CollisionSimulator{
        static int World = 0x30;
    };

    namespace World{
        static int ContactManager = 0x18;
        static int MultipleDynamicTreeBroadPhase = 0x18;
    };

    namespace UserService{
        static int localPlayer = 0x90;
    };

    namespace MultipleDynamicTreeBroadPhase{
        static int trees = 0x10;
        static int numbersByProxyId = 0x20;
    };

    namespace GameContext{
        static int WorldEntity = 0x140;
        static int MapsEntity = 0x148;
        static int UIEntity = 0x150;
        static int allServices = 0x298;
    };

    namespace MODEntity{
        static int MapComponent = 0x70;
        static int TileMapComponent = 0x88;
        static int ClimbableComponent = 0xC8;
        static int StateComponent = 0xF0;
        static int ButtonComponent = 0x140;
        static int TransformComponent = 0x158;
        static int PlayerControllerComponent = 0x170;
        static int PlayerComponent = 0x198;
        static int RigidbodyComponent = 0x1D8;
        static int TextComponent = 0x1F8;
        static int FootholdComponent = 0x1E0;
        static int MODModel = 0x268;
        static int name = 0x270;
        static int _entityId = 0x2A0;
        static int children = 0x3A8;
    };

    namespace MapComponent{
        static int FootholdComponent = 0x108;
        static int MoveBoundingMin = 0x120;
        static int MoveBoundingMax = 0x128;
    };

    namespace FootholdComponent{
        static int FootholdImpl = 0x120;
    };

    namespace TileMapComponent{
        static int TileContextList = 0x120;
    };

    namespace TileContext{
        static int convexPoints = 0x20;
    };

    namespace TextComponent{
        static int _text = 0xF0;
    };

    namespace TransformComponent{
        static int PositionX = 0x190;
        static int PositionY = 0x194;
    };

    namespace RigidbodyComponent{
        static int exists = 0xA5;
        static int currentFoothold = 0xF8;
        static int attachEntity = 0x110;
        static int airPos = 0x120;
        static int _Position = 0x1A4;
    };

    namespace StateComponent{
        static int attacking = 0xEC;
        static int _CurrentStateName = 0x138;
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

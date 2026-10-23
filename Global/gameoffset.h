#ifndef GAMEOFFSET_H
#define GAMEOFFSET_H

namespace BASE {
    static int game_width = 1296;
    static int game_height = 759;
    static int ClientCoreThreadManager_base = 0xAC616E0;//
    static int ClientCoreThreadManager_offset_lv1 = 0xB8;
    static int ClientCoreThreadManager_offset_lv2 = 0x0;
    static int ClientCoreThreadManager_GameDictionary = 0x10;

    static int Camera_base = 0xAD1CEC8; //
    static int Camera_offset_lv1 = 0xB8;
    static int Camera_offset_lv2 = 0x0;

    static int TMP_UpdateManager_base = 0xAC8EAD0;//
    static int TMP_UpdateManager_offset_lv1 = 0xB8;
    static int TMP_UpdateManager_offset_lv2 = 0x0;
    static int TMP_UpdateManager_List = 0x38;

    static int UIPopupManager_base = 0xAC8CA08; //
    static int UIPopupManager_offset_lv1 = 0xB8;
    static int UIPopupManager_offset_lv2 = 0x0;

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
        static int GameLogicEntity = 0x158;
        static int ModelEntity = 0x170;
        static int EntityCountsFromEvent = 0x1A0;
        static int allServices = 0x2A0;
    };

    namespace MODEntity{
        static int MapComponent = 0x80;
        static int TileMapComponent = 0x98;
        static int MovementComponent = 0xD0;
        static int ClimbableComponent = 0xD8;
        static int StateComponent = 0x100;
        static int SpriteGUIRendererComponent = 0x120;
        static int ButtonComponent = 0x150;
        static int UITransformComponent = 0x160;
        static int TransformComponent = 0x168;
        static int PlayerControllerComponent = 0x180;
        static int SpriteRendererComponent = 0x190;
        static int ScrollLayoutGroupComponent = 0x198;
        static int PlayerComponent = 0x1A8;
        static int RigidbodyComponent = 0x1F0;
        static int FootholdComponent = 0x1F8;
        static int TextComponent = 0x210;
        static int MODModel = 0x280;
        static int name = 0x288;
        static int _entityId = 0x2B8;
        static int children = 0x3C8;
    };

    namespace MapComponent{
        static int FootholdComponent = 0x110;
        static int MoveBoundingMin = 0x128;
        static int MoveBoundingMax = 0x130;
    };

    namespace FootholdComponent{
        static int FootholdImpl = 0x128;
    };

    namespace TileMapComponent{
        static int TileContextList = 0x128;
    };

    namespace TileContext{
        static int convexPoints = 0x20;
    };

    namespace TextComponent{
        static int _text = 0xF8;
    };

    namespace TransformComponent{
        static int PositionX = 0x198; // worldPositionForUnity
        static int PositionY = 0x19C;
    };

    namespace RigidbodyComponent{
        static int exists = 0xA5; // MODComponent字段，RegisteredUpdate
        static int currentFoothold = 0x100;
        static int attachEntity = 0x118;
        static int airPos = 0x128;
        static int _Position = 0x184; // RealMoveVelocity
    };

    namespace MovementComponent{
        static int faceLeft = 0x160;
    };

    namespace StateComponent{
        static int attacking = 0xF4; // 不确定，继承MODComponent字段 _Enable，其中RegisteredUpdate字段作用和他一样
        static int _CurrentStateName = 0x128; //lastEnteredStateName
    };

    namespace SpriteRendererComponent{
        static int _SpriteRUID = 0x160;
    };

    namespace SpriteGUIRendererComponent{
        static int MODDataRef = 0x170; // _iconDataRef
    };

    namespace ScrollLayoutGroupComponent{
        static int Position = 0x178; // 未更新， _scrollNormalizedPosition
    };

    namespace MODDataRef{
        static int dataId = 0x10;
    };

    namespace FootholdImpl{
        static int footHoldList = 0x20;
    };

    namespace Foothold{
        static int ID = 0x10;
        static int StartPoint = 0x14;
        static int EndPoint = 0x1C;
        static int length = 0x40; // Length
        static int groupID = 0x4C; // groupID
    };

    namespace MODPlayerMain{
        static int CinemachineBrain = 0x20; //playerBrainCamera
    };

    namespace CinemachineBrain{
        static int Camera = 0x50; // m_OutputCamera
    };

    namespace TextMeshPro{
        static int m_text = 0xE0; // 继承TMP_Text字段m_text
    };

    namespace UIPopupManager{
        static int OnChangeScreenSize = 0x60;
    };

    // UIAntiMacro已被加密，且改变了类名称
    // public class UIAM : UIPopupDialog
    namespace UIAntiMacro{
        static int DialogID = 0x38;
        static int posList = 0xC0;
        static int currentStatus = 0xD0;
        static int currentTick = 0xD8;
        static int readyTick = 0xE0;
    };

    namespace UITransformComponent{
        static int position = 0x1F0; //_anchoredPosition
    };

    namespace MODModel{
        static int Id = 0x30; //String Id
    };
};

#endif // GAMEOFFSET_H

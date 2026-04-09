#ifndef DEVICE_H
#define DEVICE_H

#include "utils.h"

// 设备状态枚举
typedef enum {
    DEVICE_NORMAL = 0,     // 正常
    DEVICE_REPAIRING,      // 维修中
    DEVICE_SCRAPPED         // 已报废
} DeviceStatus;

// 设备结点结构体
typedef struct DeviceNode {
    char id[MAX_ID_LEN];            // 设备编号
    char name[MAX_NAME_LEN];        // 设备名称
    char category[MAX_DEPT_LEN];    // 设备类别
    char department[MAX_DEPT_LEN];  // 所属科室
    char purchaseDate[MAX_DATE_LEN];// 购入日期
    double price;                   // 购入价格
    int status;                     // 当前状态
    int needPurchase;               // 是否需要购入
    int purchaseQuantity;           // 需要购入数量
    char handler[MAX_NAME_LEN];     // 责任人
    char location[MAX_REMARK_LEN];  // 存放位置
    char warrantyPeriod[20];        // 保修期
    char remark[MAX_REMARK_LEN];    // 备注
    struct DeviceNode* next;        // 下一个结点
} DeviceNode;

// 维修记录结点结构体
typedef struct FixNode {
    char id[MAX_ID_LEN];            // 维修编号
    char deviceId[MAX_ID_LEN];      // 设备编号
    char description[MAX_REMARK_LEN];// 故障描述
    char sendDate[MAX_DATE_LEN];    // 送修日期
    char finishDate[MAX_DATE_LEN];  // 完成日期
    double cost;                    // 维修费用
    char result[MAX_REMARK_LEN];    // 维修结果
    char handler[MAX_NAME_LEN];     // 经办人
    char remark[MAX_REMARK_LEN];    // 备注
    struct FixNode* next;           // 下一个结点
} FixNode;

// 设备链表结构体
typedef struct {
    DeviceNode* head;
    int count;
} DeviceList;

// 维修记录链表结构体
typedef struct {
    FixNode* head;
    int count;
} FixList;

// 初始化设备链表
void initDeviceList(DeviceList* list);

// 初始化维修记录链表
void initFixList(FixList* list);

// 从文件加载数据
void loadDevicesFromFile(DeviceList* list, const char* filename);
void loadFixesFromFile(FixList* list, const char* filename);

// 保存数据到文件
void saveDevicesToFile(DeviceList* list, const char* filename);
void saveFixesToFile(FixList* list, const char* filename);

// 创建新设备结点
DeviceNode* createDeviceNode();

// 创建新维修记录结点
FixNode* createFixNode();

// 检查设备编号是否重复
int isDeviceIDExist(DeviceList* list, const char* id);

// 插入设备结点
void insertDeviceNode(DeviceList* list, DeviceNode* node);

// 按编号查找设备
DeviceNode* findDeviceByID(DeviceList* list, const char* id);

// 删除设备
int deleteDeviceByID(DeviceList* list, const char* id);

// 修改设备信息
void modifyDeviceInfo(DeviceNode* node);

// 显示单个设备信息
void printDeviceOne(DeviceNode* node);

// 显示所有设备
void printDeviceAll(DeviceList* list);

// 设备报废
void scrapDevice(DeviceList* list, const char* id);

// 统计设备
void statDevices(DeviceList* list, FixList* fixList);

// 释放链表内存
void freeDeviceList(DeviceList* list);
void freeFixList(FixList* list);

// 设备管理菜单
void deviceMenu(DeviceList* dList, FixList* fList);

#endif

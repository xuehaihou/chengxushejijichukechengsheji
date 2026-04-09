#ifndef LOGISTICS_H
#define LOGISTICS_H

#include "utils.h"

// 物资结点结构体
typedef struct MaterialNode {
    char id[MAX_ID_LEN];            // 物资编号
    char name[MAX_NAME_LEN];        // 物资名称
    char category[MAX_DEPT_LEN];    // 类别
    int quantity;                   // 库存数量
    char unit[20];                  // 计量单位
    int minStock;                   // 最低库存值
    char location[MAX_REMARK_LEN];  // 存放位置
    char supplier[MAX_NAME_LEN];    // 供应商
    char lastInDate[MAX_DATE_LEN];  // 最近入库日期
    char lastOutDate[MAX_DATE_LEN]; // 最近出库日期
    char remark[MAX_REMARK_LEN];    // 备注
    struct MaterialNode* next;      // 下一个结点
} MaterialNode;

// 领用记录结点结构体
typedef struct UsageNode {
    char id[MAX_ID_LEN];            // 申领编号
    char materialId[MAX_ID_LEN];    // 物资编号
    char dept[MAX_DEPT_LEN];        // 领用部门
    int quantity;                   // 数量
    char date[MAX_DATE_LEN];        // 日期
    char handler[MAX_NAME_LEN];     // 经手人
    int auditStatus;                // 审核状态
    char remark[MAX_REMARK_LEN];    // 备注
    struct UsageNode* next;         // 下一个结点
} UsageNode;

// 物资链表结构体
typedef struct {
    MaterialNode* head;
    int count;
} MaterialList;

// 领用记录链表结构体
typedef struct {
    UsageNode* head;
    int count;
} UsageList;

// 初始化物资链表
void initMaterialList(MaterialList* list);

// 初始化领用记录链表
void initUsageList(UsageList* list);

// 从文件加载物资数据
void loadMaterialsFromFile(MaterialList* list, const char* filename);

// 从文件加载领用记录
void loadUsageFromFile(UsageList* list, const char* filename);

// 保存物资数据到文件
void saveMaterialsToFile(MaterialList* list, const char* filename);

// 保存领用记录到文件
void saveUsageToFile(UsageList* list, const char* filename);

// 创建新物资结点
MaterialNode* createMaterialNode();

// 创建新领用记录结点
UsageNode* createUsageNode();

// 检查物资编号是否重复
int isMaterialIDExist(MaterialList* list, const char* id);

// 插入物资结点
void insertMaterialNode(MaterialList* list, MaterialNode* node);

// 插入领用记录结点
void insertUsageNode(UsageList* list, UsageNode* node);

// 按编号查找物资
MaterialNode* findMaterialByID(MaterialList* list, const char* id);

// 按名称查找物资
MaterialNode* findMaterialByName(MaterialList* list, const char* name);

// 删除物资
int deleteMaterialByID(MaterialList* list, const char* id);

// 修改物资信息
void modifyMaterialInfo(MaterialNode* node);

// 物资入库
void materialInStock(MaterialList* list, const char* id, int quantity);

// 物资出库
int materialOutStock(MaterialList* list, const char* id, int quantity);

// 显示单个物资信息
void printMaterialOne(MaterialNode* node);

// 显示所有物资
void printMaterialAll(MaterialList* list);

// 显示库存预警
void printStockWarning(MaterialList* list);

// 统计物资
void statMaterials(MaterialList* list, UsageList* usageList);

// 释放物资链表内存
void freeMaterialList(MaterialList* list);

// 释放领用记录链表内存
void freeUsageList(UsageList* list);

// 后勤管理菜单
void logisticsMenu(MaterialList* mList, UsageList* uList);

// 新增物资
void addMaterial(MaterialList* list);

// 查询物资
void queryMaterial(MaterialList* list);

// 删除物资
void deleteMaterial(MaterialList* list);

// 修改物资
void modifyMaterial(MaterialList* list);

// 入库操作
void materialIn(MaterialList* list);

// 出库操作
void materialOut(MaterialList* list, UsageList* uList);

// 领用记录查询
void queryUsage(UsageList* list);

#endif
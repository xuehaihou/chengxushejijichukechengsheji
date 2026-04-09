#ifndef MEDICINE_H
#define MEDICINE_H

#include "utils.h"

// 药品结点结构体
typedef struct MedicineNode {
    char id[MAX_ID_LEN];            // 药品编号
    char name[MAX_NAME_LEN];        // 药品名称
    char category[MAX_DEPT_LEN];    // 类别
    int quantity;                   // 库存数量
    char unit[20];                  // 计量单位
    double price;                    // 单价
    char manufacturer[MAX_NAME_LEN]; // 生产厂家
    char expiryDate[MAX_DATE_LEN];    // 有效期
    char remark[MAX_REMARK_LEN];    // 备注
    struct MedicineNode* next;        // 下一个结点
} MedicineNode;

// 药品链表结构体
typedef struct {
    MedicineNode* head;
    int count;
} MedicineList;

// 初始化药品链表
void initMedicineList(MedicineList* list);

// 从文件加载药品数据
void loadMedicinesFromFile(MedicineList* list, const char* filename);

// 保存药品数据到文件
void saveMedicinesToFile(MedicineList* list, const char* filename);

// 创建新药品结点
MedicineNode* createMedicineNode();

// 检查药品编号是否重复
int isMedicineIDExist(MedicineList* list, const char* id);

// 插入药品结点
void insertMedicineNode(MedicineList* list, MedicineNode* node);

// 按编号查找药品
MedicineNode* findMedicineByID(MedicineList* list, const char* id);

// 删除药品
int deleteMedicineByID(MedicineList* list, const char* id);

// 修改药品信息
void modifyMedicineInfo(MedicineNode* node);

// 药品入库
void medicineInStock(MedicineList* list, const char* id, int quantity);

// 药品出库
int medicineOutStock(MedicineList* list, const char* id, int quantity);

// 显示单个药品信息
void printMedicineOne(MedicineNode* node);

// 显示所有药品
void printMedicineAll(MedicineList* list);

// 查看库存预警
void printMedicineWarning(MedicineList* list);

// 释放药品链表内存
void freeMedicineList(MedicineList* list);

// 药库管理菜单
void medicineMenu(MedicineList* list);

// 新增药品
void addMedicine(MedicineList* list);

// 查询药品
void queryMedicine(MedicineList* list);

// 删除药品
void deleteMedicine(MedicineList* list);

// 修改药品
void modifyMedicine(MedicineList* list);

// 入库操作
void medicineIn(MedicineList* list);

// 出库操作
void medicineOut(MedicineList* list);

#endif

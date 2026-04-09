#ifndef STAFF_H
#define STAFF_H

#include "utils.h"

// 工作状态枚举
typedef enum {
    STATUS_WORKING = 0,     // 在职
    STATUS_LEAVE,           // 请假
    STATUS_RESIGNED,        // 离职
    STATUS_TRANSFERRED      // 调岗
} WorkStatus;

// 职工结点结构体
typedef struct StaffNode {
    char id[MAX_ID_LEN];            // 工号
    char name[MAX_NAME_LEN];        // 姓名
    char gender[10];                // 性别
    int age;                        // 年龄
    char phone[MAX_PHONE_LEN];      // 电话
    char department[MAX_DEPT_LEN];  // 所属部门
    char position[MAX_POSITION_LEN];// 岗位
    char hireDate[MAX_DATE_LEN];    // 入职日期
    double salary;                  // 基本工资
    int status;                     // 工作状态
    char remark[MAX_REMARK_LEN];    // 备注
    struct StaffNode* next;         // 下一个结点
} StaffNode;

// 职工链表结构体
typedef struct {
    StaffNode* head;
    int count;
} StaffList;

// 初始化职工链表
void initStaffList(StaffList* list);

// 从文件加载职工数据
void loadStaffFromFile(StaffList* list, const char* filename);

// 保存职工数据到文件
void saveStaffToFile(StaffList* list, const char* filename);

// 创建新职工结点
StaffNode* createStaffNode();

// 检查工号是否重复
int isStaffIDExist(StaffList* list, const char* id);

// 插入职工结点
void insertStaffNode(StaffList* list, StaffNode* node);

// 按工号查找职工
StaffNode* findStaffByID(StaffList* list, const char* id);

// 按姓名查找职工
StaffNode* findStaffByName(StaffList* list, const char* name);

// 删除职工结点
int deleteStaffByID(StaffList* list, const char* id);

// 修改职工信息
void modifyStaffInfo(StaffNode* node);

// 显示单个职工信息
void printStaffOne(StaffNode* node);

// 显示所有职工信息
void printStaffAll(StaffList* list);

// 按工号排序
void sortStaffByID(StaffList* list);

// 按工资排序
void sortStaffBySalary(StaffList* list);

// 按年龄排序
void sortStaffByAge(StaffList* list);

// 按入职日期排序
void sortStaffByHireDate(StaffList* list);

// 统计各部门人数
void countStaffByDept(StaffList* list);

// 统计各岗位人数
void countStaffByPosition(StaffList* list);

// 统计在职/离职人数
void countStaffByStatus(StaffList* list);

// 计算平均工资
void calculateAvgSalary(StaffList* list);

// 释放职工链表内存
void freeStaffList(StaffList* list);

// 职工管理菜单
void staffMenu(StaffList* list);

// 新增职工
void addStaff(StaffList* list);

// 查询职工
void queryStaff(StaffList* list);

// 删除职工
void deleteStaff(StaffList* list);

// 修改职工
void modifyStaff(StaffList* list);

// 排序显示职工
void sortStaff(StaffList* list);

// 统计职工
void statStaff(StaffList* list);

#endif
#ifndef FINANCE_H
#define FINANCE_H

#include "utils.h"

// 收支类型枚举
typedef enum {
    TYPE_INCOME = 0,    // 收入
    TYPE_EXPENSE        // 支出
} FinanceType;

// 审核状态枚举
typedef enum {
    AUDIT_PENDING = 0,  // 未审核
    AUDIT_APPROVED,     // 已审核
    AUDIT_REJECTED      // 已作废
} AuditStatus;

// 财务记录结点结构体
typedef struct FinanceNode {
    char id[MAX_ID_LEN];            // 记录编号
    int type;                       // 收支类型
    double amount;                  // 金额
    char date[MAX_DATE_LEN];        // 日期
    char handler[MAX_NAME_LEN];     // 经办人
    char department[MAX_DEPT_LEN];  // 所属部门
    char purpose[MAX_REMARK_LEN];   // 用途或来源
    int auditStatus;                // 审核状态
    char remark[MAX_REMARK_LEN];    // 备注
    struct FinanceNode* next;       // 下一个结点
} FinanceNode;

// 财务链表结构体
typedef struct {
    FinanceNode* head;
    int count;
} FinanceList;

// 初始化财务链表
void initFinanceList(FinanceList* list);

// 从文件加载财务数据
void loadFinanceFromFile(FinanceList* list, const char* filename);

// 保存财务数据到文件
void saveFinanceToFile(FinanceList* list, const char* filename);

// 创建新财务记录结点
FinanceNode* createFinanceNode();

// 检查记录编号是否重复
int isFinanceIDExist(FinanceList* list, const char* id);

// 插入财务记录结点
void insertFinanceNode(FinanceList* list, FinanceNode* node);

// 按编号查找财务记录
FinanceNode* findFinanceByID(FinanceList* list, const char* id);

// 删除财务记录
int deleteFinanceByID(FinanceList* list, const char* id);

// 修改财务记录
void modifyFinanceInfo(FinanceNode* node);

// 显示单个财务记录
void printFinanceOne(FinanceNode* node);

// 显示所有财务记录
void printFinanceAll(FinanceList* list);

// 按日期范围查询
void queryFinanceByDate(FinanceList* list, const char* startDate, const char* endDate);

// 按部门查询
void queryFinanceByDept(FinanceList* list, const char* dept);

// 按经办人查询
void queryFinanceByHandler(FinanceList* list, const char* handler);

// 按类型查询
void queryFinanceByType(FinanceList* list, int type);

// 计算总收入
void calculateTotalIncome(FinanceList* list);

// 计算总支出
void calculateTotalExpense(FinanceList* list);

// 计算结余
void calculateBalance(FinanceList* list);

// 按月统计
void statFinanceByMonth(FinanceList* list, const char* yearMonth);

// 释放财务链表内存
void freeFinanceList(FinanceList* list);

// 财务管理菜单
void financeMenu(FinanceList* list);

// 新增财务记录
void addFinance(FinanceList* list);

// 查询财务记录
void queryFinance(FinanceList* list);

// 删除财务记录
void deleteFinance(FinanceList* list);

// 修改财务记录
void modifyFinance(FinanceList* list);

// 审核财务记录
void auditFinance(FinanceList* list);

// 财务报表
void financeReport(FinanceList* list);

#endif
/**
 * @file log.h
 * @brief 日志系统头文件 - 定义日志数据结构、常量和函数声明
 *
 * 本模块实现医院管理系统的日志记录功能，用于跟踪和记录系统中
 * 各类操作（如药品添加、挂号、预约等），支持管理员权限验证。
 */

#ifndef LOG_H
#define LOG_H

#include "utils.h"
#include <conio.h>

/* ==================== 常量定义 ==================== */

/** 日志详情最大长度 */
#define MAX_LOG_DETAIL 500

/** 管理员密码 - 用于日志查询权限验证 */
#define ADMIN_PASSWORD "admin123"

/* ==================== 日志类型枚举 ==================== */

/** 药品管理日志类型 - 记录药品的增删改、入库出库操作 */
#define LOG_MEDICINE    1

/** 门诊/挂号日志类型 - 记录门诊挂号信息的增删改操作 */
#define LOG_OUTPATIENT  2

/** 预约管理日志类型 - 记录预约的增删改、取消操作 */
#define LOG_APPOINT     3

/** 住院管理日志类型 - 记录入院、出院、信息修改操作 */
#define LOG_INPATIENT   4

/** 急诊管理日志类型 - 记录急诊记录的增删改操作 */
#define LOG_EMERGENCY   5

/** 血库管理日志类型 - 记录血液库存的增删改、入库出库操作 */
#define LOG_BLOOD       6

/** 设备管理日志类型 - 记录医疗设备的增删改操作 */
#define LOG_DEVICE      7

/** 财务管理日志类型 - 记录财务收支的增删改操作 */
#define LOG_FINANCE     8

/** 人事管理日志类型 - 记录员工信息的增删改操作 */
#define LOG_STAFF       9

/** 后勤管理日志类型 - 记录物资的增删改、入库出库操作 */
#define LOG_LOGISTIC    10

/* ==================== 数据结构定义 ==================== */

/**
 * @struct LogNode
 * @brief 日志节点结构体 - 存储单条日志记录
 *
 * 使用链表结构存储，每条日志包含时间、操作类型、操作人和详细信息
 */
typedef struct LogNode {
    char datetime[20];              /**< 操作时间，格式: YYYY-MM-DD HH:MM:SS */
    char operation[50];             /**< 操作描述，如"新增药品"、"删除挂号"等 */
    char operator[50];              /**< 操作人，当前为"系统"（可扩展为实际用户） */
    char detail[MAX_LOG_DETAIL];    /**< 操作详情，包含具体的数据信息 */
    struct LogNode* next;           /**< 指向下一个日志节点的指针 */
} LogNode;

/**
 * @struct LogList
 * @brief 日志链表结构体 - 管理所有日志节点
 *
 * 包含头指针和计数器，用于维护日志链表
 */
typedef struct {
    LogNode* head;      /**< 链表头指针，指向第一条日志记录 */
    int count;          /**< 日志记录总数 */
} LogList;

/* ==================== 初始化与释放函数 ==================== */

/**
 * @brief 初始化日志链表
 * @param list 指向LogList结构的指针
 *
 * 将链表头指针置空，计数器清零
 * 输入格式: 无需输入参数
 */
void initLogList(LogList* list);

/**
 * @brief 释放日志链表内存
 * @param list 指向LogList结构的指针
 *
 * 遍历链表释放所有节点的内存，防止内存泄漏
 * 输入格式: 无需输入参数
 */
void freeLogList(LogList* list);

/* ==================== 核心功能函数 ==================== */

/**
 * @brief 写入一条日志记录到文件
 * @param logType 日志类型，使用LOG_XXX常量
 * @param operation 操作描述字符串，如"新增药品"、"删除挂号"
 * @param operator 操作人名称，通常传入"系统"
 * @param detail 详细信息字符串，包含被操作对象的具体信息
 *
 * 自动追加写入对应类型的日志文件
 * 输入格式:
 *   - logType: 整数(1-10)，表示日志所属模块
 *   - operation: 字符串，描述执行的操作类型
 *   - operator: 字符串，操作者标识
 *   - detail: 字符串，包含操作的详细内容
 */
void writeLog(int logType, const char* operation, const char* operator, const char* detail);

/**
 * @brief 从文件加载日志记录到内存
 * @param list 指向LogList结构的指针，用于存储加载的日志
 * @param filename 日志文件路径
 *
 * 文件格式: datetime|operation|operator|detail\n
 * 输入格式:
 *   - list: 已初始化的LogList指针
 *   - filename: 字符串，日志文件的完整路径
 */
void loadLogFromFile(LogList* list, const char* filename);

/**
 * @brief 将内存中的日志保存到文件
 * @param list 指向LogList结构的指针，包含要保存的日志
 * @param filename 目标文件路径
 *
 * 覆盖式写入，文件格式: datetime|operation|operator|detail\n
 * 输入格式:
 *   - list: 包含日志数据的LogList指针
 *   - filename: 字符串，目标文件路径
 */
void saveLogToFile(LogList* list, const char* filename);

/* ==================== 显示函数 ==================== */

/**
 * @brief 打印单条日志记录
 * @param node 指向要打印的LogNode节点
 *
 * 格式化输出日志的所有字段信息
 * 输入格式:
 *   - node: 有效的LogNode指针
 */
void printLogOne(LogNode* node);

/**
 * @brief 打印所有日志记录
 * @param list 指向LogList结构的指针
 *
 * 遍历链表并打印每条日志，显示总数统计
 * 输入格式:
 *   - list: 包含日志数据的LogList指针
 */
void printLogAll(LogList* list);

/* ==================== 权限与菜单函数 ==================== */

/**
 * @brief 管理员身份验证
 * @return int 验证成功返回1，失败返回0
 *
 * 提示输入密码并进行比对，密码输入时字符不回显
 * 输入格式: 从键盘输入密码字符串（隐藏显示）
 */
int isAdminAuthenticated();

/**
 * @brief 日志管理菜单界面
 *
 * 提供日志查询功能的交互菜单，包括：
 * - 按类型查看日志
 * - 按操作类型筛选
 * - 按时间范围查询
 * - 查看全部日志
 * 需要先通过管理员身份验证才能访问
 * 输入格式: 通过菜单选择数字选项
 */
void logMenu();

/* ==================== 辅助函数 ==================== */

/**
 * @brief 根据日志类型获取对应的文件名
 * @param logType 日志类型常量(LOG_XXX)
 * @return const char* 返回日志文件的完整路径字符串
 *
 * 映射关系:
 *   LOG_MEDICINE -> "data/log_medicine.txt"
 *   LOG_OUTPATIENT -> "data/log_outpatient.txt"
 *   以此类推...
 * 输入格式: 整数(1-10)
 */
const char* getLogFileName(int logType);

/**
 * @brief 根据日志类型获取中文名称
 * @param logType 日志类型常量(LOG_XXX)
 * @return const char* 返回日志类型的中文字符串
 *
 * 映射关系:
 *   LOG_MEDICINE -> "药品管理"
 *   LOG_OUTPATIENT -> "门诊管理"
 *   以此类推...
 * 输入格式: 整数(1-10)
 */
const char* getLogTypeName(int logType);

#endif

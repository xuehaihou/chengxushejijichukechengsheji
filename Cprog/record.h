/**
 * @file record.h
 * @brief 病案管理模块头文件 - 定义病案数据结构和函数接口
 *
 * 本模块实现医院病案（病历档案）的完整管理功能。
 * 病案是患者在医院就诊过程中的完整医疗记录文档，
 * 是医疗质量控制和科研教学的重要数据基础。
 *
 * 核心功能:
 *   - 病案信息的增删改查(CRUD)
 *   - 病案数据的持久化存储（文件读写）
 *   - 病案归档状态管理
 *   - 病案信息展示和查询
 *
 * 数据结构:
 *   采用单链表结构存储病案记录，
 *   支持动态增长和灵活的插入删除操作。
 *
 * 病案内容包含:
 *   1. 标识信息: 病案编号、患者编号
 *   2. 基本信息: 姓名、性别、年龄
 *   3. 就诊信息: 科室、医生、日期
 *   4. 诊断信息: 入院诊断、出院诊断
 *   5. 治疗信息: 治疗摘要
 *   6. 管理信息: 归档状态、备注
 *
 * 应用场景:
 *   - 医生查看患者完整病史
 *   - 医疗质量评估和分析
 *   - 科研病例收集和管理
 *   - 医疗纠纷的证据保存
 *   - 统计报表的数据来源
 *
 * 使用方式:
 *   #include "record.h"
 *   RecordList list;
 *   initRecordList(&list);
 *   loadRecordsFromFile(&list, "data/records.txt");
 *   // ... 操作病案数据 ...
 *   saveRecordsToFile(&list, "data/records.txt");
 *   freeRecordList(&list);
 */

#ifndef RECORD_H
#define RECORD_H

#include "utils.h"

/**
 * @struct RecordNode
 * @brief 病案节点结构体 - 存储单个病案的完整信息
 *
 * 说明:
 *   每个节点代表一份完整的病案记录，
 *   包含从入院到出院的全过程诊疗信息。
 *   通过next指针形成单向链表。
 */
typedef struct RecordNode {
    char id[MAX_ID_LEN];                  /**< 病案编号 - 唯一标识符，主键 */
    char patientId[MAX_ID_LEN];           /**< 患者编号 - 关联的患者ID */
    char name[MAX_NAME_LEN];              /**< 姓名 - 患者真实姓名 */
    char gender[10];                      /**< 性别 - 男或女 */
    int age;                              /**< 年龄 - 患者年龄(正整数) */
    char department[MAX_DEPT_LEN];        /**< 科室 - 就诊科室名称 */
    char doctor[MAX_NAME_LEN];            /**< 医生 - 主治医生姓名 */
    char admitDiagnosis[MAX_REMARK_LEN];  /**< 入院诊断 - 初步诊断结果 */
    char dischargeDiagnosis[MAX_REMARK_LEN]; /**< 出院诊断 - 最终确诊结果 */
    char treatmentSummary[500];           /**< 治疗摘要 - 治疗过程概述(最长500字符) */
    char dischargeDate[MAX_DATE_LEN];     /**< 出院日期 - 格式YYYY-MM-DD */
    int archiveStatus;                    /**< 归档状态 - 0:未归档, 1:已归档 */
    char remark[MAX_REMARK_LEN];          /**< 备注 - 补充说明信息 */
    struct RecordNode* next;              /**< 后继指针 - 指向下一个病案节点 */
} RecordNode;

/**
 * @struct RecordList
 * @brief 病案链表结构体 - 管理所有病案的容器
 *
 * 说明:
 *   作为病案链表的头结点，维护链表的入口和统计信息。
 *   所有病案操作都通过此结构进行。
 */
typedef struct {
    RecordNode* head;  /**< 头指针 - 指向第一个病案节点，NULL表示空表 */
    int count;         /**< 计数器 - 当前病案总数，用于快速获取数量 */
} RecordList;

/* ==================== 初始化和加载函数 ==================== */

/**
 * @brief 初始化病案链表
 * @param list 病案链表指针
 *
 * 功能:
 *   将链表初始化为空状态，设置head=NULL，count=0。
 *   在使用其他操作前必须先调用此函数。
 */
void initRecordList(RecordList* list);

/**
 * @brief 从文件加载病案数据
 * @param list 病案链表指针（用于存储加载的数据）
 * @param filename 数据文件路径
 *
 * 功能:
 *   从文本文件读取病案记录并构建链表。
 *   文件格式：每行一条记录，字段以"|"分隔。
 *
 * 字段顺序:
 *   id|patientId|name|gender|age|department|doctor|
 *   admitDiagnosis|dischargeDiagnosis|treatmentSummary|
 *   dischargeDate|archiveStatus|remark
 */
void loadRecordsFromFile(RecordList* list, const char* filename);

/**
 * @brief 将病案数据保存到文件
 * @param list 病案链表指针（要保存的数据源）
 * @param filename 目标文件路径
 *
 * 功能:
 *   将内存中的病案链表写入文本文件。
 *   采用覆盖写模式，会清空原文件内容。
 */
void saveRecordsToFile(RecordList* list, const char* filename);

/* ==================== 增删改查函数 ==================== */

/**
 * @brief 创建新的病案节点
 * @return 成功返回病案节点指针，失败返回NULL
 *
 * 功能:
 *   通过控制台交互收集病案信息，创建完整的病案节点。
 *   包含输入验证和必填项检查。
 */
RecordNode* createRecordNode();

/**
 * @brief 检查病案编号是否已存在
 * @param list 病案链表指针
 * @param id 要检查的病案编号
 * @return 存在返回1，不存在返回0
 *
 * 功能:
 *   在链表中查找指定编号，用于新增前的唯一性验证。
 */
int isRecordIDExist(RecordList* list, const char* id);

/**
 * @brief 插入病案节点到链表
 * @param list 病案链表指针
 * @param node 要插入的病案节点
 *
 * 功能:
 *   采用头插法将新节点插入链表头部。
 *   时间复杂度O(1)。
 */
void insertRecordNode(RecordList* list, RecordNode* node);

/**
 * @brief 根据编号查找病案
 * @param list 病案链表指针
 * @param id 要查找的病案编号
 * @return 找到返回节点指针，未找到返回NULL
 *
 * 功能:
 *   在链表中线性搜索指定编号的病案。
 */
RecordNode* findRecordByID(RecordList* list, const char* id);

/**
 * @brief 根据编号删除病案
 * @param list 病案链表指针
 * @param id 要删除的病案编号
 * @return 删除成功返回1，失败返回0
 *
 * 功能:
 *   查找并删除指定病案，删除前会显示详情并要求确认。
 */
int deleteRecordByID(RecordList* list, const char* id);

/**
 * @brief 修改病案信息
 * @param node 要修改的病案节点指针
 *
 * 功能:
 *   提供选择性字段修改，用户可选择修改特定字段。
 *   可修改：入院诊断、出院诊断、治疗摘要、归档状态、备注。
 */
void modifyRecordInfo(RecordNode* node);

/* ==================== 显示和输出函数 ==================== */

/**
 * @brief 打印单条病案信息
 * @param node 病案节点指针
 *
 * 功能:
 *   格式化显示单个病案的完整详细信息。
 */
void printRecordOne(RecordNode* node);

/**
 * @brief 打印所有病案信息
 * @param list 病案链表指针
 *
 * 功能:
 *   遍历链表，打印所有病案的详细信息。
 */
void printRecordAll(RecordList* list);

/* ==================== 内存管理和菜单函数 ==================== */

/**
 * @brief 释放病案链表内存
 * @param list 病案链表指针
 *
 * 功能:
 *   释放链表中所有节点的动态分配内存。
 *   在程序退出前调用以防止内存泄漏。
 */
void freeRecordList(RecordList* list);

/**
 * @brief 病案管理主菜单
 * @param list 病案链表指针
 *
 * 功能:
 *   提供病案管理的交互式界面，支持增删改查等操作。
 *   菜单项：新增、查询、删除、修改、全部显示、返回。
 */
void recordMenu(RecordList* list);

#endif

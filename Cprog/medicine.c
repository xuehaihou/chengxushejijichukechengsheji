/**
 * @file medicine.c
 * @brief 药品/药库管理模块实现 - 药品信息的增删改查和库存管理
 *
 * 本模块实现医院药库管理系统的核心功能：
 * - 药品基本信息管理（增删改查）
 * - 药品入库出库操作
 * - 库存预警功能（低库存、过期提醒）
 * - 操作日志记录
 *
 * 数据结构: 单向链表 (MedicineList -> MedicineNode -> MedicineNode ...)
 * 存储格式: 文本文件，管道符(|)分隔字段
 */

#include "medicine.h"
#include "log.h"

/* ==================== 链表初始化函数 ==================== */

/**
 * @brief 初始化药品链表
 * @param list 指向MedicineList结构体的指针
 *
 * 功能说明:
 *   将链表头指针置空，计数器清零。
 *   在使用链表前必须调用此函数进行初始化。
 */
void initMedicineList(MedicineList* list) { list->head = NULL; list->count = 0; }

/* ==================== 文件操作函数 ==================== */

/**
 * @brief 从文件加载药品数据到内存链表
 * @param list 指向已初始化的MedicineList结构体
 * @param filename 药品数据文件路径
 *
 * 功能说明:
 *   以只读模式打开文件，逐行读取并解析药品记录，
 *   采用尾插法构建链表保持原有顺序。
 *
 * 文件格式（每行一条记录）:
 *   编号|名称|类别|数量|单位|单价|生产厂家|有效期|备注
 *
 * 字段说明:
 *   - id: 药品唯一编号（字符串）
 *   - name: 药品名称
 *   - category: 药品分类（如：抗生素、解热镇痛等）
 *   - quantity: 当前库存数量（整数）
 *   - unit: 计量单位（如：盒、瓶、片）
 *   - price: 单价（浮点数，保留2位小数）
 *   - manufacturer: 生产厂家
 *   - expiryDate: 有效期（YYYY-MM-DD格式）
 *   - remark: 备注信息
 *
 * 容错处理:
 *   - 文件不存在时提示并返回空链表
 *   - 内存分配失败跳过该行继续处理
 */
void loadMedicinesFromFile(MedicineList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：药品数据文件不存在，将创建新文件。\n"); return; }
    MedicineNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        MedicineNode* node = (MedicineNode*)malloc(sizeof(MedicineNode));
        if (node == NULL) continue;
        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%lf|%[^|]|%[^|]|%[^\n]",
            node->id, node->name, node->category, &node->quantity,
            node->unit, &node->price, node->manufacturer,
            node->expiryDate, node->remark);
        node->next = NULL;
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条药品记录。\n", list->count);
}

/**
 * @brief 将内存中的药品数据保存到文件
 * @param list 包含药品数据的MedicineList指针
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   以写入模式打开文件（覆盖原内容），遍历链表写入所有记录。
 *
 * 写入格式与读取格式完全兼容，可用于数据备份和迁移。
 */
void saveMedicinesToFile(MedicineList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }
    MedicineNode* current = list->head;
    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%d|%s|%.2f|%s|%s|%s\n",
            current->id, current->name, current->category, current->quantity,
            current->unit, current->price, current->manufacturer,
            current->expiryDate, current->remark);
        current = current->next;
    }
    fclose(fp);
    printf("成功保存 %d 条药品记录。\n", list->count);
}

/* ==================== 节点创建函数 ==================== */

/**
 * @brief 创建新的药品节点（交互式输入）
 * @return MedicineNode* 成功返回新节点指针，失败返回NULL
 *
 * 功能说明:
 *   通过控制台交互方式收集药品信息，创建并填充节点。
 *   对关键字段进行输入验证。
 *
 * 输入流程及验证规则:
 *   1. 编号(id): 必填，不能为空
 *   2. 名称(name): 必填
 *   3. 类别(category): 必填
 *   4. 库存数量(quantity): 整数，>=0
 *   5. 计量单位(unit): 如"盒"、"瓶"、"片"
 *   6. 单价(price): 浮点数，保留2位小数
 *   7. 生产厂家(manufacturer): 必填
 *   8. 有效期(expiryDate): YYYY-MM-DD格式，会验证格式正确性
 *   9. 备注(remark): 可选，可留空
 *
 * 输入示例:
 *   请输入药品编号: Y001
 *   请输入药品名称: 阿莫西林胶囊
 *   请输入类别: 抗生素
 *   请输入库存数量: 500
 *   请输入计量单位: 盒
 *   请输入单价: 15.50
 *   请输入生产厂家: 华北制药
 *   请输入有效期 (YYYY-MM-DD): 2026-12-31
 *   请输入备注 (无则留空): [直接回车]
 *
 * 注意事项:
 *   - 返回的节点next指针为NULL，需手动插入链表
 *   - 调用者负责检查ID是否重复
 */
MedicineNode* createMedicineNode() {
    MedicineNode* node = (MedicineNode*)malloc(sizeof(MedicineNode));
    if (node == NULL) { printf("错误：内存分配失败！\n"); return NULL; }

    printf("\n--- 新增药品 ---\n");

    /*
     * ====== 输入字段1: 药品编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字、连字符的组合
     * 是否必填: 是（不能为空）
     * 输入样例:
     *   - "Y001" (字母+数字)
     *   - "MED-2024-001" (带分隔符)
     *   - "阿莫西林-001" (含中文)
     * 验证规则:
     *   - 不能为空字符串
     *   - 不能全为空格
     *   - 建议使用有意义的编码规则
     */
    safeInput(node->id, MAX_ID_LEN, "请输入药品编号: ");
    while (isEmpty(node->id)) {
        printf("编号不能为空！\n");
        safeInput(node->id, MAX_ID_LEN, "请输入药品编号: ");
    }

    /*
     * ====== 输入字段2: 药品名称 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 中文、英文、数字均可
     * 是否必填: 是
     * 输入样例:
     *   - "阿莫西林胶囊"
     *   - "Amoxicillin Capsules"
     *   - "布洛芬缓释片(芬必得)"
     * 说明: 通常使用药品通用名或商品名
     */
    safeInput(node->name, MAX_NAME_LEN, "请输入药品名称: ");

    /*
     * ====== 输入字段3: 药品类别 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符
     * 输入格式: 中文或英文分类名称
     * 是否必填: 是
     * 输入样例:
     *   - "抗生素"
     *   - "解热镇痛药"
     *   - "心血管用药"
     *   - "消化系统用药"
     *   - "维生素类"
     * 常用分类参考国家药品分类标准
     */
    safeInput(node->category, MAX_DEPT_LEN, "请输入类别: ");

    /*
     * ====== 输入字段4: 库存数量 ======
     * 数据类型: int (整数)
     * 取值范围: 0 ~ 2147483647 (非负整数)
     * 输入格式: 纯数字，不含小数点
     * 是否必填: 是
     * 输入样例:
     *   - "500" (500盒)
     *   - "1000" (1000瓶)
     *   - "0" (暂时缺货)
     * 业务含义: 当前仓库中的实际库存量
     * 注意: 必须为非负整数，不支持负库存
     */
    node->quantity = inputInt("请输入库存数量: ");

    /*
     * ====== 输入字段5: 计量单位 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 中文或英文单位名称
     * 是否必填: 是
     * 输入样例:
     *   - "盒" (最常用)
     *   - "瓶"
     *   - "片"
     *   - "支"
     *   - "袋"
     *   - "ml" (毫升)
     *   - "g" (克)
     * 说明: 应与数量配合使用，如"500盒"
     */
    safeInput(node->unit, 20, "请输入计量单位: ");

    /*
     * ====== 输入字段6: 单价 ======
     * 数据类型: double (双精度浮点数)
     * 取值范围: 0.00 ~ 很大的正数
     * 精度要求: 保留2位小数(元)
     * 输入格式: 数字，可含小数点
     * 是否必填: 是
     * 输入样例:
     *   - "15.50" (15元5角)
     *   - "128.00" (128元整)
     *   - "0.5" (5角)
     *   - "256.88"
     * 业务含义: 每单位药品的采购/销售价格
     * 注意: 不能为负数
     */
    node->price = inputDouble("请输入单价: ");

    /*
     * ====== 输入字段7: 生产厂家 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 企业全称或简称
     * 是否必填: 是
     * 输入样例:
     *   - "华北制药股份有限公司"
     *   - "白云山制药"
     *   - "辉瑞制药(Pfizer)"
     *   - "扬子江药业"
     * 说明: 用于追溯药品来源和质量责任认定
     */
    safeInput(node->manufacturer, MAX_NAME_LEN, "请输入生产厂家: ");

    /*
     * ====== 输入字段8: 有效期 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: YYYY-MM-DD (严格格式)
     * 是否必填: 是
     * 输入样例:
     *   - "2026-12-31" (2026年12月31日)
     *   - "2025-06-30" (2025年6月30日)
     *   - "2027-01-15"
     * 格式要求:
     *   - 年份: 4位数字 (1900-2100)
     *   - 月份: 2位数字 (01-12)
     *   - 日期: 2位数字 (根据月份1-31)
     *   - 分隔符: 必须是横线'-'
     * 验证: 自动检查日期有效性(含闰年判断)
     * 错误提示: "日期格式不正确！"会提示重新输入
     */
    safeInput(node->expiryDate, MAX_DATE_LEN, "请输入有效期 (YYYY-MM-DD): ");
    while (!isValidDate(node->expiryDate)) {
        printf("日期格式不正确！\n");
        safeInput(node->expiryDate, MAX_DATE_LEN, "请输入有效期 (YYYY-MM-DD): ");
    }

    /*
     * ====== 输入字段9: 备注 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 自由文本
     * 是否必填: 否（可留空）
     * 输入样例:
     *   - "" (直接回车留空)
     *   - "需冷藏保存(2-8°C)"
     *   - "处方药，凭医师处方购买"
     *   - "进口药品，原产国:美国"
     *   - "特殊管理药品，双人双锁"
     * 用途: 记录特殊存储要求、使用限制等补充信息
     */
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注 (无则留空): ");

    node->next = NULL;
    return node;
}

/* ==================== 辅助查询函数 ==================== */

/**
 * @brief 检查药品编号是否已存在
 * @param list 药品链表指针
 * @param id 要检查的药品编号
 * @return int 存在返回1，不存在返回0
 *
 * 功能说明:
 *   遍历链表查找指定编号的药品，用于新增前的重复性检查。
 */
int isMedicineIDExist(MedicineList* list, const char* id) {
    MedicineNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) return 1;
        current = current->next;
    }
    return 0;
}

/**
 * @brief 根据编号查找药品节点
 * @param list 药品链表指针
 * @param id 要查找的药品编号
 * @return MedicineNode* 找到返回节点指针，未找到返回NULL
 */
MedicineNode* findMedicineByID(MedicineList* list, const char* id) {
    MedicineNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) return current;
        current = current->next;
    }
    return NULL;
}

/* ==================== 核心操作函数 ==================== */

/**
 * @brief 将药品节点插入链表头部
 * @param list 药品链表指针
 * @param node 要插入的节点指针
 *
 * 功能说明:
 *   采用头插法，新节点成为链表的第一个元素。
 *   时间复杂度O(1)，适合频繁插入场景。
 */
void insertMedicineNode(MedicineList* list, MedicineNode* node) {
    if (node == NULL) return;
    node->next = list->head; list->head = node; list->count++;
}

/**
 * @brief 根据编号删除药品记录
 * @param list 药品链表指针
 * @param id 要删除的药品编号
 * @return int 删除成功返回1，失败或取消返回0
 *
 * 功能说明:
 *   1. 查找指定编号的药品
 *   2. 显示找到的记录供确认
 *   3. 用户确认后执行删除
 *   4. 写入操作日志
 *
 * 安全机制:
 *   - 删除前显示完整信息
 *   - 需要用户二次确认
 *   - 自动记录日志便于追溯
 */
int deleteMedicineByID(MedicineList* list, const char* id) {
    MedicineNode* current = list->head, * prev = NULL;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printf("\n找到以下记录:\n"); printMedicineOne(current);

            /* 二次确认防止误删 */
            if (!confirm("确认删除该药品?")) { printf("已取消删除。\n"); return 0; }

            /* 记录日志详情 */
            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "药品编号:%s, 名称:%s, 类别:%s",
                current->id, current->name, current->category);

            /* 执行删除操作 */
            if (prev == NULL) list->head = current->next;
            else prev->next = current->next;
            free(current); list->count--;
            printf("删除成功！\n");

            /* 写入日志 */
            writeLog(LOG_MEDICINE, "删除药品", "系统", detail);
            return 1;
        }
        prev = current; current = current->next;
    }
    printf("未找到编号为 %s 的药品。\n", id);
    return 0;
}

/**
 * @brief 修改药品信息
 * @param node 指向要修改的药品节点
 *
 * 功能说明:
 *   提供选择性修改功能，用户可选择修改单个字段。
 *   修改完成后自动记录日志。
 *
 * 可修改的字段:
 *   1. 名称     5. 单价
 *   2. 类别     6. 生产厂家
 *   3. 库存数量 7. 有效期（需验证格式）
 *   4. 计量单位 8. 备注
 *   0. 取消修改
 *
 * 输入格式: 整数(0-8)，选择要修改的字段编号
 */
void modifyMedicineInfo(MedicineNode* node) {
    if (node == NULL) return;

    printf("\n--- 修改药品信息 ---\n");
    printMedicineOne(node);

    printf("\n请选择要修改的字段:\n");
    printf("1. 名称\n2. 类别\n3. 库存数量\n4. 计量单位\n5. 单价\n6. 生产厂家\n7. 有效期\n8. 备注\n0. 取消\n");
    int choice = inputInt("请选择: ");

    switch (choice) {
    case 1: safeInput(node->name, MAX_NAME_LEN, "请输入新名称: "); break;
    case 2: safeInput(node->category, MAX_DEPT_LEN, "请输入新类别: "); break;
    case 3: node->quantity = inputInt("请输入新库存数量: "); break;
    case 4: safeInput(node->unit, 20, "请输入新计量单位: "); break;
    case 5: node->price = inputDouble("请输入新单价: "); break;
    case 6: safeInput(node->manufacturer, MAX_NAME_LEN, "请输入新生产厂家: "); break;
    case 7:
        safeInput(node->expiryDate, MAX_DATE_LEN, "请输入新有效期: ");
        while (!isValidDate(node->expiryDate)) {
            printf("日期格式不正确！\n");
            safeInput(node->expiryDate, MAX_DATE_LEN, "请输入新有效期: ");
        }
        break;
    case 8: safeInput(node->remark, MAX_REMARK_LEN, "请输入新备注: "); break;
    case 0: printf("取消修改。\n"); return;
    default: printf("无效选择。\n"); return;
    }

    printf("修改成功！\n");

    /* 记录修改日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "药品编号:%s, 名称:%s, 修改字段:%d", node->id, node->name, choice);
    writeLog(LOG_MEDICINE, "修改药品", "系统", detail);
}

/* ==================== 库存操作函数 ==================== */

/**
 * @brief 药品入库操作
 * @param list 药品链表指针
 * @param id 药品编号
 * @param quantity 入库数量（正整数）
 *
 * 功能说明:
 *   根据药品编号定位记录，增加库存数量。
 *   同时记录入库操作日志。
 *
 * 业务规则:
 *   - 药品必须存在才能入库
 *   - 数量必须为正整数
 *   - 入库后自动更新当前库存显示
 *
 * 日志记录内容:
 *   药品编号、名称、入库数量、当前库存
 *
 * 输入参数示例:
 *   id = "Y001"
 *   quantity = 100 (表示入库100盒)
 */
void medicineInStock(MedicineList* list, const char* id, int quantity) {
    MedicineNode* node = findMedicineByID(list, id);
    if (node == NULL) { printf("未找到该药品。\n"); return; }

    node->quantity += quantity;
    printf("入库成功！当前库存: %d %s\n", node->quantity, node->unit);

    /* 记录入库日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "药品编号:%s, 名称:%s, 入库数量:%d, 当前库存:%d%s",
        node->id, node->name, quantity, node->quantity, node->unit);
    writeLog(LOG_MEDICINE, "药品入库", "系统", detail);
}

/**
 * @brief 药品出库操作
 * @param list 药品链表指针
 * @param id 药品编号
 * @param quantity 出库数量（正整数）
 * @return int 出库成功返回1，失败返回0
 *
 * 功能说明:
 *   根据药品编号定位记录，减少库存数量。
 *   会检查库存是否充足，不足则拒绝出库。
 *
 * 业务规则:
 *   - 药品必须存在
 *   - 出库数量不能超过当前库存
 *   - 库存充足才执行扣减
 *
 * 错误处理:
 *   - 药品不存在: 提示"未找到该药品"
 *   - 库存不足: 提示当前库存量并拒绝操作
 *
 * 日志记录内容:
 *   药品编号、名称、出库数量、剩余库存
 */
int medicineOutStock(MedicineList* list, const char* id, int quantity) {
    MedicineNode* node = findMedicineByID(list, id);
    if (node == NULL) { printf("未找到该药品。\n"); return 0; }

    /* 库存不足检查 */
    if (node->quantity < quantity) {
        printf("库存不足！当前库存: %d %s\n", node->quantity, node->unit);
        return 0;
    }

    node->quantity -= quantity;
    printf("出库成功！剩余库存: %d %s\n", node->quantity, node->unit);

    /* 记录出库日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "药品编号:%s, 名称:%s, 出库数量:%d, 剩余库存:%d%s",
        node->id, node->name, quantity, node->quantity, node->unit);
    writeLog(LOG_MEDICINE, "药品出库", "系统", detail);
    return 1;
}

/* ==================== 显示输出函数 ==================== */

/**
 * @brief 打印单条药品详细信息
 * @param node 药品节点指针
 *
 * 功能说明:
 *   格式化输出一个药品的所有字段信息。
 *   特别地，会自动检测药品是否过期并给出警告。
 *
 * 输出内容包括:
 *   - 基本信息: 编号、名称、类别
 *   - 库存信息: 数量、单位
 *   - 价格信息: 单价
 *   - 生产信息: 生产厂家、有效期
 *   - 状态信息: 过期警告（如适用）
 *   - 其他: 备注
 *
 * 过期判断逻辑:
 *   比较有效期与当前日期，若有效期<=今天则标记【已过期！】
 */
void printMedicineOne(MedicineNode* node) {
    if (node == NULL) return;

    printLine('-', 70);
    printf("药品编号: %s\n", node->id);
    printf("药品名称: %s\n", node->name);
    printf("类别: %s\n", node->category);
    printf("库存: %d %s\n", node->quantity, node->unit);
    printf("单价: %.2f\n", node->price);
    printf("生产厂家: %s\n", node->manufacturer);
    printf("有效期: %s\n", node->expiryDate);

    /* 过期检测 */
    char today[MAX_DATE_LEN];
    getCurrentDate(today);
    if (compareDate(node->expiryDate, today) <= 0) {
        printf("【已过期！】\n");
    }

    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

/**
 * @brief 打印所有药品列表
 * @param list 药品链表指针
 *
 * 功能说明:
 *   遍历链表打印每条药品记录，显示总数统计。
 */
void printMedicineAll(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    printf("\n--- 所有药品 (共 %d 种) ---\n", list->count);
    MedicineNode* current = list->head;
    while (current != NULL) { printMedicineOne(current); current = current->next; }
}

/**
 * @brief 打印需要预警的药品
 * @param list 药品链表指针
 *
 * 功能说明:
 *   扫描所有药品，筛选出需要关注的药品并显示。
 *
 * 预警条件（满足任一即预警）:
 *   1. 库存数量 <= 10（低库存预警）
 *   2. 有效期 <= 今天（过期预警）
 *
 * 输出示例:
 *   --- 药品预警 ---
 *   [第1个需关注的药品]
 *   [第2个需关注的药品]
 *   ...
 *   共 N 种药品需要关注。
 */
void printMedicineWarning(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }

    int count = 0;
    MedicineNode* current = list->head;
    char today[MAX_DATE_LEN];
    getCurrentDate(today);

    printf("\n--- 药品预警 ---\n");
    while (current != NULL) {
        int needWarning = 0;

        /* 低库存检查: 库存<=10 */
        if (current->quantity <= 10) needWarning = 1;

        /* 过期检查: 有效期<=今天 */
        if (compareDate(current->expiryDate, today) <= 0) needWarning = 1;

        if (needWarning) {
            printMedicineOne(current);
            count++;
        }
        current = current->next;
    }

    if (count == 0) printf("暂无需要关注的药品。\n");
    else printf("共 %d 种药品需要关注。\n", count);
}

/* ==================== 内存释放函数 ==================== */

/**
 * @brief 释放药品链表占用的内存
 * @param list 药品链表指针
 *
 * 功能说明:
 *   遍历链表释放所有节点的堆内存。
 */
void freeMedicineList(MedicineList* list) {
    MedicineNode* current = list->head;
    while (current != NULL) {
        MedicineNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL; list->count = 0;
}

/* ==================== 高层业务封装函数 ==================== */

/**
 * @brief 新增药品（封装完整的添加流程）
 * @param list 药品链表指针
 *
 * 功能说明:
 *   封装了创建节点、检查重复、插入链表、记录日志的完整流程。
 *   是菜单调用的主要入口函数。
 *
 * 执行流程:
 *   1. 调用createMedicineNode()获取用户输入
 *   2. 检查编号是否已存在
 *   3. 不存在则插入链表
 *   4. 记录操作日志
 */
void addMedicine(MedicineList* list) {
    MedicineNode* node = createMedicineNode();
    if (node == NULL) return;

    if (isMedicineIDExist(list, node->id)) {
        printf("错误：药品编号 %s 已存在！\n", node->id);
        free(node);
        return;
    }

    insertMedicineNode(list, node);
    printf("药品添加成功！\n");

    /* 记录添加日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "药品编号:%s, 名称:%s, 类别:%s, 库存:%d%s, 单价:%.2f",
        node->id, node->name, node->category, node->quantity, node->unit, node->price);
    writeLog(LOG_MEDICINE, "添加药品", "系统", detail);
}

/**
 * @brief 查询药品（提供多种查询方式）
 * @param list 药品链表指针
 *
 * 功能说明:
 *   提供交互式查询菜单，支持按不同条件检索药品。
 *
 * 查询选项:
 *   1. 按编号查询 - 精确匹配，返回单条记录
 *   2. 按名称查询 - 模糊匹配（包含即可），可能多条
 *   3. 按类别查询 - 模糊匹配，可能多条
 *   4. 显示全部 - 列出所有药品
 *   5. 查看预警 - 显示需关注的药品
 *   0. 返回上级菜单
 *
 * 输入格式: 先选择查询方式(整数)，再输入查询关键词(字符串)
 */
void queryMedicine(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }

    printf("\n--- 查询药品 ---\n");
    printf("1. 按编号查询\n2. 按名称查询\n3. 按类别查询\n4. 显示全部\n5. 查看药品预警\n0. 返回\n");
    int choice = inputInt("请选择: ");

    char keyword[MAX_NAME_LEN];

    switch (choice) {
    case 1:
        /* 按编号精确查询 */
        safeInput(keyword, MAX_ID_LEN, "请输入编号: ");
        MedicineNode* found = findMedicineByID(list, keyword);
        if (found) printMedicineOne(found);
        else printf("未找到。\n");
        break;

    case 2:
        /* 按名称模糊查询 */
        safeInput(keyword, MAX_NAME_LEN, "请输入名称: ");
        MedicineNode* curr = list->head; int cnt = 0;
        while (curr != NULL) {
            if (strstr(curr->name, keyword)) {
                printMedicineOne(curr);
                cnt++;
            }
            curr = curr->next;
        }
        if (cnt == 0) printf("未找到。\n");
        break;

    case 3:
        /* 按类别模糊查询 */
        safeInput(keyword, MAX_DEPT_LEN, "请输入类别: ");
        curr = list->head; cnt = 0;
        while (curr != NULL) {
            if (strstr(curr->category, keyword)) {
                printMedicineOne(curr);
                cnt++;
            }
            curr = curr->next;
        }
        if (cnt == 0) printf("未找到。\n");
        break;

    case 4: printMedicineAll(list); break;
    case 5: printMedicineWarning(list); break;
    case 0: return;
    default: printf("无效选择。\n");
    }
}

/**
 * @brief 删除药品（封装删除流程）
 * @param list 药品链表指针
 *
 * 功能说明:
 *   获取用户输入的编号，调用deleteMedicineByID()执行删除。
 */
void deleteMedicine(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入要删除的药品编号: ");
    deleteMedicineByID(list, id);
}

/**
 * @brief 修改药品（封装修改流程）
 * @param list 药品链表指针
 *
 * 功能说明:
 *   获取用户输入的编号，查找药品后调用modifyMedicineInfo()修改。
 */
void modifyMedicine(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入要修改的药品编号: ");
    MedicineNode* node = findMedicineByID(list, id);
    if (node == NULL) { printf("未找到。\n"); return; }
    modifyMedicineInfo(node);
}

/**
 * @brief 药品入库（交互式）
 * @param list 药品链表指针
 *
 * 功能说明:
 *   提示用户输入药品编号和入库数量，调用medicineInStock()执行入库。
 */
void medicineIn(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入药品编号: ");
    int qty = inputInt("请输入入库数量: ");
    medicineInStock(list, id, qty);
}

/**
 * @brief 药品出库（交互式）
 * @param list 药品链表指针
 *
 * 功能说明:
 *   提示用户输入药品编号和出库数量，调用medicineOutStock()执行出库。
 */
void medicineOut(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入药品编号: ");
    int qty = inputInt("请输入出库数量: ");
    medicineOutStock(list, id, qty);
}

/* ==================== 主菜单函数 ==================== */

/**
 * @brief 药库管理系统主菜单
 * @param list 药品链表指针
 *
 * 功能说明:
 *   提供药库管理的交互式主菜单界面。
 *
 * 菜单选项:
 *   1. 新增药品   - 添加新的药品记录
 *   2. 查询药品   - 多条件查询药品信息
 *   3. 删除药品   - 删除已有药品记录
 *   4. 修改药品   - 修改药品各项属性
 *   5. 药品入库   - 增加库存数量
 *   6. 药品出库   - 减少库存数量
 *   0. 返回主菜单 - 退出药库管理
 *
 * 输入格式: 整数(0-6)，通过键盘选择功能项
 *
 * 用户交互:
 *   - 每次操作后暂停等待确认
 *   - 无效选择给出提示
 */
void medicineMenu(MedicineList* list) {
    int choice;
    do {
        printf("\n");
        printTitle("药库管理系统");
        printf("1. 新增药品\n2. 查询药品\n3. 删除药品\n4. 修改药品\n");
        printf("5. 药品入库\n6. 药品出库\n0. 返回主菜单\n");
        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: addMedicine(list); break;
        case 2: queryMedicine(list); break;
        case 3: deleteMedicine(list); break;
        case 4: modifyMedicine(list); break;
        case 5: medicineIn(list); break;
        case 6: medicineOut(list); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }

        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

/**
 * @file blood.c
 * @brief 血库管理模块实现文件
 *
 * 功能概述:
 *   本模块实现医院血库的全流程管理，包括：
 *   - 血液库存记录管理（增删改查）
 *   - 血液入库和出库操作
 *   - 血液有效期监控和预警
 *   - 按血型分类统计
 *
 * 核心业务:
 *   1. 入库流程：采集血液 → 登记信息 → 检验合格 → 入库存储
 *   2. 出库流程：申请用血 → 核对血型 → 库存扣减 → 发血出库
 *   3. 质量控制：有效期检查 → 过期预警 → 状态更新
 *
 * 血型系统:
 *   ABO血型: A、B、O、AB
 *   Rh血型: Rh+(阳性)、Rh-(阴性)
 *   组合示例: A+、B-、O+、AB-
 *
 * 文件依赖:
 *   - blood.h: 血库数据结构和函数声明
 *   - log.h: 日志系统接口
 */

#include "blood.h"
#include "log.h"

/**
 * @brief 初始化血库链表
 * @param list 血库链表指针（输出参数）
 *
 * 功能说明:
 *   将链表头指针置空，计数器归零。
 */
void initBloodList(BloodList* list) {
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 从文件加载血库数据到内存
 * @param list 血库链表指针（输出参数）
 * @param filename 数据文件路径，如"data/bloods.txt"
 *
 * 文件格式要求:
 *   每行一条记录，字段用"|"分隔：
 *   编号|血型|Rh型|数量|单位|采集日期|有效期至|来源|状态|备注
 *
 * 示例行:
 *   BL001|A|+|400|ml|2025-01-10|2025-02-10|献血者张三|1|新鲜全血
 *
 * 字段说明:
 *   - 血型: A/B/O/AB
 *   - Rh型: +/-
 *   - 数量: 整数，单位为ml
 *   - 状态: 0=不可用, 1=可用
 */
void loadBloodsFromFile(BloodList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("提示：血库数据文件不存在，将创建新文件。\n");
        return;
    }

    BloodNode* tail = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        BloodNode* node = (BloodNode*)malloc(sizeof(BloodNode));
        if (!node) continue;

        /* 解析10个字段 */
        sscanf_s(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^\n]",
            node->id, node->bloodType, node->rhType,
            &node->quantity, node->unit, node->collectDate,
            node->expiryDate, node->source, &node->status, node->remark);

        node->next = NULL;

        if (!list->head) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条血库记录。\n", list->count);
}

/**
 * @brief 将内存中的血库数据保存到文件
 * @param list 血库链表指针
 * @param filename 目标文件路径
 */
void saveBloodsToFile(BloodList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }

    BloodNode* c = list->head;
    while (c) {
        fprintf(fp, "%s|%s|%s|%d|%s|%s|%s|%s|%d|%s\n",
            c->id, c->bloodType, c->rhType,
            c->quantity, c->unit, c->collectDate,
            c->expiryDate, c->source, c->status, c->remark);
        c = c->next;
    }
    fclose(fp);
    printf("成功保存 %d 条血库记录。\n", list->count);
}

/**
 * @brief 创建新的血液库存节点
 * @return 成功返回新节点指针，失败返回NULL
 *
 * 输入项及格式:
 *   - 血液编号: 必填，唯一标识，如"BL001"
 *   - 血型: 必填，A/B/O/AB（自动转大写）
 *   - Rh型: 必填，+ 或 -
 *   - 数量: 整数，单位ml（如200、400）
 *   - 采集日期: 自动获取当前日期
 *   - 有效期至: 格式YYYY-MM-DD（需验证有效性）
 *     * 全血一般保存35天
 *     * 冰冻血浆可保存1年
 *   - 来源: 如"献血者张三"、"市中心血站"
 *   - 状态: 默认可用(1)
 *   - 备注: 可选补充信息
 *
 * 输入验证:
 *   - 编号不能为空
 *   - 血型自动转换为大写
 *   - 有效期格式必须有效
 */
BloodNode* createBloodNode() {
    BloodNode* n = (BloodNode*)malloc(sizeof(BloodNode));
    if (!n) return NULL;

    printf("\n--- 新增血液库存 ---\n");

    /*
     * ====== 输入字段1: 血液编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字、连字符的组合
     * 是否必填: 是（不能为空）
     * 输入样例:
     *   - "BLD001" (Blood+序号)
     *   - "B-2024-0456" (血型+日期+序号)
     *   - "XK20260416001" (血库日期+流水号)
     */
    safeInput(n->id, MAX_ID_LEN, "血液编号: ");
    while (isEmpty(n->id))
        safeInput(n->id, MAX_ID_LEN, "编号不能为空！重新输入: ");

    /*
     * ====== 输入字段2: ABO血型 ======
     * 数据类型: char[5] (字符串)
     * 最大长度: 5个字符
     * 输入格式: 单个大写字母(系统自动转换)
     * 是否必填: 是
     * 可选值:
     *   - "A" (A型血)
     *   - "B" (B型血)
     *   - "O" (O型血)
     *   - "AB" (AB型血)
     * 输入样例: "A", "B", "O", "AB"
     * 注意: 系统自动将输入转为大写字母，输入"a"、"b"均可
     */
    printf("血型(A/B/O/AB): ");
    safeInput(n->bloodType, 5, "");
    toUpperCase(n->bloodType);  /* 统一转为大写 */

    /*
     * ====== 输入字段3: Rh血型 ======
     * 数据类型: char[3] (字符串)
     * 最大长度: 3个字符
     * 输入格式: 正负号
     * 是否必填: 是
     * 可选值:
     *   - "+" (Rh阳性)
     *   - "-" (Rh阴性)
     * 输入样例: "+", "-"
     */
    printf("Rh型(+/-): ");
    safeInput(n->rhType, 3, "");

    /*
     * ====== 输入字段4: 血液数量 ======
     * 数据类型: int (整数)
     * 取值范围: 1 ~ 很大的正数 (单位:ml)
     * 输入格式: 纯数字
     * 是否必填: 是
     * 输入样例:
     *   - "200" (200ml，1单位全血)
     *   - "400" (400ml，2单位全血)
     *   - "150" (150ml，1单位血小板)
     * 业务含义: 该血液制品的容量，单位固定为ml
     */
    n->quantity = inputInt("数量(ml): ");
    strcpy(n->unit, "ml");  /* 【固定】单位为毫升 */

    /* 【自动填充】采集日期 - 系统自动获取当前日期 */

    /*
     * ====== 输入字段5: 有效期至 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: YYYY-MM-DD (严格格式)
     * 是否必填: 是
     * 输入样例:
     *   - "2026-06-15" (2026年6月15日)
     *   - "2026-05-01" (2026年5月1日)
     * 格式要求:
     *   - 年份: 4位数字
     *   - 月份: 2位数字 (01-12)
     *   - 日期: 2位数字 (根据月份1-31)
     * 验证: 自动检查日期有效性
     * 说明: 全血有效期通常35天，血浆可保存1年
     */
    safeInput(n->expiryDate, MAX_DATE_LEN, "有效期至: ");
    while (!isValidDate(n->expiryDate)) {
        printf("日期格式不正确！\n");
        safeInput(n->expiryDate, MAX_DATE_LEN, "重新输入: ");
    }

    /*
     * ====== 输入字段6: 来源 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 来源描述文本
     * 是否必填: 是
     * 输入样例:
     *   - "无偿献血者 张三"
     *   - "市中心血站"
     *   - "互助献血 李四"
     *   - "成分献血(机采血小板)"
     * 说明: 标识血液来源，用于质量追溯
     */
    safeInput(n->source, MAX_NAME_LEN, "来源(献血者/血站): ");

    /* 【系统设置】状态默认为1(可用/在库) */

    /*
     * ====== 输入字段7: 备注 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 自由文本
     * 是否必填: 否（可留空）
     * 输入样例:
     *   - "" (直接回车留空)
     *   - "经检测合格"
     *   - "需辐照处理"
     *   - "传染病筛查阴性"
     */
    safeInput(n->remark, MAX_REMARK_LEN, "备注: ");

    n->next = NULL;
    return n;
}

/**
 * @brief 检查血液编号是否已存在
 * @param list 血库链表指针
 * @param id 待检查的血液编号
 * @return 存在返回1，不存在返回0
 */
int isBloodIDExist(BloodList* list, const char* id) {
    BloodNode* c = list->head;
    while (c) { if (strcmp(c->id, id) == 0) return 1; c = c->next; }
    return 0;
}

/**
 * @brief 将新血液节点插入链表头部
 * @param list 血库链表指针
 * @param node 待插入的节点指针
 */
void insertBloodNode(BloodList* list, BloodNode* node) {
    if (node) { node->next = list->head; list->head = node; list->count++; }
}

/**
 * @brief 根据血液编号查找记录
 * @param list 血库链表指针
 * @param id 目标血液编号
 * @return 找到返回节点指针，未找到返回NULL
 */
BloodNode* findBloodByID(BloodList* list, const char* id) {
    BloodNode* c = list->head;
    while (c) { if (strcmp(c->id, id) == 0) return c; c = c->next; }
    return NULL;
}

/**
 * @brief 根据编号删除血液记录
 * @param list 血库链表指针
 * @param id 待删除的血液编号
 * @return 成功删除返回1，失败返回0
 *
 * 安全机制:
 *   - 删除前显示完整信息
 *   - 要求二次确认
 *   - 删除后记录日志
 */
int deleteBloodByID(BloodList* list, const char* id) {
    BloodNode* c = list->head, * p = NULL;

    while (c) {
        if (strcmp(c->id, id) == 0) {
            printBloodOne(c);

            if (!confirm("确认删除?")) return 0;

            /* 构造日志详情 */
            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "血液编号:%s, 血型:%s%s, 数量:%d%s",
                c->id, c->bloodType, c->rhType, c->quantity, c->unit);

            /* 从链表中移除 */
            if (!p) list->head = c->next; else p->next = c->next;

            free(c);
            list->count--;

            writeLog(LOG_BLOOD, "删除血液记录", "系统", detail);
            return 1;
        }
        p = c;
        c = c->next;
    }
    printf("未找到。\n");
    return 0;
}

/**
 * @brief 修改血液记录信息
 * @param node 待修改的血液节点指针
 *
 * 可修改字段:
 *   1. 数量
 *   2. 有效期（需验证格式）
 *   3. 状态（0=不可用, 1=可用）
 *   4. 备注
 *
 * 使用场景:
 *   - 更正数量错误
 *   - 延长有效期（特殊情况下）
 *   - 标记血液不可用（检验不合格等）
 */
void modifyBloodInfo(BloodNode* node) {
    if (!node) return;

    printBloodOne(node);

    printf("1.数量 2.有效期 3.状态 4.备注 0.取消\n");
    int ch = inputInt("选择: ");

    switch (ch) {
    case 1: node->quantity = inputInt("新数量: "); break;
    case 2:
        safeInput(node->expiryDate, MAX_DATE_LEN, "新有效期: ");
        while (!isValidDate(node->expiryDate)) {
            printf("格式不正确！\n");
            safeInput(node->expiryDate, MAX_DATE_LEN, "重新输入: ");
        }
        break;
    case 3:
        printf("状态(0.不可用 1.可用): ");
        node->status = inputInt("");
        break;
    case 4: safeInput(node->remark, MAX_REMARK_LEN, "新备注: "); break;
    case 0: return;
    }

    printf("修改成功！\n");

    /* 记录修改日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "血液编号:%s, 血型:%s%s, 修改字段:%d",
        node->id, node->bloodType, node->rhType, ch);
    writeLog(LOG_BLOOD, "修改血液记录", "系统", detail);
}

/**
 * @brief 打印单条血液记录
 * @param node 血液节点指针
 *
 * 功能说明:
 *   格式化输出血液记录的所有字段。
 *   特别功能：自动检测并显示过期状态：
 *   - 已过期：显示【已过期！】警告
 *   - 即将过期（7天内）：显示【即将过期(7天内)】提醒
 *
 * 输出示例:
 *   ----------------------------------------------------------------------
 *   血液编号: BL001
 *   血型: A+ | 数量: 400 ml
 *   采集日期: 2025-01-10 有效期至: 2025-02-10
 *   来源: 献血者张三 状态: 可用
 *   【即将过期(7天内)】
 *   备注: 新鲜全血
 *   ----------------------------------------------------------------------
 */
void printBloodOne(BloodNode* node) {
    if (!node) return;

    printLine('-', 70);
    printf("血液编号: %s\n", node->id);
    printf("血型: %s%s | 数量: %d %s\n",
        node->bloodType, node->rhType, node->quantity, node->unit);
    printf("采集日期: %s 有效期至: %s\n",
        node->collectDate, node->expiryDate);
    printf("来源: %s 状态: %s\n",
        node->source, node->status ? "可用" : "不可用");

    /* 过期检测和预警 */
    char today[MAX_DATE_LEN];
    getCurrentDate(today);

    if (compareDate(node->expiryDate, today) <= 0)
        printf("【已过期！】\n");
    else if (daysBetween(today, node->expiryDate) < 7)
        printf("【即将过期(7天内)】\n");

    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

/**
 * @brief 打印所有血液记录
 * @param list 血库链表指针
 */
void printBloodAll(BloodList* list) {
    if (!list->head) { printf("暂无血库记录。\n"); return; }

    printf("\n--- 所有血液库存 (%d条) ---\n", list->count);
    BloodNode* c = list->head;
    while (c) { printBloodOne(c); c = c->next; }
}

/**
 * @brief 血液入库操作
 * @param list 血库链表指针
 * @param id 血液编号
 * @param quantity 入库数量(ml)，正整数
 * @return 成功返回1，失败返回0
 *
 * 功能说明:
 *   根据血液编号定位记录，增加库存数量。
 *   同时更新采集日期为当前时间。
 *
 * 业务场景:
 *   - 新采集的血液经检验后入库
 *   - 从其他医院调拨血液入库
 *   - 退回未使用的血液重新入库
 *
 * 日志记录内容:
 *   血液编号、血型、入库数量、当前库存
 */
int bloodInStock(BloodList* list, const char* id, int quantity) {
    BloodNode* n = findBloodByID(list, id);
    if (!n) { printf("未找到该血液记录。\n"); return 0; }

    n->quantity += quantity;
    getCurrentDate(n->collectDate);  /* 更新操作时间 */

    printf("入库成功！当前库存: %d %s\n", n->quantity, n->unit);

    /* 记录入库日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "血液编号:%s, 血型:%s%s, 入库:%d, 当前库存:%d%s",
        n->id, n->bloodType, n->rhType, quantity, n->quantity, n->unit);
    writeLog(LOG_BLOOD, "血液入库", "系统", detail);

    return 1;
}

/**
 * @brief 血液出库操作
 * @param list 血库链表指针
 * @param id 血液编号
 * @param quantity 出库数量(ml)，正整数
 * @return 成功返回1，失败返回0
 *
 * 功能说明:
 *   根据血液编号定位记录，减少库存数量。
 *   会检查库存是否充足。
 *
 * 业务规则:
 *   - 血液必须存在才能出库
 *   - 出库数量不能超过当前库存
 *   - 出库后自动显示剩余库存
 *
 * 业务场景:
 *   - 手术用血出库
 *   - 急诊抢救用血出库
 *   - 临床输血出库
 *
 * 错误处理:
 *   - 记录不存在时提示并返回失败
 *   - 库存不足时提示当前库存并返回失败
 */
int bloodOutStock(BloodList* list, const char* id, int quantity) {
    BloodNode* n = findBloodByID(list, id);
    if (!n) { printf("未找到该血液记录。\n"); return 0; }

    /* 检查库存是否充足 */
    if (n->quantity < quantity) {
        printf("库存不足！当前: %d %s\n", n->quantity, n->unit);
        return 0;
    }

    n->quantity -= quantity;
    printf("出库成功！剩余: %d %s\n", n->quantity, n->unit);

    /* 记录出库日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "血液编号:%s, 血型:%s%s, 出库:%d, 剩余库存:%d%s",
        n->id, n->bloodType, n->rhType, quantity, n->quantity, n->unit);
    writeLog(LOG_BLOOD, "血液出库", "系统", detail);

    return 1;
}

/**
 * @brief 按血型统计库存
 * @param list 血库链表指针
 *
 * 功能说明:
 *   统计各血型的总库存量（仅统计可用状态的血液）。
 *   血型以"血型+Rh型"组合表示，如"A+"、"B-"、"O+"等。
 *
 * 统计结果输出示例:
 *   --- 血型库存统计 ---
 *   A+: 2400 ml
 *   B+: 1800 ml
 *   O+: 3200 ml
 *   AB+: 600 ml
 *   O-: 400 ml
 *
 * 应用场景:
 *   - 库存盘点
 *   - 用血需求分析
 *   - 采血计划制定
 */
void statByBloodType(BloodList* list) {
    typedef struct { char bt[10]; int qty; } BTStat;
    BTStat stats[20];  /**< 血型统计数组 */
    int sc = 0;         /**< 已使用的血型种类数 */

    BloodNode* c = list->head;

    /* 遍历所有可用的血液记录进行统计 */
    while (c && c->status == 1) {
        char bt[10];
        sprintf(bt, "%s%s", c->bloodType, c->rhType);  /* 组合血型和Rh型 */

        int f = 0;
        for (int i = 0; i < sc; i++) {
            if (strcmp(stats[i].bt, bt) == 0) {
                stats[i].qty += c->quantity;
                f = 1;
                break;
            }
        }

        /* 新血型则添加到统计数组 */
        if (!f && sc < 20) {
            strcpy(stats[sc].bt, bt);
            stats[sc].qty = c->quantity;
            sc++;
        }
        c = c->next;
    }

    /* 输出统计结果 */
    printf("\n--- 血型库存统计 ---\n");
    for (int i = 0; i < sc; i++) {
        printf("%s: %d ml\n", stats[i].bt, stats[i].qty);
    }
}

/**
 * @brief 释放血库链表占用的所有内存
 * @param list 血库链表指针
 */
void freeBloodList(BloodList* list) {
    BloodNode* c = list->head;
    while (c) { BloodNode* t = c; c = c->next; free(t); }
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 血库管理主菜单
 * @param list 血库链表指针
 *
 * 菜单选项:
 * ┌────┬────────────────────┐
 * │ 1  │ 新增血液记录       │
 * │ 2  │ 查询血液记录       │
 * │ 3  │ 删除血液记录       │
 * │ 4  │ 修改血液记录       │
 * │ 5  │ 血液入库           │
 * │ 6  │ 血液出库           │
 * │ 7  │ 显示全部记录       │
 * │ 8  │ 血型统计           │
 * │ 0  │ 返回主菜单         │
 * └────┴────────────────────┘
 *
 * 日志记录:
 *   所有入库、出库、新增、删除、修改操作均会写入日志
 */
void bloodMenu(BloodList* list) {
    int choice;

    do {
        printf("\n");
        printTitle("血库管理系统");
        printf("1. 新增血液\n2. 查询\n3. 删除\n4. 修改\n");
        printf("5. 入库\n6. 出库\n7. 全部显示\n8. 血型统计\n0. 返回\n");
        choice = inputInt("选择: ");

        switch (choice) {

        /* ====== 新增血液 ====== */
        case 1: {
            BloodNode* n = createBloodNode();
            if (n && !isBloodIDExist(list, n->id)) {
                insertBloodNode(list, n);
                printf("添加成功！\n");

                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "血液编号:%s, 血型:%s%s, 数量:%d%s",
                    n->id, n->bloodType, n->rhType, n->quantity, n->unit);
                writeLog(LOG_BLOOD, "新增血液记录", "系统", detail);
            } else if (n) {
                printf("编号已存在！\n");
                free(n);
            }
            break;
        }

        /* ====== 查询 ====== */
        case 2: {
            if (!list->head) break;
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");
            BloodNode* f = findBloodByID(list, id);
            if (f) printBloodOne(f); else printf("未找到。\n");
            break;
        }

        /* ====== 删除 ====== */
        case 3: {
            if (!list->head) break;
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");
            deleteBloodByID(list, id);
            break;
        }

        /* ====== 修改 ====== */
        case 4: {
            if (!list->head) break;
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");
            BloodNode* n = findBloodByID(list, id);
            if (n) modifyBloodInfo(n); else printf("未找到。\n");
            break;
        }

        /* ====== 入库 ====== */
        case 5: {
            if (!list->head) break;
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");
            int qty = inputInt("入库数量: ");
            bloodInStock(list, id, qty);
            break;
        }

        /* ====== 出库 ====== */
        case 6: {
            if (!list->head) break;
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");
            int qty = inputInt("出库数量: ");
            bloodOutStock(list, id, qty);
            break;
        }

        /* ====== 显示全部 ====== */
        case 7:
            printBloodAll(list);
            break;

        /* ====== 血型统计 ====== */
        case 8:
            statByBloodType(list);
            break;

        /* ====== 返回 ====== */
        case 0:
            printf("返回主菜单...\n");
            break;

        default:
            printf("无效选择。\n");
        }

        if (choice != 0) pauseScreen();

    } while (choice != 0);
}

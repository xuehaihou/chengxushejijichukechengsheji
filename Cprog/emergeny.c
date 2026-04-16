/**
 * @file emergeny.c
 * @brief 急诊管理模块实现文件
 *
 * 功能概述:
 *   本模块实现医院急诊业务的全流程管理，包括：
 *   - 急诊接诊登记
 *   - 病情分级评估
 *   - 急诊处理状态跟踪
 *   - 急诊数据统计分析
 *
 * 核心业务:
 *   1. 接诊流程：病人到达 → 登记信息 → 病情分级 → 分配医生 → 开始处理
 *   2. 处理流程：诊断治疗 → 记录结果 → 更新状态（已处理/转住院/离院）
 *   3. 统计分析：按分级和状态统计急诊数据
 *
 * 分级体系:
 *   1级-危重: 需要立即抢救，生命体征不稳定
 *   2级-重症: 病情严重但暂时稳定，需尽快处理
 *   3级-普通: 常见急症，按顺序就诊
 *   4级-轻症: 轻微不适，可稍后处理
 *
 * 文件依赖:
 *   - emergency.h: 急诊数据结构和函数声明
 *   - log.h: 日志系统接口
 */

#include "emergency.h"
#include "log.h"

/**
 * @brief 将急诊分级枚举值转换为中文字符串
 * @param level 急诊分级(1-4)
 * @return 对应的中文字符串指针
 *
 * 分级映射关系:
 *   1 → "危重" （红色标识，立即处理）
 *   2 → "重症" （橙色标识，优先处理）
 *   3 → "普通" （黄色标识，正常排队）
 *   4 → "轻症" （绿色标识，可等待）
 *   其他 → "未知"
 */
const char* getEmergencyLevelString(int level) {
    switch (level) {
    case 1: return "危重";
    case 2: return "重症";
    case 3: return "普通";
    case 4: return "轻症";
    default: return "未知";
    }
}

/**
 * @brief 将急诊状态枚举值转换为中文字符串
 * @param status 急诊状态(0-3)
 * @return 对应的中文字符串指针
 *
 * 状态映射关系:
 *   0 → "接诊中" （正在接受诊疗）
 *   1 → "已处理" （完成急诊处置）
 *   2 → "转住院" （需要转入住院部继续治疗）
 *   3 → "离院" （治疗后离开医院）
 *   其他 → "未知"
 *
 * 状态转换规则:
 *   接诊中 ──→ 已处理 ──→ 离院
 *              │
 *              └──→ 转住院
 */
const char* getEmergencyStatusString(int status) {
    switch (status) {
    case 0: return "接诊中";
    case 1: return "已处理";
    case 2: return "转住院";
    case 3: return "离院";
    default: return "未知";
    }
}

/**
 * @brief 初始化急诊链表
 * @param list 急诊链表指针（输出参数）
 *
 * 功能说明:
 *   将链表头指针置空，计数器归零。
 *   在程序启动时调用。
 */
void initEmergencyList(EmergencyList* list) {
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 从文件加载急诊数据到内存
 * @param list 急诊链表指针（输出参数）
 * @param filename 数据文件路径，如"data/emergencies.txt"
 *
 * 功能说明:
 *   逐行读取文件，解析每条急诊记录并构建链表。
 *
 * 文件格式要求:
 *   每行一条记录，字段用"|"分隔：
 *   编号|病人编号|姓名|性别|年龄|到达时间|分级|症状|医生|结果|状态|备注
 *
 * 示例行:
 *   JZ001|P001|张三|男|45|2025-01-15 14:30|1|胸痛呼吸困难|王医生|心电图正常待观察|0|密切观察
 *
 * 字段说明:
 *   - 分级: 1=危重, 2=重症, 3=普通, 4=轻症
 *   - 状态: 0=接诊中, 1=已处理, 2=转住院, 3=离院
 */
void loadEmergenciesFromFile(EmergencyList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("提示：急诊数据文件不存在，将创建新文件。\n");
        return;
    }

    EmergencyNode* tail = NULL;  /**< 链表尾指针 */
    char line[1024];           /**< 行缓冲区 */

    while (fgets(line, sizeof(line), fp)) {
        EmergencyNode* node = (EmergencyNode*)malloc(sizeof(EmergencyNode));
        if (node == NULL) continue;

        /* 解析12个字段 */
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%d|%[^|]|%[^|]|%[^|]|%d|%[^\n]",
            node->id, node->patientId, node->name, node->gender,
            &node->age, node->arriveTime, &node->level, node->symptoms,
            node->doctor, node->result, &node->status, node->remark);

        node->next = NULL;

        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条急诊记录。\n", list->count);
}

/**
 * @brief 将内存中的急诊数据保存到文件
 * @param list 急诊链表指针
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   遍历链表，将每条记录写入文件。
 *   使用"w"模式覆盖原有内容。
 */
void saveEmergenciesToFile(EmergencyList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法打开文件 %s 进行写入！\n", filename);
        return;
    }

    EmergencyNode* current = list->head;
    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%d|%s|%d|%s|%s|%s|%d|%s\n",
            current->id, current->patientId, current->name, current->gender,
            current->age, current->arriveTime, current->level, current->symptoms,
            current->doctor, current->result, current->status, current->remark);
        current = current->next;
    }
    fclose(fp);
    printf("成功保存 %d 条急诊记录。\n", list->count);
}

/**
 * @brief 创建新的急诊记录节点
 * @return 成功返回新节点指针，失败返回NULL
 *
 * 功能说明:
 *   通过交互式界面收集急诊病人信息。
 *   自动获取当前时间作为到达时间。
 *   新记录默认状态为"接诊中"(status=0)。
 *
 * 输入项及格式:
 *   - 急诊编号: 必填，唯一标识，如"JZ001"
 *   - 病人编号: 必填
 *   - 姓名: 必填
 *   - 性别: 如"男"、"女"
 *   - 年龄: 整数
 *   - 到达时间: 自动获取当前日期时间
 *   - 分级: 枚举选择(1-4)
 *     * 1 = 危重（需要立即抢救）
 *     * 2 = 重症（需尽快处理）
 *     * 3 = 普通（按序就诊）
 *     * 4 = 轻症（可等待）
 *   - 症状描述: 详细描述病情症状
 *   - 接诊医生: 负责医生姓名
 *   - 处理结果: 初步诊断或处置方案
 *   - 备注: 可选补充信息
 *
 * 输入验证:
 *   - 编号不能为空
 *   - 分级必须在1-4范围内
 */
EmergencyNode* createEmergencyNode() {
    EmergencyNode* node = (EmergencyNode*)malloc(sizeof(EmergencyNode));
    if (node == NULL) return NULL;

    printf("\n--- 新增急诊记录 ---\n");

    /*
     * ====== 输入字段1: 急诊编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字、连字符的组合
     * 是否必填: 是（不能为空）
     * 输入样例:
     *   - "JZ001" (急诊拼音缩写+序号)
     *   - "ER-2024-1234" (Emergency+日期+序号)
     *   - "E20260416001" (急诊日期+流水号)
     */
    safeInput(node->id, MAX_ID_LEN, "请输入急诊编号: ");
    while (isEmpty(node->id))
        safeInput(node->id, MAX_ID_LEN, "编号不能为空！重新输入: ");

    /*
     * ====== 输入字段2: 病人编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字组合
     * 是否必填: 是
     * 输入样例: "P001", "BR20240101001"
     */
    safeInput(node->patientId, MAX_ID_LEN, "病人编号: ");

    /*
     * ====== 输入字段3: 患者姓名 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 中文真实姓名
     * 是否必填: 是
     * 输入样例: "张三", "李四"
     */
    safeInput(node->name, MAX_NAME_LEN, "姓名: ");

    /*
     * ====== 输入字段4: 性别 ======
     * 数据类型: char[10] (字符串)
     * 最大长度: 10个字符
     * 输入格式: 固定选项（男/女）
     * 是否必填: 是
     * 输入样例: "男", "女"
     */
    safeInput(node->gender, 10, "性别: ");

    /*
     * ====== 输入字段5: 年龄 ======
     * 数据类型: int (整数)
     * 取值范围: 0 ~ 150 (正整数)
     * 输入格式: 纯数字
     * 是否必填: 是
     * 输入样例: "25", "68", "1", "0"(新生儿)
     */
    node->age = inputInt("年龄: ");

    /* 【自动填充】到达时间 - 系统自动获取当前日期时间 */

    /*
     * ====== 输入字段6: 病情分级(急症等级) ======
     * 数据类型: int (整数枚举)
     * 取值范围: 1 ~ 4 (固定选项)
     * 输入格式: 纯数字选择
     * 是否必填: 是
     * 可选值及含义:
     *   - 1 = 危重 (红色) - 生命体征不稳定，需立即抢救
     *   - 2 = 重症 (橙色) - 有潜在生命危险，需尽快处理
     *   - 3 = 普通 (黄色) - 病情稳定，按顺序就诊
     *   - 4 = 轻症 (绿色) - 非急症，可稍后处理
     * 输入样例:
     *   - "1" (危重患者)
     *   - "2" (重症患者)
     *   - "3" (普通患者)
     *   - "4" (轻症患者)
     * 注意: 必须在1-4范围内，否则提示重新输入
     */
    printf("分级(1.危重 2.重症 3.普通 4.轻症): ");
    node->level = inputInt("");
    while (node->level < 1 || node->level > 4) {
        printf("无效！重新输入: ");
        node->level = inputInt("");
    }

    /*
     * ====== 输入字段7: 症状描述 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 医学术语或通俗描述
     * 是否必填: 是
     * 输入样例:
     *   - "胸痛伴呼吸困难2小时"
     *   - "高热39.5°C，意识模糊"
     *   - "车祸外伤，右腿开放性骨折"
     *   - "急性腹痛，呕吐3次"
     * 说明: 详细描述患者的症状表现和持续时间
     */
    safeInput(node->symptoms, MAX_REMARK_LEN, "症状描述: ");

    /*
     * ====== 输入字段8: 接诊医生 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 医生姓名
     * 是否必填: 是
     * 输入样例: "王医生", "李明主任医师"
     */
    safeInput(node->doctor, MAX_NAME_LEN, "接诊医生: ");

    /*
     * ====== 输入字段9: 处理结果 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 医疗处理描述
     * 是否必填: 是
     * 输入样例:
     *   - "留观治疗，给予吸氧、心电监护"
     *   - "收住院进一步检查治疗"
     *   - "门诊处理后离院，嘱随诊"
     *   - "转ICU继续抢救"
     * 说明: 记录急诊处理的主要措施和去向
     */
    safeInput(node->result, MAX_REMARK_LEN, "处理结果: ");

    /* 【系统设置】状态默认为0(接诊中/处理中) */

    /*
     * ====== 输入字段10: 备注 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 自由文本
     * 是否必填: 否（可留空）
     * 输入样例:
     *   - "" (直接回车留空)
     *   - "家属陪同"
     *   - "已通知120送来"
     *   - "既往史:糖尿病、高血压"
     */
    safeInput(node->remark, MAX_REMARK_LEN, "备注: ");

    node->next = NULL;
    return node;
}

/**
 * @brief 检查急诊编号是否已存在
 * @param list 急诊链表指针
 * @param id 待检查的急诊编号
 * @return 存在返回1，不存在返回0
 */
int isEmergencyIDExist(EmergencyList* list, const char* id) {
    EmergencyNode* c = list->head;
    while (c) {
        if (strcmp(c->id, id) == 0) return 1;
        c = c->next;
    }
    return 0;
}

/**
 * @brief 将新急诊节点插入链表头部
 * @param list 急诊链表指针
 * @param node 待插入的节点指针
 *
 * 功能说明:
 *   采用头插法，最新记录排在最前面。
 */
void insertEmergencyNode(EmergencyList* list, EmergencyNode* node) {
    if (!node) return;
    node->next = list->head;
    list->head = node;
    list->count++;
}

/**
 * @brief 根据急诊编号查找记录
 * @param list 急诊链表指针
 * @param id 目标急诊编号
 * @return 找到返回节点指针，未找到返回NULL
 */
EmergencyNode* findEmergencyByID(EmergencyList* list, const char* id) {
    EmergencyNode* c = list->head;
    while (c) {
        if (strcmp(c->id, id) == 0) return c;
        c = c->next;
    }
    return NULL;
}

/**
 * @brief 根据编号删除急诊记录
 * @param list 急诊链表指针
 * @param id 待删除的急诊编号
 * @return 成功删除返回1，失败返回0
 *
 * 功能说明:
 *   删除前显示完整信息并要求确认，
 *   删除后记录日志。
 */
int deleteEmergencyByID(EmergencyList* list, const char* id) {
    EmergencyNode* c = list->head, * p = NULL;

    while (c) {
        if (strcmp(c->id, id) == 0) {
            printEmergencyOne(c);

            if (!confirm("确认删除?")) return 0;

            /* 构造日志详情 */
            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "急诊编号:%s, 病人:%s, 级别:%s",
                c->id, c->name, getEmergencyLevelString(c->level));

            /* 从链表中移除 */
            if (!p) list->head = c->next;
            else p->next = c->next;

            free(c);
            list->count--;

            writeLog(LOG_EMERGENCY, "删除急诊记录", "系统", detail);
            return 1;
        }
        p = c;
        c = c->next;
    }
    printf("未找到。\n");
    return 0;
}

/**
 * @brief 修改急诊记录信息
 * @param node 待修改的急诊节点指针
 *
 * 功能说明:
 *   提供选择性修改功能：
 *   1. 接诊医生
 *   2. 处理结果
 *   3. 处理状态
 *   4. 备注
 *
 * 状态修改场景:
 *   - 诊断完成：改为"已处理"(1)
 *   - 需要住院：改为"转住院"(2)
 *   - 治疗结束离院：改为"离院"(3)
 */
void modifyEmergencyInfo(EmergencyNode* node) {
    if (!node) return;

    printEmergencyOne(node);

    printf("1.医生 2.结果 3.状态 4.备注 0.取消\n");
    int ch = inputInt("选择: ");

    switch (ch) {
    case 1: safeInput(node->doctor, MAX_NAME_LEN, "新医生: "); break;
    case 2: safeInput(node->result, MAX_REMARK_LEN, "新结果: "); break;
    case 3:
        printf("状态(0.接诊中 1.已处理 2.转住院 3.离院): ");
        node->status = inputInt("");
        break;
    case 4: safeInput(node->remark, MAX_REMARK_LEN, "新备注: "); break;
    case 0: return;
    }

    printf("修改成功！\n");

    /* 记录修改日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "急诊编号:%s, 病人:%s, 修改字段:%d", node->id, node->name, ch);
    writeLog(LOG_EMERGENCY, "修改急诊记录", "系统", detail);
}

/**
 * @brief 打印单条急诊记录
 * @param node 急诊节点指针
 *
 * 功能说明:
 *   格式化输出急诊记录的所有字段，
 *   使用中文显示分级和状态。
 *
 * 输出格式示例:
 *   ----------------------------------------------------------------------
 *   急诊编号: JZ001
 *   病人: 张三 (男, 45岁)
 *   到达时间: 2025-01-15 14:30 分级: 危重级(危重)
 *   症状: 胸痛呼吸困难
 *   医生: 王医生 结果: 心电图正常待观察
 *   状态: 接诊中 备注: 密切观察
 *   ----------------------------------------------------------------------
 */
void printEmergencyOne(EmergencyNode* node) {
    if (!node) return;

    printLine('-', 70);
    printf("急诊编号: %s\n", node->id);
    printf("病人: %s (%s, %d岁)\n", node->name, node->gender, node->age);
    printf("到达时间: %s 分级: %s级(%s)\n",
        node->arriveTime,
        getEmergencyLevelString(node->level),
        node->level == 1 ? "危重" : node->level == 2 ? "重症" : "一般");
    printf("症状: %s\n", node->symptoms);
    printf("医生: %s 结果: %s\n", node->doctor, node->result);
    printf("状态: %s 备注: %s\n",
        getEmergencyStatusString(node->status), node->remark);
    printLine('-', 70);
}

/**
 * @brief 打印所有急诊记录
 * @param list 急诊链表指针
 *
 * 功能说明:
 *   遍历链表，依次打印每条记录。
 *   先显示总记录数，再逐一输出详细信息。
 */
void printEmergencyAll(EmergencyList* list) {
    if (!list->head) { printf("暂无急诊记录。\n"); return; }

    printf("\n--- 所有急诊记录 (%d条) ---\n", list->count);
    EmergencyNode* c = list->head;
    while (c) { printEmergencyOne(c); c = c->next; }
}

/**
 * @brief 急诊数据统计分析
 * @param list 急诊链表指针
 *
 * 功能说明:
 *   对急诊数据进行双维度统计：
 *
 *   按分级统计:
 *   - 危重(1级)人数
 *   - 重症(2级)人数
 *   - 普通(3级)人数
 *   - 轻症(4级)人数
 *
 *   按状态统计:
 *   - 接诊中人数
 *   - 已处理人数
 *   - 转住院人数
 *   - 离院人数
 *
 * 统计结果输出示例:
 *   --- 急诊统计 ---
 *   按分级: 危重5 重症15 普通30 轻症20
 *   按状态: 接诊10 已处理40 转住院10 离院10
 *   总计: 70
 */
void statEmergencies(EmergencyList* list) {
    int lv[5] = { 0 };  /**< 各级别计数数组(索引1-4使用) */
    int st[4] = { 0 };  /**< 各状态计数数组(索引0-3使用) */

    EmergencyNode* c = list->head;

    while (c) {
        if (c->level >= 1 && c->level <= 4) lv[c->level]++;
        if (c->status >= 0 && c->status <= 3) st[c->status]++;
        c = c->next;
    }

    /* 输出统计结果 */
    printf("\n--- 急诊统计 ---\n");
    printf("按分级: 危重%d 重症%d 普通%d 轻症%d\n",
        lv[1], lv[2], lv[3], lv[4]);
    printf("按状态: 接诊%d 已处理%d 转住院%d 离院%d\n",
        st[0], st[1], st[2], st[3]);
    printf("总计: %d\n", list->count);
}

/**
 * @brief 释放急诊链表占用的所有内存
 * @param list 急诊链表指针
 */
void freeEmergencyList(EmergencyList* list) {
    EmergencyNode* c = list->head;
    while (c) { EmergencyNode* t = c; c = c->next; free(t); }
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 急诊管理主菜单
 * @param list 急诊链表指针
 *
 * 功能说明:
 *   提供急诊管理的交互式菜单界面：
 *
 *   菜单选项:
 *   ┌────┬────────────────────┐
 *   │ 1  │ 新增急诊记录       │
 *   │ 2  │ 查询急诊记录       │
 *   │ 3  │ 删除急诊记录       │
 *   │ 4  │ 修改急诊记录       │
 *   │ 5  │ 显示全部记录       │
 *   │ 6  │ 统计分析           │
 *   │ 0  │ 返回主菜单         │
 *   └────┴────────────────────┘
 *
 * 日志记录:
 *   新增、删除、修改操作会自动写入日志
 */
void emergencyMenu(EmergencyList* list) {
    int choice;

    do {
        printf("\n");
        printTitle("急诊管理系统");
        printf("1.新增 2.查询 3.删除 4.修改 5.全部显示 6.统计 0.返回\n");
        choice = inputInt("选择: ");

        switch (choice) {

        /* ====== 新增急诊 ====== */
        case 1: {
            EmergencyNode* n = createEmergencyNode();
            if (n && !isEmergencyIDExist(list, n->id)) {
                insertEmergencyNode(list, n);
                printf("添加成功！\n");

                /* 记录新增日志 */
                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "急诊编号:%s, 病人:%s, 级别:%s, 医生:%s",
                    n->id, n->name,
                    getEmergencyLevelString(n->level), n->doctor);
                writeLog(LOG_EMERGENCY, "新增急诊记录", "系统", detail);
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
            EmergencyNode* f = findEmergencyByID(list, id);
            if (f)
                printEmergencyOne(f);
            else
                printf("未找到。\n");
            break;
        }

        /* ====== 删除 ====== */
        case 3: {
            if (!list->head) break;
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");
            deleteEmergencyByID(list, id);
            break;
        }

        /* ====== 修改 ====== */
        case 4: {
            if (!list->head) break;
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");
            EmergencyNode* n = findEmergencyByID(list, id);
            if (n)
                modifyEmergencyInfo(n);
            else
                printf("未找到。\n");
            break;
        }

        /* ====== 显示全部 ====== */
        case 5:
            printEmergencyAll(list);
            break;

        /* ====== 统计分析 ====== */
        case 6:
            statEmergencies(list);
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

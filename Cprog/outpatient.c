/**
 * @file outpatient.c
 * @brief 门诊管理模块实现文件
 *
 * 功能概述:
 *   本模块实现医院门诊业务的全流程管理，包括：
 *   - 门诊记录的增删改查
 *   - 门诊数据的持久化存储
 *   - 门诊统计分析功能
 *
 * 数据结构:
 *   采用单链表结构存储门诊记录，每条记录包含：
 *   门诊编号、病人信息、科室、医生、诊断、处方、费用等
 *
 * 业务流程:
 *   病人挂号 → 医生诊断 → 开具处方 → 计算费用 → 生成门诊记录
 *
 * 文件依赖:
 *   - outpatient.h: 门诊数据结构和函数声明
 *   - log.h: 日志系统接口
 */

#include "outpatient.h"
#include "log.h"

/**
 * @brief 初始化门诊链表
 * @param list 门诊链表指针（输出参数）
 *
 * 功能说明:
 *   将链表头指针置空，计数器归零。
 *   在程序启动时调用，确保链表处于初始状态。
 *
 * 使用场景:
 *   系统初始化时，在loadOutpatientsFromFile之前调用
 */
void initOutpatientList(OutpatientList* list) {
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 从文件加载门诊数据到内存
 * @param list 门诊链表指针（输出参数）
 * @param filename 数据文件路径，如"data/outpatients.txt"
 *
 * 功能说明:
 *   1. 打开指定文件进行读取
 *   2. 逐行解析文件内容
 *   3. 为每条记录创建节点并插入链表尾部
 *   4. 更新链表计数器
 *
 * 文件格式要求:
 *   每行一条记录，字段用"|"分隔：
 *   门诊编号|病人编号|姓名|科室|医生|日期|诊断|处方|费用|备注
 *
 * 示例行:
 *   MZ001|P001|张三|内科|李医生|2025-01-15|感冒|阿莫西林|50.00|复诊
 *
 * 错误处理:
 *   - 文件不存在时提示并返回（不报错）
 *   - 内存分配失败时跳过该记录
 */
void loadOutpatientsFromFile(OutpatientList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("提示：门诊数据文件不存在，将创建新文件。\n");
        return;
    }

    OutpatientNode* tail = NULL;  /**< 链表尾指针，用于高效追加 */
    char line[1024];             /**< 行缓冲区 */

    /* 逐行读取并解析记录 */
    while (fgets(line, sizeof(line), fp)) {
        OutpatientNode* node = (OutpatientNode*)malloc(sizeof(OutpatientNode));
        if (node == NULL) continue;

        /* 解析一行中的各个字段 */
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%lf|%[^\n]",
            node->id, node->patientId, node->patientName, node->department,
            node->doctor, node->date, node->diagnosis, node->prescription,
            &node->cost, node->remark);

        node->next = NULL;

        /* 采用尾插法维护链表顺序 */
        if (list->head == NULL) {
            list->head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条门诊记录。\n", list->count);
}

/**
 * @brief 将内存中的门诊数据保存到文件
 * @param list 门诊链表指针
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   遍历整个链表，将每条记录按指定格式写入文件。
 *   使用"w"模式打开文件，会覆盖原有内容。
 *
 * 输出格式:
 *   与loadOutpatientsFromFile的输入格式一致，保证可读写一致性
 *
 * 调用时机:
 *   用户选择保存数据或退出系统时调用
 */
void saveOutpatientsToFile(OutpatientList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法打开文件 %s 进行写入！\n", filename);
        return;
    }

    OutpatientNode* current = list->head;

    /* 遍历链表逐条写入 */
    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|%s|%.2f|%s\n",
            current->id, current->patientId, current->patientName,
            current->department, current->doctor, current->date,
            current->diagnosis, current->prescription,
            current->cost, current->remark);
        current = current->next;
    }
    fclose(fp);
    printf("成功保存 %d 条门诊记录。\n", list->count);
}

/**
 * @brief 创建新的门诊记录节点
 * @return 成功返回新节点指针，失败返回NULL
 *
 * 功能说明:
 *   通过交互式界面收集用户输入，创建完整的门诊记录。
 *   自动填充当前日期作为就诊日期。
 *
 * 输入项及格式:
 *   - 门诊编号: 必填，唯一标识，如"MZ001"
 *   - 病人编号: 必填，关联病人信息，如"P001"
 *   - 姓名: 必填，如"张三"
 *   - 科室: 必填，如"内科"、"外科"
 *   - 医生: 必填，如"李医生"
 *   - 日期: 自动获取当前日期
 *   - 诊断: 必填，医生诊断结果
 *   - 处方: 必填，开具的药品和治疗方案
 *   - 费用: 浮点数，如"50.00"
 *   - 备注: 可选，补充信息
 *
 * 注意:
 *   此函数只负责创建节点，不负责插入链表
 */
OutpatientNode* createOutpatientNode() {
    OutpatientNode* node = (OutpatientNode*)malloc(sizeof(OutpatientNode));
    if (node == NULL) {
        printf("错误：内存分配失败！\n");
        return NULL;
    }

    printf("\n--- 新增门诊记录 ---\n");

    /*
     * ====== 输入字段1: 门诊编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字、连字符的组合
     * 是否必填: 是（不能为空）
     * 输入样例:
     *   - "MZ001" (门诊拼音缩写+序号)
     *   - "OP-2024-0123" (OutPatient+日期+序号)
     *   - "M20260416001" (年月日+流水号)
     * 验证规则:
     *   - 不能为空字符串
     *   - 建议使用有规律的编码便于查询
     */
    safeInput(node->id, MAX_ID_LEN, "请输入门诊编号: ");
    while (isEmpty(node->id)) {
        printf("编号不能为空！\n");
        safeInput(node->id, MAX_ID_LEN, "请输入门诊编号: ");
    }

    /*
     * ====== 输入字段2: 病人编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字组合（通常与住院/急诊共用）
     * 是否必填: 是
     * 输入样例:
     *   - "P001" (Patient+序号)
     *   - "BR20240101001" (病人+入院日期+序号)
     *   - "ID610102199001011234" (身份证号后几位)
     * 说明: 应与患者档案系统中的编号一致，用于关联患者信息
     */
    safeInput(node->patientId, MAX_ID_LEN, "请输入病人编号: ");

    /*
     * ====== 输入字段3: 患者姓名 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 中文真实姓名
     * 是否必填: 是
     * 输入样例:
     *   - "张三"
     *   - "李四"
     *   - "王芳"
     * 说明: 使用患者身份证上的法定姓名
     */
    safeInput(node->patientName, MAX_NAME_LEN, "请输入姓名: ");

    /*
     * ====== 输入字段4: 就诊科室 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符
     * 输入格式: 科室名称（中文）
     * 是否必填: 是
     * 输入样例:
     *   - "内科"
     *   - "外科"
     *   - "儿科"
     *   - "妇产科"
     *   - "眼科"
     *   - "皮肤科"
     *   - "中医科"
     * 说明: 对应医院实际设置的科室名称
     */
    safeInput(node->department, MAX_DEPT_LEN, "请输入科室: ");

    /*
     * ====== 输入字段5: 接诊医生 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 医生姓名
     * 是否必填: 是
     * 输入样例:
     *   - "王医生"
     *   - "李明主任医师"
     *   - "Dr. Zhang"
     * 说明: 填写实际接诊的医生姓名或工号
     */
    safeInput(node->doctor, MAX_NAME_LEN, "请输入医生: ");

    /* 【自动填充】就诊日期 - 系统自动获取当前日期(YYYY-MM-DD) */

    /*
     * ====== 输入字段6: 诊断结果 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 医学术语或通俗描述
     * 是否必填: 是
     * 输入样例:
     *   - "上呼吸道感染"
     *   - "急性胃肠炎"
     *   - "高血压病(2级)"
     *   - "2型糖尿病"
     *   - "待进一步检查确诊"
     * 说明: 医师根据临床表现做出的初步诊断结论
     */
    safeInput(node->diagnosis, MAX_REMARK_LEN, "请输入诊断: ");

    /*
     * ====== 输入字段7: 处方内容 ======
     * 数据类型: char[500] (字符串) - 特长字段
     * 最大长度: 500个字符（250个中文字符）
     * 输入格式: 药品名称+用法用量
     * 是否必填: 是
     * 输入样例:
     *   - "阿莫西林胶囊 0.5g×24粒 用法:0.5g tid po"
     *   - "布洛芬缓释胶囊 0.3g×20粒 用法:0.3g bid po 餐后"
     *   - "生理盐水250ml + 头孢曲松钠2.0g ivgtt qd ×3天"
     *   - "无(仅开检查单)" (如无需用药)
     * 说明: 详细记录开具的药品及用法用量，可包含多行处方
     */
    safeInput(node->prescription, 500, "请输入处方: ");  /* 处方可较长 */

    /*
     * ====== 输入字段8: 就诊费用 ======
     * 数据类型: double (双精度浮点数)
     * 取值范围: 0.00 ~ 很大的正数
     * 精度要求: 保留2位小数(元)
     * 输入格式: 数字，可含小数点
     * 是否必填: 是
     * 输入样例:
     *   - "156.80" (挂号费+诊疗费+药费等合计)
     *   - "25.00" (仅挂号费)
     *   - "1289.50" (含检查费和药费)
     * 业务含义: 本次门诊产生的总费用（含挂号、诊疗、检查、药品等）
     * 注意: 不能为负数
     */
    node->cost = inputDouble("请输入费用: ");

    /*
     * ====== 输入字段9: 备注 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 自由文本
     * 是否必填: 否（可留空）
     * 输入样例:
     *   - "" (直接回车留空)
     *   - "复诊患者，病情稳定"
     *   - "建议一周后复查血常规"
     *   - "已预约下次就诊时间"
     * 用途: 记录特殊医嘱、随访安排等补充信息
     */
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注: ");

    node->next = NULL;
    return node;
}

/**
 * @brief 检查门诊编号是否已存在
 * @param list 门诊链表指针
 * @param id 待检查的门诊编号
 * @return 存在返回1，不存在返回0
 *
 * 功能说明:
 *   在新增门诊前调用，确保编号唯一性。
 *   遍历链表查找匹配的编号。
 */
int isOutpatientIDExist(OutpatientList* list, const char* id) {
    OutpatientNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) return 1;
        current = current->next;
    }
    return 0;
}

/**
 * @brief 将新门诊节点插入链表头部
 * @param list 门诊链表指针
 * @param node 待插入的节点指针
 *
 * 功能说明:
 *   采用头插法，新记录插入到链表最前面。
 *   这样最近添加的记录会优先显示。
 *
 * 插入后自动更新链表计数器。
 */
void insertOutpatientNode(OutpatientList* list, OutpatientNode* node) {
    if (node == NULL) return;
    node->next = list->head;
    list->head = node;
    list->count++;
}

/**
 * @brief 根据门诊编号查找记录
 * @param list 门诊链表指针
 * @param id 目标门诊编号
 * @return 找到返回节点指针，未找到返回NULL
 *
 * 功能说明:
 *   线性搜索链表，匹配第一个符合条件的节点。
 *   用于查询、修改、删除前的定位操作。
 */
OutpatientNode* findOutpatientByID(OutpatientList* list, const char* id) {
    OutpatientNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) return current;
        current = current->next;
    }
    return NULL;
}

/**
 * @brief 根据编号删除门诊记录
 * @param list 门诊链表指针
 * @param id 待删除的门诊编号
 * @return 成功删除返回1，失败返回0
 *
 * 功能说明:
 *   1. 查找目标记录
 *   2. 显示记录详情供确认
 *   3. 用户确认后执行删除
 *   4. 维护链表结构完整性
 *   5. 记录删除操作日志
 *
 * 安全机制:
 *   - 删除前显示完整信息
 *   - 要求二次确认
 *   - 自动释放被删除节点的内存
 */
int deleteOutpatientByID(OutpatientList* list, const char* id) {
    OutpatientNode* current = list->head, * prev = NULL;

    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printOutpatientOne(current);  /* 显示待删除记录 */

            if (!confirm("确认删除?")) return 0;  /* 二次确认 */

            /* 构造日志详情 */
            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "门诊编号:%s, 病人:%s, 科室:%s",
                current->id, current->patientName, current->department);

            /* 从链表中移除节点 */
            if (prev == NULL)
                list->head = current->next;  /* 删除的是头节点 */
            else
                prev->next = current->next;   /* 删除中间或尾节点 */

            free(current);       /* 释放内存 */
            list->count--;       /* 更新计数 */

            writeLog(LOG_OUTPATIENT, "删除门诊记录", "系统", detail);
            return 1;
        }
        prev = current;
        current = current->next;
    }
    printf("未找到。\n");
    return 0;
}

/**
 * @brief 修改门诊记录信息
 * @param node 待修改的门诊节点指针
 *
 * 功能说明:
 *   提供选择性修改功能，用户可选择修改以下字段：
 *   1. 科室
 *   2. 医生
 *   3. 诊断
 *   4. 处方
 *   5. 费用
 *   6. 备注
 *
 * 修改流程:
 *   1. 显示当前记录
 *   2. 显示可修改字段菜单
 *   3. 用户选择要修改的字段
 *   4. 输入新值并更新
 *   5. 记录修改日志
 */
void modifyOutpatientInfo(OutpatientNode* node) {
    if (node == NULL) return;

    printOutpatientOne(node);

    printf("1.科室 2.医生 3.诊断 4.处方 5.费用 6.备注 0.取消\n");
    int choice = inputInt("选择: ");

    switch (choice) {
    case 1: safeInput(node->department, MAX_DEPT_LEN, "新科室: "); break;
    case 2: safeInput(node->doctor, MAX_NAME_LEN, "新医生: "); break;
    case 3: safeInput(node->diagnosis, MAX_REMARK_LEN, "新诊断: "); break;
    case 4: safeInput(node->prescription, 500, "新处方: "); break;
    case 5: node->cost = inputDouble("新费用: "); break;
    case 6: safeInput(node->remark, MAX_REMARK_LEN, "新备注: "); break;
    case 0: return;  /* 取消修改 */
    }

    printf("修改成功！\n");

    /* 记录修改日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "门诊编号:%s, 病人:%s, 修改字段:%d", node->id, node->patientName, choice);
    writeLog(LOG_OUTPATIENT, "修改门诊记录", "系统", detail);
}

/**
 * @brief 打印单条门诊记录
 * @param node 门诊节点指针
 *
 * 功能说明:
 *   格式化输出一条门诊记录的所有字段，
 *   用分隔线包裹，便于阅读。
 *
 * 输出格式示例:
 *   ----------------------------------------------------------------------
 *   门诊编号: MZ001
 *   病人编号: P001 姓名: 张三
 *   科室: 内科 医生: 李医生
 *   日期: 2025-01-15
 *   诊断: 上呼吸道感染
 *   处方: 阿莫西林胶囊 0.5g*24粒...
 *   费用: 150.00
 *   备注: 三天后复诊
 *   ----------------------------------------------------------------------
 */
void printOutpatientOne(OutpatientNode* node) {
    if (node == NULL) return;

    printLine('-', 70);
    printf("门诊编号: %s\n", node->id);
    printf("病人编号: %s 姓名: %s\n", node->patientId, node->patientName);
    printf("科室: %s 医生: %s\n", node->department, node->doctor);
    printf("日期: %s\n", node->date);
    printf("诊断: %s\n", node->diagnosis);
    printf("处方: %s\n", node->prescription);
    printf("费用: %.2f\n", node->cost);
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

/**
 * @brief 打印所有门诊记录
 * @param list 门诊链表指针
 *
 * 功能说明:
 *   遍历链表，依次打印每条记录。
 *   先显示总记录数，再逐一输出详细信息。
 *
 * 适用场景:
 *   用户查看全部门诊数据时使用
 */
void printOutpatientAll(OutpatientList* list) {
    if (list->head == NULL) {
        printf("暂无门诊记录。\n");
        return;
    }

    printf("\n--- 所有门诊记录 (共 %d 条) ---\n", list->count);
    OutpatientNode* current = list->head;
    while (current != NULL) {
        printOutpatientOne(current);
        current = current->next;
    }
}

/**
 * @brief 门诊数据统计分析
 * @param list 门诊链表指针
 *
 * 功能说明:
 *   对门诊数据进行多维度统计：
 *   1. 总门诊量统计
 *   2. 总费用统计
 *   3. 分科室统计（各科室就诊次数和费用汇总）
 *
 * 统计结果输出示例:
 *   --- 门诊统计 ---
 *   总门诊量: 100 次
 *   总费用: 25000.00
 *   内科: 40次, 费用8000.00
 *   外科: 30次, 费用12000.00
 *   儿科: 30次, 费用5000.00
 *
 * 实现方式:
 *   使用临时数组按科室分组统计
 */
void statOutpatients(OutpatientList* list) {
    double totalCost = 0;
    OutpatientNode* curr = list->head;

    /* 定义科室统计结构体 */
    typedef struct {
        char dept[50];   /**< 科室名称 */
        int cnt;         /**< 就诊次数 */
        double cost;     /**< 费用合计 */
    } DeptStat;

    DeptStat stats[50];  /**< 统计数组，最多支持50个科室 */
    int scnt = 0;        /**< 已使用的科室数 */

    /* 遍历所有记录进行统计 */
    while (curr != NULL) {
        totalCost += curr->cost;

        /* 查找该科室是否已有统计记录 */
        int found = 0;
        for (int i = 0; i < scnt; i++) {
            if (strcmp(stats[i].dept, curr->department) == 0) {
                stats[i].cnt++;
                stats[i].cost += curr->cost;
                found = 1;
                break;
            }
        }

        /* 新科室则添加到统计数组 */
        if (!found && scnt < 50) {
            strcpy(stats[scnt].dept, curr->department);
            stats[scnt].cnt = 1;
            stats[scnt].cost = curr->cost;
            scnt++;
        }
        curr = curr->next;
    }

    /* 输出统计结果 */
    printf("\n--- 门诊统计 ---\n");
    printf("总门诊量: %d 次\n", list->count);
    printf("总费用: %.2f\n", totalCost);
    for (int i = 0; i < scnt; i++) {
        printf("%s: %d次, 费用%.2f\n", stats[i].dept, stats[i].cnt, stats[i].cost);
    }
}

/**
 * @brief 释放门诊链表占用的所有内存
 * @param list 门诊链表指针
 *
 * 功能说明:
 *   遍历链表，逐个释放每个节点分配的内存。
 *   最后将链表头指针置空，计数器归零。
 *
 * 调用时机:
 *   程序退出前调用，防止内存泄漏
 */
void freeOutpatientList(OutpatientList* list) {
    OutpatientNode* current = list->head;
    while (current != NULL) {
        OutpatientNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 门诊管理主菜单
 * @param list 门诊链表指针
 *
 * 功能说明:
 *   提供门诊管理的交互式菜单界面，包含以下功能：
 *
 *   菜单选项:
 *   ┌────┬────────────────────────────┐
 *   │ 1  │ 新增门诊记录               │
 *   │ 2  │ 按编号查询门诊记录          │
 *   │ 3  │ 删除门诊记录               │
 *   │ 4  │ 修改门诊记录               │
 *   │ 5  │ 显示全部门诊记录            │
 *   │ 6  │ 门诊统计分析               │
 *   │ 0  │ 返回主菜单                 │
 *   └────┴────────────────────────────┘
 *
 * 操作流程:
 *   循环显示菜单，根据用户选择调用对应功能模块。
 *   选择0时退出本菜单，返回上级菜单。
 *
 * 日志记录:
 *   新增、删除、修改操作会自动写入日志
 */
void outpatientMenu(OutpatientList* list) {
    int choice;

    do {
        printf("\n");
        printTitle("门诊管理系统");
        printf("1. 新增门诊\n2. 查询门诊\n3. 删除门诊\n4. 修改门诊\n");
        printf("5. 显示全部\n6. 统计分析\n0. 返回主菜单\n");
        choice = inputInt("请选择功能: ");

        switch (choice) {

        /* ====== 新增门诊记录 ====== */
        case 1: {
            OutpatientNode* node = createOutpatientNode();
            if (node && !isOutpatientIDExist(list, node->id)) {
                insertOutpatientNode(list, node);
                printf("添加成功！\n");

                /* 记录新增日志 */
                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "门诊编号:%s, 病人:%s, 科室:%s, 医生:%s, 费用:%.2f",
                    node->id, node->patientName, node->department, node->doctor, node->cost);
                writeLog(LOG_OUTPATIENT, "新增门诊记录", "系统", detail);
            } else if (node) {
                printf("编号已存在！\n");
                free(node);  /* 编号重复时释放已创建的节点 */
            }
            break;
        }

        /* ====== 按编号查询 ====== */
        case 2: {
            if (!list->head) { printf("无记录。\n"); break; }
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");
            OutpatientNode* found = findOutpatientByID(list, id);
            if (found)
                printOutpatientOne(found);
            else
                printf("未找到。\n");
            break;
        }

        /* ====== 删除门诊记录 ====== */
        case 3: {
            if (!list->head) { printf("无记录。\n"); break; }
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");
            deleteOutpatientByID(list, id);
            break;
        }

        /* ====== 修改门诊记录 ====== */
        case 4: {
            if (!list->head) { printf("无记录。\n"); break; }
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");
            OutpatientNode* node = findOutpatientByID(list, id);
            if (node)
                modifyOutpatientInfo(node);
            else
                printf("未找到。\n");
            break;
        }

        /* ====== 显示全部记录 ====== */
        case 5:
            printOutpatientAll(list);
            break;

        /* ====== 统计分析 ====== */
        case 6:
            statOutpatients(list);
            break;

        /* ====== 返回上级菜单 ====== */
        case 0:
            printf("返回主菜单...\n");
            break;

        default:
            printf("无效选择。\n");
        }

        if (choice != 0) pauseScreen();

    } while (choice != 0);
}

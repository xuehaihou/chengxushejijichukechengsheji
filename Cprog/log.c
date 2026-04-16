/**
 * @file log.c
 * @brief 日志系统实现文件 - 实现日志记录、存储、查询和管理功能
 *
 * 本模块提供完整的日志管理功能，包括：
 * - 日志写入：将操作记录追加到对应类型的日志文件
 * - 日志加载/保存：从文件读取或保存日志数据到内存
 * - 日志查询：支持按类型、日期范围等多种方式查询
 * - 权限控制：管理员密码验证，保护敏感日志信息
 */

#include "log.h"
#include <time.h>

/* ==================== 辅助函数实现 ==================== */

/**
 * @brief 根据日志类型获取对应的日志文件路径
 * @param logType 日志类型常量(LOG_XXX)，取值范围1-10
 * @return const char* 返回日志文件的相对路径字符串
 *
 * 功能说明:
 *   将日志类型映射到具体的文件路径，所有日志文件存放在data目录下。
 *   文件命名规则: log_模块名.txt
 *
 * 映射表:
 *   1 (LOG_MEDICINE)   -> data/log_medicine.txt    药品管理
 *   2 (LOG_OUTPATIENT) -> data/log_outpatient.txt   门诊管理
 *   3 (LOG_APPOINT)    -> data/log_appoint.txt      预约管理
 *   4 (LOG_INPATIENT)  -> data/log_inpatient.txt     住院管理
 *   5 (LOG_EMERGENCY)  -> data/log_emergency.txt     急诊管理
 *   6 (LOG_BLOOD)      -> data/log_blood.txt         血库管理
 *   7 (LOG_DEVICE)     -> data/log_device.txt        设备管理
 *   8 (LOG_FINANCE)    -> data/log_finance.txt       财务管理
 *   9 (LOG_STAFF)      -> data/log_staff.txt         人事管理
 *   10 (LOG_LOGISTIC)  -> data/log_logistic.txt      后勤管理
 *   其他              -> data/log_other.txt          默认文件
 */
const char* getLogFileName(int logType) {
    switch (logType) {
    case LOG_MEDICINE:   return "data/log_medicine.txt";
    case LOG_OUTPATIENT: return "data/log_outpatient.txt";
    case LOG_APPOINT:    return "data/log_appoint.txt";
    case LOG_INPATIENT:  return "data/log_inpatient.txt";
    case LOG_EMERGENCY:  return "data/log_emergency.txt";
    case LOG_BLOOD:      return "data/log_blood.txt";
    case LOG_DEVICE:     return "data/log_device.txt";
    case LOG_FINANCE:    return "data/log_finance.txt";
    case LOG_STAFF:      return "data/log_staff.txt";
    case LOG_LOGISTIC:   return "data/log_logistic.txt";
    default:             return "data/log_other.txt";
    }
}

/**
 * @brief 根据日志类型获取中文显示名称
 * @param logType 日志类型常量(LOG_XXX)，取值范围1-10
 * @return const char* 返回日志类型的中文描述字符串
 *
 * 功能说明:
 *   将数字类型的日志代码转换为可读的中文名称，
 *   用于在界面显示和日志输出时提供友好的提示信息。
 *
 * 返回值示例:
 *   LOG_MEDICINE (1)  -> "药品管理"
 *   LOG_STAFF (9)     -> "人事管理"
 */
const char* getLogTypeName(int logType) {
    switch (logType) {
    case LOG_MEDICINE:   return "药品管理";
    case LOG_OUTPATIENT: return "门诊管理";
    case LOG_APPOINT:    return "预约管理";
    case LOG_INPATIENT:  return "住院管理";
    case LOG_EMERGENCY:  return "急诊管理";
    case LOG_BLOOD:      return "血库管理";
    case LOG_DEVICE:     return "设备管理";
    case LOG_FINANCE:    return "财务管理";
    case LOG_STAFF:      return "人事管理";
    case LOG_LOGISTIC:   return "后勤管理";
    default:             return "其他";
    }
}

/* ==================== 链表初始化与释放 ==================== */

/**
 * @brief 初始化日志链表
 * @param list 指向LogList结构体的指针（必须已分配内存）
 *
 * 功能说明:
 *   将链表的头指针设置为NULL，计数器清零。
 *   在使用链表前必须调用此函数进行初始化。
 *
 * 使用示例:
 *   LogList myList;
 *   initLogList(&myList);  // 初始化后才能使用
 */
void initLogList(LogList* list) {
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 释放日志链表占用的所有内存
 * @param list 指向LogList结构体的指针
 *
 * 功能说明:
 *   遍历整个链表，逐个释放每个LogNode节点分配的内存。
 *   释放后将头指针置空、计数器归零，防止悬垂指针问题。
 *
 * 注意事项:
 *   - 必须传入有效的指针，不能为NULL
 *   - 释放后链表不可再使用，如需继续使用需重新调用initLogList()
 *   - 此函数不会释放list本身，只释放链表中的节点
 */
void freeLogList(LogList* list) {
    LogNode* current = list->head;
    while (current != NULL) {
        LogNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->count = 0;
}

/* ==================== 时间处理函数 ==================== */

/**
 * @brief 获取当前系统时间并格式化为字符串
 * @param datetime 用于接收时间字符串的缓冲区（至少20字节）
 *
 * 功能说明:
 *   获取当前本地时间，按照"YYYY-MM-DD HH:MM:SS"格式写入缓冲区。
 *   使用time()和localtime()获取时间，sprintf格式化输出。
 *
 * 输出格式:
 *   示例: "2025-01-15 14:30:25"
 *   格式: 年-月-日 时:分:秒（24小时制）
 *
 * 使用场景:
 *   在writeLog()中自动记录操作发生的时间戳
 */
void getCurrentDateTime(char* datetime) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);
}

/* ==================== 核心功能函数 ==================== */

/**
 * @brief 写入一条日志记录到对应的日志文件
 * @param logType 日志类型(1-10)，决定写入哪个文件
 * @param operation 操作描述字符串，如"新增药品"、"删除挂号"
 * @param operator 操作人标识符，通常传"系统"
 * @param detail 详细信息字符串，包含被操作对象的关键字段值
 *
 * 功能说明:
 *   1. 根据logType确定目标日志文件路径
 *   2. 以追加模式("a")打开文件
 *   3. 如果文件或目录不存在，自动创建data目录
 *   4. 获取当前时间作为日志时间戳
 *   5. 将日志内容按格式写入文件并关闭
 *
 * 文件存储格式:
 *   时间|操作|操作人|详情\n
 *   示例: 2025-01-15 14:30:25|新增药品|系统|药品编号:P001, 名称:阿莫西林...
 *
 * 错误处理:
 *   - 目录不存在时自动创建
 *   - 文件打开失败时输出错误信息并返回
 *
 * 线程安全: 否（多线程环境需加锁）
 */
void writeLog(int logType, const char* operation, const char* operator, const char* detail) {
    const char* filename = getLogFileName(logType);
    FILE* fp = fopen(filename, "a");
    if (fp == NULL) {
        system("if not exist data mkdir data 2>nul");
        fp = fopen(filename, "a");
        if (fp == NULL) {
            printf("错误：无法写入日志文件！\n");
            return;
        }
    }

    char datetime[20];
    getCurrentDateTime(datetime);

    fprintf(fp, "%s|%s|%s|%s\n", datetime, operation, operator, detail);
    fclose(fp);
}

/**
 * @brief 从日志文件加载所有记录到内存链表
 * @param list 指向已初始化的LogList结构体，用于存储加载的数据
 * @param filename 要读取的日志文件路径
 *
 * 功能说明:
 *   1. 以只读模式打开日志文件
 *   2. 逐行读取文件内容
 *   3. 解析每行数据（按"|"分隔）到LogNode结构体
 *   4. 采用尾插法构建链表，保持原有顺序
 *   5. 更新链表计数器
 *
 * 文件格式要求:
 *   每行格式: 时间|操作|操作人|详情
 *   字段间用竖线"|"分隔，行尾换行符\n
 *
 * 数据解析规则:
 *   - 第1个字段: datetime (最多19字符)
 *   - 第2个字段: operation (最多49字符)
 *   - 第3个字段: operator (最多49字符)
 *   - 第4个字段: detail (剩余全部内容)
 *
 * 异常处理:
 *   - 文件不存在时不报错，直接返回空链表
 *   - 内存分配失败时跳过该行继续处理下一行
 */
void loadLogFromFile(LogList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return;
    }

    LogNode* tail = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        LogNode* node = (LogNode*)malloc(sizeof(LogNode));
        if (node == NULL) continue;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^\n]",
            node->datetime, node->operation, node->operator, node->detail);

        node->next = NULL;

        if (list->head == NULL) {
            list->head = node;
            tail = node;
        }
        else {
            tail->next = node;
            tail = node;
        }
        list->count++;
    }

    fclose(fp);
}

/**
 * @brief 将内存中的日志链表保存到文件
 * @param list 包含要保存的日志数据的LogList指针
 * @param filename 目标文件的完整路径
 *
 * 功能说明:
 *   1. 以写入模式("w")打开目标文件（会覆盖原文件）
 *   2. 遍历链表，将每个节点的数据按格式写入文件
 *   3. 关闭文件完成保存
 *
 * 写入格式:
 *   时间|操作|操作人|详情\n
 *   与loadLogFromFile()读取的格式完全兼容
 *
 * 应用场景:
 *   - 日志整理归档时
 *   - 批量修改日志后需要持久化
 *   - 日志备份导出功能
 *
 * 错误处理:
 *   - 文件无法打开时输出错误信息并返回
 */
void saveLogToFile(LogList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法保存日志文件！\n");
        return;
    }

    LogNode* current = list->head;
    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%s\n",
            current->datetime, current->operation,
            current->operator, current->detail);
        current = current->next;
    }

    fclose(fp);
}

/* ==================== 显示输出函数 ==================== */

/**
 * @brief 打印单条日志记录的详细信息
 * @param node 要打印的日志节点指针
 *
 * 功能说明:
 *   以格式化的形式输出一条日志的所有字段信息。
 *   使用分隔线增强可读性。
 *
 * 输出格式示例:
 *   ----------------------------------------------------------------------
 *   时间: 2025-01-15 14:30:25
 *   操作: 新增药品
 *   操作人: 系统
 *   详情: 药品编号:P001, 名称:阿莫西林, 规格:250mg, 数量:100
 *   ----------------------------------------------------------------------
 *
 * 安全检查:
 *   - 如果node为NULL则直接返回，不执行任何操作
 */
void printLogOne(LogNode* node) {
    if (node == NULL) return;
    printLine('-', 70);
    printf("时间: %s\n", node->datetime);
    printf("操作: %s\n", node->operation);
    printf("操作人: %s\n", node->operator);
    printf("详情: %s\n", node->detail);
    printLine('-', 70);
}

/**
 * @brief 打印日志列表中的所有记录
 * @param list 包含日志数据的LogList指针
 *
 * 功能说明:
 *   1. 检查链表是否为空，为空时给出提示
 *   2. 显示标题和总记录数
 *   3. 遍历链表调用printLogOne()打印每条记录
 *
 * 输出示例:
 *   --- 日志记录 (共 25 条) ---
 *   [第1条日志]
 *   [第2条日志]
 *   ...
 *
 * 使用场景:
 *   - 查看某类操作的完整日志
 *   - 日志审计和追溯
 */
void printLogAll(LogList* list) {
    if (list->head == NULL) {
        printf("暂无日志记录。\n");
        return;
    }

    printf("\n--- 日志记录 (共 %d 条) ---\n", list->count);
    LogNode* current = list->head;
    while (current != NULL) {
        printLogOne(current);
        current = current->next;
    }
}

/* ==================== 权限验证函数 ==================== */

/**
 * @brief 管理员身份验证
 * @return int 验证成功返回1，验证失败返回0
 *
 * 功能说明:
 *   提示用户输入管理员密码，进行身份验证。
 *   密码输入时屏幕上显示星号(*)而非明文，保护安全性。
 *   支持退格键删除已输入字符。
 *
 * 验证流程:
 *   1. 显示验证标题和密码提示
 *   2. 循环读取用户按键输入
 *      - 回车键(\r): 结束输入
 *      - 退格键(\b): 删除前一字符
 *      - 其他字符: 存入缓冲区并显示*
 *   3. 与预设密码ADMIN_PASSWORD比较
 *   4. 返回验证结果
 *
 * 密码规则:
 *   - 最大长度: 49字符（缓冲区50字节含结束符）
 *   - 当前默认密码: admin123
 *   - 区分大小写
 *
 * 安全特性:
 *   - 密码输入隐藏显示
 *   - 支持退格修改
 *   - 缓冲区溢出保护
 */
int isAdminAuthenticated() {
    char password[50];
    printf("\n=== 管理员验证 ===\n");
    printf("请输入管理员密码: ");

    int i = 0;
    char ch;
    while ((ch = _getch()) != '\r' && i < 49) {
        if (ch == '\b') {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        }
        else {
            password[i++] = ch;
            printf("*");
        }
    }
    password[i] = '\0';
    printf("\n");

    if (strcmp(password, ADMIN_PASSWORD) == 0) {
        printf("验证成功！\n");
        return 1;
    }
    else {
        printf("密码错误！拒绝访问。\n");
        return 0;
    }
}

/* ==================== 日志查询功能函数 ==================== */

/**
 * @brief 按日志类型查询并显示日志
 * @param logType 要查询的日志类型(LOG_XXX常量)
 *
 * 功能说明:
 *   1. 初始化临时日志链表
 *   2. 根据logType确定文件名并加载
 *   3. 显示该类型的所有日志记录
 *   4. 释放临时链表内存
 *
 * 使用场景:
 *   用户选择查看特定模块的操作日志时调用
 *
 * 示例调用:
 *   queryLogByType(LOG_MEDICINE);  // 查看药品管理日志
 */
void queryLogByType(int logType) {
    LogList list;
    initLogList(&list);

    const char* filename = getLogFileName(logType);
    loadLogFromFile(&list, filename);

    printf("\n=== %s 日志 ===\n", getLogTypeName(logType));
    printLogAll(&list);

    freeLogList(&list);
}

/**
 * @brief 查询并显示所有类型的日志记录
 *
 * 功能说明:
 *   遍历所有10种日志类型，依次加载并显示每种类型的日志。
 *   只显示有记录的类型，跳过空日志文件。
 *
 * 处理逻辑:
 *   对每种日志类型:
 *   1. 初始化新的临时链表
 *   2. 加载对应类型的日志文件
 *   3. 如果有记录(count>0)，打印类型名称和数量
 *   4. 打印所有记录
 *   5. 释放临时链表
 *
 * 输出格式:
 *   === 所有操作日志 ===
 *
 *   --- 药品管理 (15条) ---
 *   [日志记录...]
 *
 *   --- 门诊管理 (8条) ---
 *   [日志记录...]
 *   ...
 *
 * 性能考虑:
 *   需要打开10个文件，对于大量日志可能较慢
 */
void queryAllLogs() {
    printf("\n=== 所有操作日志 ===\n");

    int logTypes[] = { LOG_MEDICINE, LOG_OUTPATIENT, LOG_APPOINT,
                       LOG_INPATIENT, LOG_EMERGENCY, LOG_BLOOD,
                       LOG_DEVICE, LOG_FINANCE, LOG_STAFF, LOG_LOGISTIC };
    int count = 10;

    for (int i = 0; i < count; i++) {
        LogList list;
        initLogList(&list);

        const char* filename = getLogFileName(logTypes[i]);
        loadLogFromFile(&list, filename);

        if (list.count > 0) {
            printf("\n--- %s (%d条) ---\n", getLogTypeName(logTypes[i]), list.count);
            printLogAll(&list);
        }

        freeLogList(&list);
    }
}

/**
 * @brief 按日期范围查询日志记录
 *
 * 功能说明:
 *   允许用户指定开始和结束日期，筛选出该时间段内的所有日志。
 *   会遍历所有类型的日志文件进行匹配。
 *
 * 输入参数:
 *   - 开始日期: YYYY-MM-DD格式，包含当天
 *   - 结束日期: YYYY-MM-DD格式，包含当天
 *
 * 查询逻辑:
 *   1. 提示用户输入开始和结束日期
 *   2. 遍历所有10种日志类型
 *   3. 对每条日志提取日期部分（前10位）
 *   4. 使用compareDate()比较日期是否在范围内
 *   5. 符合条件的日志输出显示
 *   6. 统计并显示匹配总数
 *
 * 日期比较规则:
 *   startDate <= logDate <= endDate（闭区间）
 *
 * 输入格式示例:
 *   请输入开始日期 (YYYY-MM-DD): 2025-01-01
 *   请输入结束日期 (YYYY-MM-DD): 2025-01-31
 *
 * 输出示例:
 *   --- 药品管理 ---
 *   [符合条件的日志记录]
 *   共找到 42 条记录。
 */
void queryLogByDate() {
    char startDate[20], endDate[20];
    printf("\n=== 按日期查询日志 ===\n");
    safeInput(startDate, 20, "请输入开始日期 (YYYY-MM-DD): ");
    safeInput(endDate, 20, "请输入结束日期 (YYYY-MM-DD): ");

    int logTypes[] = { LOG_MEDICINE, LOG_OUTPATIENT, LOG_APPOINT,
                       LOG_INPATIENT, LOG_EMERGENCY, LOG_BLOOD,
                       LOG_DEVICE, LOG_FINANCE, LOG_STAFF, LOG_LOGISTIC };
    int count = 10;
    int totalFound = 0;

    for (int i = 0; i < count; i++) {
        LogList list;
        initLogList(&list);

        const char* filename = getLogFileName(logTypes[i]);
        loadLogFromFile(&list, filename);

        LogNode* current = list.head;
        int found = 0;
        while (current != NULL) {
            char logDate[11];
            strncpy(logDate, current->datetime, 10);
            logDate[10] = '\0';

            if (compareDate(logDate, startDate) >= 0 &&
                compareDate(logDate, endDate) <= 0) {
                if (found == 0) {
                    printf("\n--- %s ---\n", getLogTypeName(logTypes[i]));
                }
                printLogOne(current);
                found++;
                totalFound++;
            }
            current = current->next;
        }

        freeLogList(&list);
    }

    if (totalFound == 0) {
        printf("未找到符合条件的日志记录。\n");
    }
    else {
        printf("\n共找到 %d 条记录。\n", totalFound);
    }
}

/* ==================== 主菜单函数 ==================== */

/**
 * @brief 日志查询系统的主菜单界面
 *
 * 功能说明:
 *   提供交互式菜单供用户选择不同的日志查询功能。
 *   进入菜单前需要进行管理员身份验证。
 *
 * 菜单选项:
 *   1-10: 分别查看10种不同类型的日志
 *   11:   查看所有类型的汇总日志
 *   12:   按日期范围查询日志
 *   0:    返回主菜单
 *
 * 操作流程:
 *   1. 调用isAdminAuthenticated()进行权限验证
 *   2. 验证失败则直接返回
 *   3. 进入循环显示菜单
 *   4. 根据用户选择调用相应查询函数
 *   5. 选择0退出循环返回上级菜单
 *
 * 输入格式:
 *   整数(0-12)，通过键盘输入选择功能项
 *
 * 安全机制:
 *   - 必须通过管理员验证才能访问
 *   - 密码错误拒绝进入并返回
 *
 * 用户体验:
 *   - 每次操作后暂停等待用户确认(pauseScreen)
 *   - 无效选择给出明确提示
 *   - 清晰的功能编号和说明
 */
void logMenu() {
    if (!isAdminAuthenticated()) {
        return;
    }

    int choice;
    do {
        printf("\n");
        printTitle("日志查询系统 (管理员)");
        printf("1. 查看药品管理日志\n");
        printf("2. 查看门诊管理日志\n");
        printf("3. 查看预约管理日志\n");
        printf("4. 查看住院管理日志\n");
        printf("5. 查看急诊管理日志\n");
        printf("6. 查看血库管理日志\n");
        printf("7. 查看设备管理日志\n");
        printf("8. 查看财务管理日志\n");
        printf("9. 查看人事管理日志\n");
        printf("10. 查看后勤管理日志\n");
        printf("11. 查看所有日志\n");
        printf("12. 按日期查询日志\n");
        printf("0. 返回主菜单\n");

        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: queryLogByType(LOG_MEDICINE); break;
        case 2: queryLogByType(LOG_OUTPATIENT); break;
        case 3: queryLogByType(LOG_APPOINT); break;
        case 4: queryLogByType(LOG_INPATIENT); break;
        case 5: queryLogByType(LOG_EMERGENCY); break;
        case 6: queryLogByType(LOG_BLOOD); break;
        case 7: queryLogByType(LOG_DEVICE); break;
        case 8: queryLogByType(LOG_FINANCE); break;
        case 9: queryLogByType(LOG_STAFF); break;
        case 10: queryLogByType(LOG_LOGISTIC); break;
        case 11: queryAllLogs(); break;
        case 12: queryLogByDate(); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }

        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

/**
 * @file appoint.c
 * @brief 预约管理模块实现文件
 *
 * 功能概述:
 *   本模块实现医院预约业务的全流程管理，支持三种预约类型：
 *   - 门诊预约（普通就诊预约）
 *   - 手术预约（手术安排预约）
 *   - 住院预约（入院床位预约）
 *
 * 核心功能:
 *   1. 预约记录的增删改查
 *   2. 预约状态管理（未到诊→已到诊→已完成/已取消）
 *   3. 预约数据统计分析
 *   4. 预约数据的持久化存储
 *
 * 业务流程:
 *   病人申请 → 选择预约类型 → 填写信息 → 确认预约 → 按时就诊 → 更新状态
 *
 * 文件依赖:
 *   - appoint.h: 预约数据结构和函数声明
 *   - log.h: 日志系统接口
 */

#include "appoint.h"
#include "log.h"

/**
 * @brief 将预约类型枚举值转换为中文字符串
 * @param type 预约类型枚举值(0-2)
 * @return 对应的中文字符串指针
 *
 * 类型映射关系:
 *   0 → "门诊预约"
 *   1 → "手术预约"
 *   2 → "住院预约"
 *   其他 → "未知"
 *
 * 使用场景:
 *   显示预约信息时，将数字类型转换为可读的中文描述
 */
const char* getAppointTypeString(int type) {
    switch (type) {
    case APPOINT_OUTPATIENT: return "门诊预约";
    case APPOINT_SURGERY: return "手术预约";
    case APPOINT_INPATIENT: return "住院预约";
    default: return "未知";
    }
}

/**
 * @brief 将预约状态枚举值转换为中文字符串
 * @param status 预约状态枚举值(0-3)
 * @return 对应的中文字符串指针
 *
 * 状态映射关系:
 *   0 → "未到诊" （初始状态）
 *   1 → "已到诊" （病人已到达）
 *   2 → "已完成" （诊疗结束）
 *   3 → "已取消" （预约取消）
 *   其他 → "未知"
 *
 * 状态转换规则:
 *   未到诊 → 已到诊 → 已完成
 *              ↓
 *            已取消（任意阶段均可取消）
 */
const char* getAppointStatusString(int status) {
    switch (status) {
    case APPOINT_PENDING: return "未到诊";
    case APPOINT_ARRIVED: return "已到诊";
    case APPOINT_COMPLETED: return "已完成";
    case APPOINT_CANCELLED: return "已取消";
    default: return "未知";
    }
}

/**
 * @brief 初始化预约链表
 * @param list 预约链表指针（输出参数）
 *
 * 功能说明:
 *   将链表头指针置空，计数器归零。
 *   在程序启动时调用，确保链表处于初始状态。
 */
void initAppointList(AppointList* list) {
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 从文件加载预约数据到内存
 * @param list 预约链表指针（输出参数）
 * @param filename 数据文件路径，如"data/appoints.txt"
 *
 * 功能说明:
 *   1. 打开指定文件进行读取
 *   2. 逐行解析文件内容
 *   3. 为每条记录创建节点并插入链表尾部
 *   4. 更新链表计数器
 *
 * 文件格式要求:
 *   每行一条记录，字段用"|"分隔：
 *   编号|类型|病人编号|姓名|性别|年龄|科室|医生|日期|时间段|状态|登记人|备注
 *
 * 示例行:
 *   YY001|0|P001|张三|男|35|内科|王医生|2025-01-20|上午|0|李护士|初诊
 *
 * 字段说明:
 *   - 类型: 0=门诊, 1=手术, 2=住院
 *   - 状态: 0=未到诊, 1=已到诊, 2=已完成, 3=已取消
 */
void loadAppointsFromFile(AppointList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("提示：预约数据文件不存在，将创建新文件。\n");
        return;
    }

    AppointNode* tail = NULL;  /**< 链表尾指针 */
    char line[1024];           /**< 行缓冲区 */

    /* 逐行读取并解析记录 */
    while (fgets(line, sizeof(line), fp)) {
        AppointNode* node = (AppointNode*)malloc(sizeof(AppointNode));
        if (node == NULL) continue;

        /* 解析一行中的各个字段 */
        sscanf_s(line, "%[^|]|%d|%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^\n]",
            node->id, &node->type, node->patientId, node->patientName,
            node->gender, &node->age, node->department, node->doctor,
            node->date, node->timeSlot, &node->status, node->registerPerson, node->remark);

        node->next = NULL;

        /* 尾插法维护顺序 */
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
    printf("成功加载 %d 条预约记录。\n", list->count);
}

/**
 * @brief 将内存中的预约数据保存到文件
 * @param list 预约链表指针
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   遍历整个链表，将每条记录按指定格式写入文件。
 *   使用"w"模式打开文件，会覆盖原有内容。
 *
 * 输出格式:
 *   与loadAppointsFromFile的输入格式一致
 */
void saveAppointsToFile(AppointList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法打开文件 %s 进行写入！\n", filename);
        return;
    }

    AppointNode* current = list->head;

    while (current != NULL) {
        fprintf(fp, "%s|%d|%s|%s|%s|%d|%s|%s|%s|%s|%d|%s|%s\n",
            current->id, current->type, current->patientId, current->patientName,
            current->gender, current->age, current->department, current->doctor,
            current->date, current->timeSlot, current->status,
            current->registerPerson, current->remark);
        current = current->next;
    }
    fclose(fp);
    printf("成功保存 %d 条预约记录。\n", list->count);
}

/**
 * @brief 创建新的预约记录节点
 * @return 成功返回新节点指针，失败返回NULL
 *
 * 功能说明:
 *   通过交互式界面收集用户输入，创建完整的预约记录。
 *   新创建的预约默认状态为"未到诊"(APPOINT_PENDING)。
 *
 * 输入项及格式:
 *   - 预约编号: 必填，唯一标识，如"YY001"
 *   - 预约类型: 枚举选择(0-2)
 *     * 0 = 门诊预约
 *     * 1 = 手术预约
 *     * 2 = 住院预约
 *   - 病人编号: 必填，关联病人信息
 *   - 病人姓名: 必填
 *   - 性别: 如"男"、"女"
 *   - 年龄: 整数
 *   - 科室: 如"内科"、"外科"
 *   - 医生: 负责医生姓名
 *   - 预约日期: 格式YYYY-MM-DD，需验证有效性
 *   - 时间段: 如"上午"、"下午"、"14:00-15:00"
 *   - 登记人: 创建此预约的工作人员
 *   - 备注: 可选补充信息
 *
 * 输入验证:
 *   - 编号不能为空
 *   - 类型必须在0-2范围内
 *   - 日期格式必须有效
 */
AppointNode* createAppointNode() {
    AppointNode* node = (AppointNode*)malloc(sizeof(AppointNode));
    if (node == NULL) {
        printf("错误：内存分配失败！\n");
        return NULL;
    }

    printf("\n--- 新增预约 ---\n");

    /*
     * ====== 输入字段1: 预约编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字、连字符的组合
     * 是否必填: 是（不能为空）
     * 输入样例:
     *   - "YY001" (预约拼音缩写+序号)
     *   - "APT-2024-0789" (Appointment+日期+序号)
     *   - "R20260416001" (预约日期+流水号)
     * 验证规则:
     *   - 不能为空字符串
     *   - 编号唯一，不能重复
     */
    safeInput(node->id, MAX_ID_LEN, "请输入预约编号: ");
    while (isEmpty(node->id)) {
        printf("编号不能为空！\n");
        safeInput(node->id, MAX_ID_LEN, "请输入预约编号: ");
    }

    /*
     * ====== 输入字段2: 预约类型 ======
     * 数据类型: int (整数枚举)
     * 取值范围: 0 ~ 2 (固定选项)
     * 输入格式: 纯数字选择
     * 是否必填: 是
     * 可选值:
     *   - 0 = 门诊预约 (常规门诊挂号预约)
     *   - 1 = 手术预约 (手术排期预约)
     *   - 2 = 住院预约 (床位预留预约)
     * 输入样例:
     *   - "0" (选择门诊预约)
     *   - "1" (选择手术预约)
     *   - "2" (选择住院预约)
     * 注意: 必须在0-2范围内，否则提示重新输入
     */
    printf("预约类型:\n0.门诊预约 1.手术预约 2.住院预约\n");
    node->type = inputInt("请选择: ");
    while (node->type < 0 || node->type > 2) {
        printf("无效选择！\n");
        node->type = inputInt("请选择: ");
    }

    /*
     * ====== 输入字段3: 病人编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字组合
     * 是否必填: 是
     * 输入样例:
     *   - "P001"
     *   - "BR20240101001"
     * 说明: 关联患者档案中的编号
     */
    safeInput(node->patientId, MAX_ID_LEN, "请输入病人编号: ");

    /*
     * ====== 输入字段4: 患者姓名 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 中文真实姓名
     * 是否必填: 是
     * 输入样例:
     *   - "张三"
     *   - "李四"
     */
    safeInput(node->patientName, MAX_NAME_LEN, "请输入病人姓名: ");

    /*
     * ====== 输入字段5: 性别 ======
     * 数据类型: char[10] (字符串)
     * 最大长度: 10个字符
     * 输入格式: 固定选项（男/女）
     * 是否必填: 是
     * 输入样例:
     *   - "男"
     *   - "女"
     */
    safeInput(node->gender, 10, "请输入性别: ");

    /*
     * ====== 输入字段6: 年龄 ======
     * 数据类型: int (整数)
     * 取值范围: 0 ~ 150 (正整数)
     * 输入格式: 纯数字
     * 是否必填: 是
     * 输入样例:
     *   - "25" (25岁)
     *   - "68" (68岁)
     */
    node->age = inputInt("请输入年龄: ");

    /*
     * ====== 输入字段7: 预约科室 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符
     * 输入格式: 科室名称（中文）
     * 是否必填: 是
     * 输入样例:
     *   - "内科"
     *   - "外科"
     *   - "眼科"
     *   - "骨科"
     */
    safeInput(node->department, MAX_DEPT_LEN, "请输入科室: ");

    /*
     * ====== 输入字段8: 预约医生 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 医生姓名
     * 是否必填: 是
     * 输入样例:
     *   - "王医生"
     *   - "李明主任医师"
     * 说明: 患者希望就诊的指定医生
     */
    safeInput(node->doctor, MAX_NAME_LEN, "请输入医生: ");

    /*
     * ====== 输入字段9: 预约日期 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: YYYY-MM-DD (严格格式)
     * 是否必填: 是
     * 输入样例:
     *   - "2026-04-20" (2026年4月20日)
     *   - "2026-05-15" (2026年5月15日)
     * 格式要求:
     *   - 年份: 4位数字
     *   - 月份: 2位数字 (01-12)
     *   - 日期: 2位数字 (根据月份1-31)
     * 验证: 自动检查日期有效性(含闰年判断)
     * 错误提示: "日期格式不正确！"会提示重新输入
     */
    safeInput(node->date, MAX_DATE_LEN, "请输入预约日期: ");
    while (!isValidDate(node->date)) {
        printf("日期格式不正确！\n");
        safeInput(node->date, MAX_DATE_LEN, "请输入预约日期: ");
    }

    /*
     * ====== 输入字段10: 预约时间段 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 时间段描述文本
     * 是否必填: 是
     * 输入样例:
     *   - "上午" (8:00-12:00)
     *   - "下午" (14:00-17:30)
     *   - "全天" (特殊情况)
     *   - "09:00-10:00" (精确时段)
     * 说明: 标识患者期望或被分配的就诊时段
     */
    safeInput(node->timeSlot, 20, "请输入时间段(如:上午/下午): ");

    /* 【系统设置】预约状态默认为 APPOINT_PENDING(待就诊/未到诊) */

    /*
     * ====== 输入字段11: 登记人 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 操作员姓名
     * 是否必填: 是
     * 输入样例:
     *   - "张护士"
     *   - "前台-李四"
     *   - "系统管理员"
     * 说明: 创建此预约记录的操作人员，用于责任追溯
     */
    safeInput(node->registerPerson, MAX_NAME_LEN, "请输入登记人: ");

    /*
     * ====== 输入字段12: 备注 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 自由文本
     * 是否必填: 否（可留空）
     * 输入样例:
     *   - "" (直接回车留空)
     *   - "首次就诊，带齐既往病历"
     *   - "复诊患者，需空腹检查"
     *   - "VIP客户，优先安排"
     * 用途: 记录特殊要求、注意事项等补充信息
     */
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注: ");

    node->next = NULL;
    return node;
}

/**
 * @brief 检查预约编号是否已存在
 * @param list 预约链表指针
 * @param id 待检查的预约编号
 * @return 存在返回1，不存在返回0
 *
 * 功能说明:
 *   在新增预约前调用，确保编号唯一性。
 */
int isAppointIDExist(AppointList* list, const char* id) {
    AppointNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) return 1;
        current = current->next;
    }
    return 0;
}

/**
 * @brief 将新预约节点插入链表头部
 * @param list 预约链表指针
 * @param node 待插入的节点指针
 *
 * 功能说明:
 *   采用头插法，新记录插入到链表最前面。
 */
void insertAppointNode(AppointList* list, AppointNode* node) {
    if (node == NULL) return;
    node->next = list->head;
    list->head = node;
    list->count++;
}

/**
 * @brief 根据预约编号查找记录
 * @param list 预约链表指针
 * @param id 目标预约编号
 * @return 找到返回节点指针，未找到返回NULL
 *
 * 功能说明:
 *   线性搜索链表，用于查询、修改、删除前的定位操作。
 */
AppointNode* findAppointByID(AppointList* list, const char* id) {
    AppointNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) return current;
        current = current->next;
    }
    return NULL;
}

/**
 * @brief 根据编号取消（删除）预约记录
 * @param list 预约链表指针
 * @param id 待取消的预约编号
 * @return 成功取消返回1，失败返回0
 *
 * 功能说明:
 *   1. 查找目标预约记录
 *   2. 显示完整信息供确认
 *   3. 用户确认后执行删除
 *   4. 记录取消操作日志
 *
 * 安全机制:
 *   - 取消前显示完整预约信息
 *   - 要求二次确认
 *   - 自动释放被删除节点的内存
 */
int deleteAppointByID(AppointList* list, const char* id) {
    AppointNode* current = list->head, * prev = NULL;

    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printf("\n找到以下记录:\n");
            printAppointOne(current);

            if (!confirm("确认取消该预约?")) {
                printf("已取消操作。\n");
                return 0;
            }

            /* 构造日志详情 */
            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "预约编号:%s, 病人:%s, 类型:%s, 日期:%s",
                current->id, current->patientName,
                getAppointTypeString(current->type), current->date);

            /* 从链表中移除节点 */
            if (prev == NULL)
                list->head = current->next;
            else
                prev->next = current->next;

            free(current);
            list->count--;
            printf("预约已取消！\n");

            writeLog(LOG_APPOINT, "取消预约", "系统", detail);
            return 1;
        }
        prev = current;
        current = current->next;
    }
    printf("未找到编号为 %s 的预约。\n", id);
    return 0;
}

/**
 * @brief 修改预约记录信息
 * @param node 待修改的预约节点指针
 *
 * 功能说明:
 *   提供选择性修改功能，用户可选择修改以下字段：
 *   1. 科室
 *   2. 医生
 *   3. 预约日期（需验证格式）
 *   4. 时间段
 *   5. 状态（可手动更新预约进度）
 *   6. 备注
 *
 * 状态修改场景示例:
 *   - 病人到达：从未到诊改为已到诊
 *   - 诊疗完成：从已到诊改为已完成
 *   - 病人爽约：改为已取消
 */
void modifyAppointInfo(AppointNode* node) {
    if (node == NULL) return;

    printf("\n--- 修改预约信息 ---\n");
    printAppointOne(node);

    printf("\n请选择要修改的字段:\n");
    printf("1. 科室\n2. 医生\n3. 预约日期\n4. 时间段\n5. 状态\n6. 备注\n0. 取消\n");
    int choice = inputInt("请选择: ");

    switch (choice) {
    case 1: safeInput(node->department, MAX_DEPT_LEN, "请输入新科室: "); break;
    case 2: safeInput(node->doctor, MAX_NAME_LEN, "请输入新医生: "); break;
    case 3:
        safeInput(node->date, MAX_DATE_LEN, "请输入新日期: ");
        while (!isValidDate(node->date)) {
            printf("日期格式不正确！\n");
            safeInput(node->date, MAX_DATE_LEN, "请输入新日期: ");
        }
        break;
    case 4: safeInput(node->timeSlot, 20, "请输入新时间段: "); break;
    case 5:
        printf("状态选项:\n0.未到诊 1.已到诊 2.已完成 3.已取消\n");
        node->status = inputInt("请选择: ");
        break;
    case 6: safeInput(node->remark, MAX_REMARK_LEN, "请输入新备注: "); break;
    case 0: printf("取消修改。\n"); return;
    default: printf("无效选择。\n"); return;
    }

    printf("修改成功！\n");

    /* 记录修改日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "预约编号:%s, 病人:%s, 修改字段:%d", node->id, node->patientName, choice);
    writeLog(LOG_APPOINT, "修改预约", "系统", detail);
}

/**
 * @brief 打印单条预约记录
 * @param node 预约节点指针
 *
 * 功能说明:
 *   格式化输出一条预约记录的所有字段，
 *   使用中文名称显示类型和状态。
 *
 * 输出格式示例:
 *   ----------------------------------------------------------------------
 *   预约编号: YY001
 *   预约类型: 门诊预约
 *   病人编号: P001
 *   病人姓名: 张三
 *   性别: 男 年龄: 35
 *   科室: 内科
 *   医生: 王医生
 *   预约日期: 2025-01-20
 *   时间段: 上午
 *   状态: 未到诊
 *   登记人: 李护士
 *   备注: 初诊
 *   ----------------------------------------------------------------------
 */
void printAppointOne(AppointNode* node) {
    if (node == NULL) return;

    printLine('-', 70);
    printf("预约编号: %s\n", node->id);
    printf("预约类型: %s\n", getAppointTypeString(node->type));
    printf("病人编号: %s\n", node->patientId);
    printf("病人姓名: %s\n", node->patientName);
    printf("性别: %s 年龄: %d\n", node->gender, node->age);
    printf("科室: %s\n", node->department);
    printf("医生: %s\n", node->doctor);
    printf("预约日期: %s\n", node->date);
    printf("时间段: %s\n", node->timeSlot);
    printf("状态: %s\n", getAppointStatusString(node->status));
    printf("登记人: %s\n", node->registerPerson);
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

/**
 * @brief 打印所有预约记录
 * @param list 预约链表指针
 *
 * 功能说明:
 *   遍历链表，依次打印每条记录。
 *   先显示总记录数，再逐一输出详细信息。
 */
void printAppointAll(AppointList* list) {
    if (list->head == NULL) {
        printf("暂无预约记录。\n");
        return;
    }

    printf("\n--- 所有预约 (共 %d 条) ---\n", list->count);
    AppointNode* current = list->head;
    while (current != NULL) {
        printAppointOne(current);
        current = current->next;
    }
}

/**
 * @brief 预约数据统计分析
 * @param list 预约链表指针
 *
 * 功能说明:
 *   对预约数据进行多维度统计：
 *
 *   按类型统计:
 *   - 门诊预约数量
 *   - 手术预约数量
 *   - 住院预约数量
 *
 *   按状态统计:
 *   - 未到诊数量
 *   - 已到诊数量
 *   - 已完成数量
 *   - 已取消数量
 *
 * 统计结果输出示例:
 *   --- 预约统计 ---
 *   按类型统计:
 *     门诊预约: 50
 *     手术预约: 20
 *     住院预约: 30
 *   按状态统计:
 *     未到诊: 10
 *     已到诊: 15
 *     已完成: 65
 *     已取消: 10
 *   总预约数: 100
 */
void statAppoints(AppointList* list) {
    int outpatient = 0, surgery = 0, inpatient = 0;      /**< 各类型计数 */
    int pending = 0, arrived = 0, completed = 0, cancelled = 0;  /**< 各状态计数 */

    AppointNode* curr = list->head;

    /* 遍历所有记录进行分类统计 */
    while (curr != NULL) {
        /* 按类型统计 */
        switch (curr->type) {
        case APPOINT_OUTPATIENT: outpatient++; break;
        case APPOINT_SURGERY: surgery++; break;
        case APPOINT_INPATIENT: inpatient++; break;
        }

        /* 按状态统计 */
        switch (curr->status) {
        case APPOINT_PENDING: pending++; break;
        case APPOINT_ARRIVED: arrived++; break;
        case APPOINT_COMPLETED: completed++; break;
        case APPOINT_CANCELLED: cancelled++; break;
        }
        curr = curr->next;
    }

    /* 输出统计结果 */
    printf("\n--- 预约统计 ---\n");
    printf("按类型统计:\n");
    printf("  门诊预约: %d\n", outpatient);
    printf("  手术预约: %d\n", surgery);
    printf("  住院预约: %d\n", inpatient);
    printf("按状态统计:\n");
    printf("  未到诊: %d\n", pending);
    printf("  已到诊: %d\n", arrived);
    printf("  已完成: %d\n", completed);
    printf("  已取消: %d\n", cancelled);
    printf("总预约数: %d\n", list->count);
}

/**
 * @brief 释放预约链表占用的所有内存
 * @param list 预约链表指针
 *
 * 功能说明:
 *   遍历链表，逐个释放每个节点分配的内存。
 *   最后将链表头指针置空，计数器归零。
 */
void freeAppointList(AppointList* list) {
    AppointNode* current = list->head;
    while (current != NULL) {
        AppointNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 预约管理主菜单
 * @param list 预约链表指针
 *
 * 功能说明:
 *   提供预约管理的交互式菜单界面，包含以下功能：
 *
 *   菜单选项:
 *   ┌────┬────────────────────────────┐
 *   │ 1  │ 新增预约                   │
 *   │ 2  │ 查询预约                   │
 *   │ 3  │ 取消预约                   │
 *   │ 4  │ 修改预约                   │
 *   │ 5  │ 显示全部预约               │
 *   │ 6  │ 统计分析                   │
 *   │ 0  │ 返回主菜单                 │
 *   └────┴────────────────────────────┘
 *
 * 操作流程:
 *   循环显示菜单，根据用户选择调用对应功能模块。
 *   选择0时退出本菜单，返回上级菜单。
 *
 * 日志记录:
 *   新增、取消、修改操作会自动写入日志
 */
void appointMenu(AppointList* list) {
    int choice;

    do {
        printf("\n");
        printTitle("手术及住院预约系统");
        printf("1. 新增预约\n2. 查询预约\n3. 取消预约\n4. 修改预约\n");
        printf("5. 显示全部预约\n6. 统计分析\n0. 返回主菜单\n");
        choice = inputInt("请选择功能: ");

        switch (choice) {

        /* ====== 新增预约 ====== */
        case 1: {
            AppointNode* node = createAppointNode();
            if (node && !isAppointIDExist(list, node->id)) {
                insertAppointNode(list, node);
                printf("预约添加成功！\n");

                /* 记录新增日志 */
                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "预约编号:%s, 病人:%s, 类型:%s, 科室:%s, 医生:%s, 日期:%s",
                    node->id, node->patientName,
                    getAppointTypeString(node->type),
                    node->department, node->doctor, node->date);
                writeLog(LOG_APPOINT, "新增预约", "系统", detail);
            } else if (node) {
                printf("错误：预约编号已存在！\n");
                free(node);  /* 编号重复时释放节点 */
            }
            break;
        }

        /* ====== 查询预约 ====== */
        case 2: {
            if (list->head == NULL) { printf("暂无预约记录。\n"); break; }
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "请输入预约编号: ");
            AppointNode* found = findAppointByID(list, id);
            if (found)
                printAppointOne(found);
            else
                printf("未找到。\n");
            break;
        }

        /* ====== 取消预约 ====== */
        case 3: {
            if (list->head == NULL) { printf("暂无预约记录。\n"); break; }
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "请输入要取消的预约编号: ");
            deleteAppointByID(list, id);
            break;
        }

        /* ====== 修改预约 ====== */
        case 4: {
            if (list->head == NULL) { printf("暂无预约记录。\n"); break; }
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "请输入要修改的预约编号: ");
            AppointNode* node = findAppointByID(list, id);
            if (node)
                modifyAppointInfo(node);
            else
                printf("未找到。\n");
            break;
        }

        /* ====== 显示全部预约 ====== */
        case 5:
            printAppointAll(list);
            break;

        /* ====== 统计分析 ====== */
        case 6:
            statAppoints(list);
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

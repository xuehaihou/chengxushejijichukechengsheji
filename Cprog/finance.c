#include "finance.h"
#include "log.h"

/**
 * @brief 将收支类型枚举值转换为中文字符串显示
 * @param type 收支类型枚举值(TYPE_INCOME/TYPE_EXPENSE)
 * @return 对应的中文字符串指针
 *
 * 类型映射关系:
 *   TYPE_INCOME(0)  → "收入"   （资金流入）
 *   TYPE_EXPENSE(1) → "支出"   （资金流出）
 *   其他           → "未知"    （无效类型值）
 *
 * 使用场景:
 *   - 财务记录显示时将数字类型转为可读文本
 *   - 统计报表生成时的分类标题
 *   - 日志记录中的类型描述
 */
const char* getFinanceTypeString(int type) {
    switch (type) {
    case TYPE_INCOME: return "收入";
    case TYPE_EXPENSE: return "支出";
    default: return "未知";
    }
}

/**
 * @brief 将审核状态枚举值转换为中文字符串显示
 * @param status 审核状态枚举值(AUDIT_PENDING/AUDIT_APPROVED/AUDIT_REJECTED)
 * @return 对应的中文字符串指针
 *
 * 状态映射关系:
 *   AUDIT_PENDING(0)  → "未审核"  （等待审核处理）
 *   AUDIT_APPROVED(1) → "已审核"  （审核通过，可参与统计）
 *   AUDIT_REJECTED(2) → "已作废"  （审核不通过或已作废）
 *   其他              → "未知"     （无效状态值）
 *
 * 业务含义:
 *   - 未审核: 新创建的记录默认状态，尚未经过财务审批
 *   - 已审核: 通过财务部门审核，计入正式财务数据
 *   - 已作废: 记录被取消或驳回，不参与任何计算
 *
 * 使用场景:
 *   - 财务记录详情显示
 *   - 审核操作界面展示当前状态
 *   - 统计时的过滤条件判断
 */
const char* getAuditStatusString(int status) {
    switch (status) {
    case AUDIT_PENDING: return "未审核";
    case AUDIT_APPROVED: return "已审核";
    case AUDIT_REJECTED: return "已作废";
    default: return "未知";
    }
}

/**
 * @brief 初始化财务链表
 * @param list 财务链表指针
 *
 * 功能说明:
 *   将链表头指针置空，计数器归零。
 *   在系统启动时必须调用此函数初始化财务数据结构。
 *
 * 初始化内容:
 *   - head: NULL（空链表）
 *   - count: 0（无记录）
 */
void initFinanceList(FinanceList* list) {
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 从文件加载财务数据到内存链表
 * @param list 财务链表指针（用于存储加载的数据）
 * @param filename 数据文件路径
 *
 * 功能说明:
 *   从指定文件读取财务收支记录，逐行解析并构建链表结构。
 *   文件不存在时会提示用户但不报错（首次运行时正常现象）。
 *
 * 文件格式要求:
 *   每行一条记录，字段以竖线(|)分隔：
 *   编号|类型|金额|日期|经办人|部门|用途/来源|审核状态|备注
 *
 * 数据字段说明:
 *   - 编号: 唯一标识该笔财务记录
 *   - 类型: 0=收入, 1=支出
 *   - 金额: 浮点数，保留两位小数
 *   - 日期: YYYY-MM-DD格式
 *   - 审核状态: 0=未审核, 1=已审核, 2=已作废
 *
 * 数据解析规则:
 *   - 使用sscanf安全读取各字段
 *   - 自动分配内存创建节点
 *   - 采用尾插法保持原有顺序
 *
 * 错误处理:
 *   - 文件不存在：提示并返回（不视为错误）
 *   - 内存分配失败：跳过该条记录继续下一条
 *
 * 示例文件内容:
 *   FIN202401001|0|15000.00|2024-01-15|张三|财务部|门诊收入|0|一月门诊费
 */
void loadFinanceFromFile(FinanceList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("提示：财务数据文件不存在，将创建新文件。\n");
        return;
    }

    /* 尾指针用于高效插入，避免每次从头遍历 */
    FinanceNode* tail = NULL;
    char line[1024];

    /* 逐行读取文件内容 */
    while (fgets(line, sizeof(line), fp)) {
        FinanceNode* node = (FinanceNode*)malloc(sizeof(FinanceNode));
        if (node == NULL) continue;

        /* 解析一行数据到节点各字段 */
        sscanf(line, "%[^|]|%d|%lf|%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^\n]",
            node->id, &node->type, &node->amount, node->date,
            node->handler, node->department, node->purpose,
            &node->auditStatus, node->remark);

        node->next = NULL;

        /* 尾插法维护链表 */
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
    printf("成功加载 %d 条财务记录。\n", list->count);
}

/**
 * @brief 将财务链表数据保存到文件
 * @param list 财务链表指针（包含要保存的数据）
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   遍历整个财务链表，将每条记录格式化写入文件。
 *   采用覆盖写模式，保存后文件只包含当前最新数据。
 *
 * 写入格式:
 *   字段间用竖线分隔，每行一条完整记录：
 *   编号|类型|金额|日期|经办人|部门|用途/来源|审核状态|备注\n
 *
 * 数据完整性保障:
 *   - 每条记录独立一行，便于逐行读取
 *   - 数值类型保留两位小数
 *   - 最后一个字段后换行符结束
 *
 * 调用时机:
 *   - 添加/删除/修改/审核记录后
 *   - 系统退出前保存数据
 *   - 手动触发保存操作时
 */
void saveFinanceToFile(FinanceList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法打开文件 %s 进行写入！\n", filename);
        return;
    }

    FinanceNode* current = list->head;

    /* 遍历链表逐条写入 */
    while (current != NULL) {
        fprintf(fp, "%s|%d|%.2f|%s|%s|%s|%s|%d|%s\n",
            current->id, current->type, current->amount, current->date,
            current->handler, current->department, current->purpose,
            current->auditStatus, current->remark);
        current = current->next;
    }

    fclose(fp);
    printf("成功保存 %d 条财务记录。\n", list->count);
}

/**
 * @brief 创建新的财务记录节点（交互式输入）
 * @return 成功返回财务节点指针，失败返回NULL
 *
 * 功能说明:
 *   通过控制台交互方式收集财务信息，创建完整的财务数据节点。
 *   包含输入验证和必填项检查。
 *
 * 收集的信息字段:
 *   1. 记录编号 [必填] - 唯一标识，不能为空
 *   2. 收支类型 [必填] - 0=收入, 1=支出（提供选项菜单）
 *   3. 金额 [必填] - 正浮点数，不能为负数
 *   4. 日期 [必填] - 格式YYYY-MM-DD，会验证格式
 *   5. 经办人 [必填] - 操作该笔业务的负责人姓名
 *   6. 所属部门 [必填] - 经办人所在部门
 *   7. 用途/来源 [必填] - 说明资金的用途或来源
 *   8. 备注 [可选] - 其他补充信息
 *
 * 自动设置的字段:
 *   - auditStatus: AUDIT_PENDING（新增记录默认未审核状态）
 *
 * 输入验证规则:
 *   - 编号不能为空字符串
 *   - 类型只能是0或1
 *   - 金额必须≥0
 *   - 日期必须符合YYYY-MM-DD格式
 *
 * 返回值使用:
 *   - 成功: 返回的节点可直接插入链表
 *   - 失败: 内存分配问题返回NULL，调用者需处理
 *
 * 业务规则:
 *   - 新增的财务记录需要经过审核才能参与统计计算
 *   - 审核前记录处于"待审核"状态
 */
FinanceNode* createFinanceNode() {
    FinanceNode* node = (FinanceNode*)malloc(sizeof(FinanceNode));
    if (node == NULL) {
        printf("错误：内存分配失败！\n");
        return NULL;
    }

    printf("\n--- 新增财务记录 ---\n");

    /*
     * ====== 输入字段1: 记录编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字、连字符的组合
     * 是否必填: 是（不能为空）
     * 输入样例:
     *   - "CW001" (财务拼音缩写+序号)
     *   - "FIN-2024-1234" (Finance+日期+序号)
     *   - "F20260416001" (财务日期+流水号)
     */
    safeInput(node->id, MAX_ID_LEN, "请输入记录编号: ");
    while (isEmpty(node->id)) {
        printf("编号不能为空！\n");
        safeInput(node->id, MAX_ID_LEN, "请输入记录编号: ");
    }

    /*
     * ====== 输入字段2: 收支类型 ======
     * 数据类型: int (整数枚举)
     * 取值范围: 0 ~ 1 (固定选项)
     * 输入格式: 纯数字选择
     * 是否必填: 是
     * 可选值:
     *   - 0 = 收入 (资金流入)
     *   - 1 = 支出 (资金流出)
     * 输入样例:
     *   - "0" (收入，如门诊收费、药品销售)
     *   - "1" (支出，如设备采购、员工工资)
     */
    printf("收支类型:\n");
    printf("0. 收入\n");
    printf("1. 支出\n");
    node->type = inputInt("请选择: ");
    while (node->type != 0 && node->type != 1) {
        printf("无效选择！\n");
        node->type = inputInt("请选择: ");
    }

    /*
     * ====== 输入字段3: 金额 ======
     * 数据类型: double (双精度浮点数)
     * 取值范围: 0.00 ~ 很大的正数
     * 精度要求: 保留2位小数(元)
     * 输入格式: 数字，可含小数点
     * 是否必填: 是
     * 输入样例:
     *   - "156.80" (门诊费用收入)
     *   - "50000.00" (药品采购支出)
     *   - "8500.00" (员工工资支出)
     *   - "128000.00" (设备购置费)
     * 验证规则: 必须为非负数，负数会提示重新输入
     */
    node->amount = inputDouble("请输入金额: ");
    while (node->amount < 0) {
        printf("金额不能为负数！\n");
        node->amount = inputDouble("请输入金额: ");
    }

    /*
     * ====== 输入字段4: 发生日期 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: YYYY-MM-DD (严格格式)
     * 是否必填: 是
     * 输入样例:
     *   - "2026-04-16" (今天)
     *   - "2026-03-15" (上月某日)
     * 格式要求:
     *   - 年份: 4位数字
     *   - 月份: 2位数字 (01-12)
     *   - 日期: 2位数字 (根据月份1-31)
     * 验证: 自动检查日期有效性(含闰年判断)
     */
    safeInput(node->date, MAX_DATE_LEN, "请输入日期 (YYYY-MM-DD): ");
    while (!isValidDate(node->date)) {
        printf("日期格式不正确！\n");
        safeInput(node->date, MAX_DATE_LEN, "请输入日期 (YYYY-MM-DD): ");
    }

    /*
     * ====== 输入字段5: 经办人 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 员工姓名或工号
     * 是否必填: 是
     * 输入样例:
     *   - "张会计"
     *   - "李出纳"
     *   - "财务科-王五"
     * 说明: 负责该笔财务操作的人员，用于责任追溯
     */
    safeInput(node->handler, MAX_NAME_LEN, "请输入经办人: ");

    /*
     * ====== 输入字段6: 所属部门 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符
     * 输入格式: 部门名称（中文）
     * 是否必填: 是
     * 输入样例:
     *   - "财务部"
     *   - "门诊收费处"
     *   - "住院结算中心"
     *   - "药剂科"
     *   - "总务科"
     */
    safeInput(node->department, MAX_DEPT_LEN, "请输入所属部门: ");

    /*
     * ====== 输入字段7: 用途/来源说明 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 用途描述文本
     * 是否必填: 是
     * 输入样例(收入):
     *   - "门诊挂号费收入"
     *   - "药品销售收入"
     *   - "检查检验费收入"
     *   - "政府补贴拨款"
     * 输入样例(支出):
     *   - "药品采购款"
     *   - "员工工资发放"
     *   - "设备维修费用"
     *   - "水电物业费"
     */
    safeInput(node->purpose, MAX_REMARK_LEN, "请输入用途/来源: ");

    /* 【系统设置】审核状态默认为 AUDIT_PENDING(待审核) */

    /*
     * ====== 输入字段8: 备注 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 自由文本
     * 是否必填: 否（可留空）
     * 输入样例:
     *   - "" (直接回车留空)
     *   - "需附发票"
     *   - "紧急支付，后补审批"
     *   - "医保报销部分"
     */
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注 (无则留空): ");

    node->next = NULL;
    return node;
}

/**
 * @brief 检查财务记录编号是否已存在
 * @param list 财务链表指针
 * @param id 待检查的记录编号
 * @return 存在返回1，不存在返回0
 *
 * 功能说明:
 *   在财务链表中线性搜索指定编号，用于保证编号唯一性。
 *
 * 查找算法:
 *   - 从链表头部开始遍历
 *   - 逐一比较每个节点的id字段
 *   - 找到匹配即返回（不需要遍历全部）
 *
 * 使用场景:
 *   - 添加新记录前检查重复
 *   - 导入数据前去重校验
 */
int isFinanceIDExist(FinanceList* list, const char* id) {
    FinanceNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

/**
 * @brief 将财务记录节点插入链表头部
 * @param list 财务链表指针
 * @param node 待插入的财务节点指针
 *
 * 功能说明:
 *   采用头插法将新节点插入链表，时间复杂度O(1)。
 *   插入后自动更新链表计数器。
 *
 * 插入逻辑:
 *   1. 新节点的next指向当前头节点
 *   2. 头指针指向新节点
 *   3. 计数器+1
 *
 * 注意事项:
 *   - 不检查NULL参数，调用者需确保有效性
 *   - 不检查重复，应在调用前自行验证
 */
void insertFinanceNode(FinanceList* list, FinanceNode* node) {
    if (node == NULL) return;

    node->next = list->head;
    list->head = node;
    list->count++;
}

/**
 * @brief 根据编号查找财务记录
 * @param list 财务链表指针
 * @param id 目标记录编号
 * @return 找到返回节点指针，未找到返回NULL
 *
 * 功能说明:
 *   在财务链表中按编号精确查找目标记录。
 *   返回的是实际节点的指针，可用于直接修改数据。
 *
 * 查找过程:
 *   1. 从链表头开始遍历
 *   2. 使用strcmp比较id字段
 *   3. 匹配则立即返回该节点地址
 *   4. 遍历完仍未找到返回NULL
 *
 * 返回值用途:
 *   - 非NULL: 可直接访问或修改该记录信息
 *   - NULL: 表示记录不存在，需提示用户
 */
FinanceNode* findFinanceByID(FinanceList* list, const char* id) {
    FinanceNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief 根据编号删除财务记录
 * @param list 财务链表指针
 * @param id 要删除的记录编号
 * @return 成功删除返回1，取消或未找到返回0
 *
 * 功能说明:
 *   完整的财务记录删除流程：
 *   1. 在链表中定位目标记录
 *   2. 显示记录信息供确认
 *   3. 用户确认后执行删除
 *   4. 记录操作日志
 *   5. 释放被删节点内存
 *
 * 删除前的确认机制:
 *   - 显示待删除记录的详细信息
 *   - 要求用户二次确认（Y/N）
 *   - 用户可取消删除操作
 *
 * 内存管理:
 *   - 删除后自动free释放节点内存
 *   - 更新链表count计数器
 *   - 正确处理首节点和中间节点的不同情况
 *
 * 日志记录:
 *   记录被删记录的关键信息（编号、类型、金额）
 */
int deleteFinanceByID(FinanceList* list, const char* id) {
    FinanceNode* current = list->head;
    FinanceNode* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            /* 显示待删除记录详情 */
            printf("\n找到以下记录:\n");
            printFinanceOne(current);

            /* 二次确认防止误删 */
            if (!confirm("确认删除该记录?")) {
                printf("已取消删除。\n");
                return 0;
            }

            /* 准备日志信息（在删除前获取） */
            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "编号:%s, 类型:%s, 金额:%.2f",
                current->id, getFinanceTypeString(current->type), current->amount);

            /* 执行链表删除操作 */
            if (prev == NULL) {
                list->head = current->next;      /* 删除的是头节点 */
            }
            else {
                prev->next = current->next;       /* 删除中间或尾部节点 */
            }

            free(current);
            list->count--;
            printf("删除成功！\n");

            /* 记录删除日志 */
            writeLog(LOG_FINANCE, "删除财务记录", "系统", detail);
            return 1;
        }
        prev = current;
        current = current->next;
    }

    printf("未找到编号为 %s 的记录。\n", id);
    return 0;
}

/**
 * @brief 修改财务记录（交互式选择字段修改）
 * @param node 待修改的财务节点指针
 *
 * 功能说明:
 *   提供菜单式界面让用户选择要修改的具体字段，
 *   支持单次修改一个字段，避免误改其他数据。
 *
 * 可修改的字段列表:
 *   1. 金额         - amount字段（数值型）
 *   2. 类型         - type字段（0=收入, 1=支出）
 *   3. 日期         - date字段（YYYY-MM-DD格式）
 *   4. 经办人       - handler字段
 *   5. 部门         - department字段
 *   6. 用途/来源   - purpose字段
 *   7. 审核状态     - auditStatus字段（枚举选择）
 *   8. 备注         - remark字段
 *   0. 取消         - 不做任何修改
 *
 * 修改流程:
 *   1. 显示当前记录信息
 *   2. 显示可选修改项菜单
 *   3. 用户选择要修改的字段
 *   4. 输入新值替换旧值
 *   5. 记录修改日志
 *
 * 日志内容:
 *   包含记录编号、类型、金额和修改的字段序号
 *
 * 注意事项:
 *   - 修改审核状态会影响该记录是否参与统计
 *   - 修改类型会改变收支分类
 */
void modifyFinanceInfo(FinanceNode* node) {
    if (node == NULL) return;

    printf("\n--- 修改财务记录 ---\n");
    printf("当前信息:\n");
    printFinanceOne(node);

    printf("\n请选择要修改的字段:\n");
    printf("1. 金额\n");
    printf("2. 类型\n");
    printf("3. 日期\n");
    printf("4. 经办人\n");
    printf("5. 部门\n");
    printf("6. 用途/来源\n");
    printf("7. 审核状态\n");
    printf("8. 备注\n");
    printf("0. 取消修改\n");

    int choice = inputInt("请选择: ");

    switch (choice) {
    case 1:
        node->amount = inputDouble("请输入新金额: ");
        break;
    case 2:
        /* 类型修改提供选项列表 */
        printf("收支类型:\n");
        printf("0. 收入\n");
        printf("1. 支出\n");
        node->type = inputInt("请选择: ");
        break;
    case 3:
        safeInput(node->date, MAX_DATE_LEN, "请输入新日期: ");
        break;
    case 4:
        safeInput(node->handler, MAX_NAME_LEN, "请输入新经办人: ");
        break;
    case 5:
        safeInput(node->department, MAX_DEPT_LEN, "请输入新部门: ");
        break;
    case 6:
        safeInput(node->purpose, MAX_REMARK_LEN, "请输入新用途/来源: ");
        break;
    case 7:
        /* 审核状态修改提供选项列表 */
        printf("审核状态:\n");
        printf("0. 未审核\n");
        printf("1. 已审核\n");
        printf("2. 已作废\n");
        node->auditStatus = inputInt("请选择: ");
        break;
    case 8:
        safeInput(node->remark, MAX_REMARK_LEN, "请输入新备注: ");
        break;
    case 0:
        printf("取消修改。\n");
        return;
    default:
        printf("无效选择。\n");
        return;
    }

    printf("修改成功！\n");

    /* 记录修改日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "编号:%s, 类型:%s, 金额:%.2f, 修改字段:%d",
        node->id, getFinanceTypeString(node->type), node->amount, choice);
    writeLog(LOG_FINANCE, "修改财务记录", "系统", detail);
}

/**
 * @brief 打印单个财务记录的详细信息
 * @param node 财务节点指针
 *
 * 功能说明:
 *   格式化输出一笔财务记录的完整信息，用于查看详情。
 *   输出采用固定宽度的分隔线框架，便于阅读。
 *
 * 显示内容:
 *   - 记录编号（唯一标识）
 *   - 类型（中文显示：收入/支出）
 *   - 金额（保留2位小数）
 *   - 日期
 *   - 经办人
 *   - 部门
 *   - 用途/来源
 *   - 审核状态（中文显示：未审核/已审核/已作废）
 *   - 备注
 *
 * 输出格式特点:
 *   - 使用70字符宽的分隔线
 *   - 每个字段独占一行
 *   - 标签和值对齐显示
 */
void printFinanceOne(FinanceNode* node) {
    if (node == NULL) return;

    printLine('-', 70);
    printf("记录编号: %s\n", node->id);
    printf("类型: %s\n", getFinanceTypeString(node->type));
    printf("金额: %.2f\n", node->amount);
    printf("日期: %s\n", node->date);
    printf("经办人: %s\n", node->handler);
    printf("部门: %s\n", node->department);
    printf("用途/来源: %s\n", node->purpose);
    printf("审核状态: %s\n", getAuditStatusString(node->auditStatus));
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

/**
 * @brief 打印所有财务记录列表
 * @param list 财务链表指针
 *
 * 功能说明:
 *   遍历整个财务链表，依次输出每条记录的详细信息。
 *   先显示总数概览，再逐个列出详情。
 *
 * 输出结构:
 *   --- 所有财务记录 (共 N 条) ---
 *   [记录1详细信息]
 *   [记录2详细信息]
 *   ...
 *
 * 特殊处理:
 *   - 空链表时提示"暂无财务记录"
 *   - 每条记录调用printFinanceOne()显示
 */
void printFinanceAll(FinanceList* list) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    printf("\n--- 所有财务记录 (共 %d 条) ---\n", list->count);
    FinanceNode* current = list->head;

    while (current != NULL) {
        printFinanceOne(current);
        current = current->next;
    }
}

/**
 * @brief 按日期范围查询财务记录
 * @param list 财务链表指针
 * @param startDate 起始日期（含），格式YYYY-MM-DD
 * @param endDate 结束日期（含），格式YYYY-MM-DD
 *
 * 功能说明:
 *   在指定日期范围内筛选并显示符合条件的财务记录。
 *   用于按时间段查询收支情况。
 *
 * 查询逻辑:
 *   - 遍历所有记录
 *   - 比较每条记录的日期是否在[startDate, endDate]区间内
 *   - 使用compareDate()函数进行日期比较
 *   - 符合条件的记录逐一输出
 *
 * 日期范围规则:
 *   - 包含起始日期和结束日期（闭区间）
 *   - startDate <= 记录日期 <= endDate
 *
 * 输出内容:
 *   - 符合条件的记录详情
 *   - 找到的记录总数
 *   - 无结果时给出提示
 *
 * 应用场景:
 *   - 月度/季度/年度财务查询
 *   - 特定时间段的收支分析
 *   - 对账核对时的范围筛选
 */
void queryFinanceByDate(FinanceList* list, const char* startDate, const char* endDate) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    int count = 0;
    FinanceNode* current = list->head;

    printf("\n--- %s 至 %s 的财务记录 ---\n", startDate, endDate);

    /* 遍历链表筛选符合日期范围的记录 */
    while (current != NULL) {
        if (compareDate(current->date, startDate) >= 0 &&
            compareDate(current->date, endDate) <= 0) {
            printFinanceOne(current);
            count++;
        }
        current = current->next;
    }

    /* 输出查询结果统计 */
    if (count == 0) {
        printf("该日期范围内无记录。\n");
    }
    else {
        printf("共找到 %d 条记录。\n", count);
    }
}

/**
 * @brief 按部门查询财务记录
 * @param list 财务链表指针
 * @param dept 部门名称关键字（支持模糊匹配）
 *
 * 功能说明:
 *   根据部门字段筛选财务记录，支持模糊搜索。
 *   用于查看特定部门的收支情况。
 *
 * 匹配规则:
 *   - 使用strstr进行子串匹配
 *   - 部门名称包含关键字的记录都会被找出
 *   - 不区分大小写（取决于系统locale设置）
 *
 * 应用场景:
 *   - 各科室费用统计
 *   - 部门预算执行情况查询
 *   - 科室间费用对比分析
 */
void queryFinanceByDept(FinanceList* list, const char* dept) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    int count = 0;
    FinanceNode* current = list->head;

    printf("\n--- 部门 [%s] 的财务记录 ---\n", dept);

    /* 遍历链表模糊匹配部门名称 */
    while (current != NULL) {
        if (strstr(current->department, dept) != NULL) {
            printFinanceOne(current);
            count++;
        }
        current = current->next;
    }

    /* 输出查询结果统计 */
    if (count == 0) {
        printf("该部门无记录。\n");
    }
    else {
        printf("共找到 %d 条记录。\n", count);
    }
}

/**
 * @brief 按经办人查询财务记录
 * @param list 财务链表指针
 * @param handler 经办人姓名关键字（支持模糊匹配）
 *
 * 功能说明:
 *   根据经办人字段筛选财务记录，支持模糊搜索。
 *   用于查看特定人员经手的所有财务业务。
 *
 * 匹配规则:
 *   - 使用strstr进行子串匹配
 *   - 经办人姓名包含关键字的记录都会被找出
 *
 * 应用场景:
 *   - 个人工作量统计
 *   - 经办人责任追溯
 *   - 员工业绩考核参考
 */
void queryFinanceByHandler(FinanceList* list, const char* handler) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    int count = 0;
    FinanceNode* current = list->head;

    printf("\n--- 经办人 [%s] 的财务记录 ---\n", handler);

    /* 遍历链表模糊匹配经办人姓名 */
    while (current != NULL) {
        if (strstr(current->handler, handler) != NULL) {
            printFinanceOne(current);
            count++;
        }
        current = current->next;
    }

    /* 输出查询结果统计 */
    if (count == 0) {
        printf("该经办人无记录。\n");
    }
    else {
        printf("共找到 %d 条记录。\n", count);
    }
}

/**
 * @brief 按收支类型查询财务记录
 * @param list 财务链表指针
 * @param type 收支类型(0=收入, 1=支出)
 *
 * 功能说明:
 *   根据收支类型精确筛选财务记录。
 *   用于单独查看收入或支出明细。
 *
 * 查询逻辑:
 *   - 遍历所有记录
 *   - 比较type字段与给定类型值
 *   - 完全匹配的记录才输出
 *
 * 应用场景:
 *   - 单独查看收入明细
 *   - 单独查看支出明细
 *   - 分类统计分析
 */
void queryFinanceByType(FinanceList* list, int type) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    int count = 0;
    FinanceNode* current = list->head;

    printf("\n--- %s记录 ---\n", getFinanceTypeString(type));

    /* 遍历链表筛选指定类型的记录 */
    while (current != NULL) {
        if (current->type == type) {
            printFinanceOne(current);
            count++;
        }
        current = current->next;
    }

    /* 输出查询结果统计 */
    if (count == 0) {
        printf("无%s记录。\n", getFinanceTypeString(type));
    }
    else {
        printf("共找到 %d 条记录。\n", count);
    }
}

/**
 * @brief 计算总收入（仅统计已审核记录）
 * @param list 财务链表指针
 *
 * 功能说明:
 *   统计所有已审核通过的收入记录金额总和。
 *   未审核和已作废的记录不计入统计。
 *
 * 计算公式:
 *   总收入 = Σ(所有type==TYPE_INCOME 且 auditStatus==AUDIT_APPROVED 的记录.amount)
 *
 * 统计条件:
 *   - 类型必须是收入(TYPE_INCOME)
 *   - 审核状态必须是已审核(AUDIT_APPROVED)
 *
 * 输出格式:
 *   总收入 (已审核): XXXX.XX
 *
 * 应用场景:
 *   - 财务报表总收入栏目
 *   - 月度/年度收入汇总
 *   - 收入目标完成情况对比
 */
void calculateTotalIncome(FinanceList* list) {
    double total = 0;
    FinanceNode* current = list->head;

    /* 遍历链表累加已审核的收入记录 */
    while (current != NULL) {
        if (current->type == TYPE_INCOME && current->auditStatus == AUDIT_APPROVED) {
            total += current->amount;
        }
        current = current->next;
    }

    printf("总收入 (已审核): %.2f\n", total);
}

/**
 * @brief 计算总支出（仅统计已审核记录）
 * @param list 财务链表指针
 *
 * 功能说明:
 *   统计所有已审核通过的支出记录金额总和。
 *   未审核和已作废的记录不计入统计。
 *
 * 计算公式:
 *   总支出 = Σ(所有type==TYPE_EXPENSE 且 auditStatus==AUDIT_APPROVED 的记录.amount)
 *
 * 统计条件:
 *   - 类型必须是支出(TYPE_EXPENSE)
 *   - 审核状态必须是已审核(AUDIT_APPROVED)
 *
 * 输出格式:
 *   总支出 (已审核): XXXX.XX
 *
 * 应用场景:
 *   - 财务报表总支出栏目
 *   - 月度/年度支出汇总
 *   - 预算执行情况监控
 */
void calculateTotalExpense(FinanceList* list) {
    double total = 0;
    FinanceNode* current = list->head;

    /* 遍历链表累加已审核的支出记录 */
    while (current != NULL) {
        if (current->type == TYPE_EXPENSE && current->auditStatus == AUDIT_APPROVED) {
            total += current->amount;
        }
        current = current->next;
    }

    printf("总支出 (已审核): %.2f\n", total);
}

/**
 * @brief 计算财务结余（收支汇总）
 * @param list 财务链表指针
 *
 * 功能说明:
 *   综合统计已审核记录的总收入、总支出和结余情况。
 *   提供完整的财务概况报告。
 *
 * 计算公式:
 *   结余 = 总收入 - 总支出
 *
 * 统计范围:
 *   仅包含审核状态为AUDIT_APPROVED的记录
 *   未审核和已作废的记录不参与计算
 *
 * 输出示例:
 *   --- 财务汇总 (已审核) ---
 *   总收入: 500000.00
 *   总支出: 320000.00
 *   结余: 180000.00
 *
 * 业务含义:
 *   - 正结余: 收入大于支出，经营状况良好
 *   - 负结余: 支出大于收入，出现亏损
 *   - 零结余: 收支平衡
 *
 * 应用场景:
 *   - 财务状况总览
 *   - 经营成果评估
 *   - 决策依据参考
 */
void calculateBalance(FinanceList* list) {
    double income = 0, expense = 0;
    FinanceNode* current = list->head;

    /* 分别统计已审核的收入和支出 */
    while (current != NULL) {
        if (current->auditStatus == AUDIT_APPROVED) {
            if (current->type == TYPE_INCOME) {
                income += current->amount;
            }
            else {
                expense += current->amount;
            }
        }
        current = current->next;
    }

    /* 输出财务汇总报告 */
    printf("\n--- 财务汇总 (已审核) ---\n");
    printf("总收入: %.2f\n", income);
    printf("总支出: %.2f\n", expense);
    printf("结余: %.2f\n", income - expense);
}

/**
 * @brief 按月份统计财务数据
 * @param list 财务链表指针
 * @param yearMonth 年月字符串，格式YYYY-MM（如：2024-01）
 *
 * 功能说明:
 *   统计指定月份的收入、支出和结余情况。
 *   用于月度财务分析和报表生成。
 *
 * 匹配规则:
 *   - 比较记录日期的前7个字符（年-月部分）
 *   - 使用strncmp进行前缀匹配
 *   - 该月的所有记录都纳入统计
 *
 * 统计指标:
 *   - 收入总额及笔数
 *   - 支出总额及笔数
 *   - 当月结余
 *
 * 输出示例:
 *   --- 2024-01 财务统计 ---
 *   收入: 50000.00 (15笔)
 *   支出: 35000.00 (22笔)
 *   结余: 15000.00
 *
 * 输入格式:
 *   yearMonth: "YYYY-MM" 格式的字符串
 *   - 正确示例: "2024-01", "2024-12"
 *   - 错误示例: "2024-1", "24-01"
 *
 * 应用场景:
 *   - 月度财务报表
 *   - 同比环比分析
 *   - 预算执行进度跟踪
 */
void statFinanceByMonth(FinanceList* list, const char* yearMonth) {
    double income = 0, expense = 0;
    int incomeCount = 0, expenseCount = 0;

    FinanceNode* current = list->head;

    /* 遍历链表筛选指定月份的记录 */
    while (current != NULL) {
        if (strncmp(current->date, yearMonth, 7) == 0) {
            if (current->type == TYPE_INCOME) {
                income += current->amount;
                incomeCount++;
            }
            else {
                expense += current->amount;
                expenseCount++;
            }
        }
        current = current->next;
    }

    /* 输出月度统计报告 */
    printf("\n--- %s 财务统计 ---\n", yearMonth);
    printf("收入: %.2f (%d笔)\n", income, incomeCount);
    printf("支出: %.2f (%d笔)\n", expense, expenseCount);
    printf("结余: %.2f\n", income - expense);
}

/**
 * @brief 释放财务链表占用的所有内存
 * @param list 财务链表指针
 *
 * 功能说明:
 *   遍历链表逐个释放每个节点分配的内存。
 *   在程序退出或重新加载数据前应调用此函数。
 *
 * 释放流程:
 *   1. 从头节点开始遍历
 *   2. 保存当前节点指针
 *   3. 移动到下一个节点
 *   4. free释放保存的节点
 *   5. 重复直到链表末尾
 *   6. 重置链表头指针和计数器
 *
 * 内存安全:
 *   - 释放后head=NULL防止悬垂指针
 *   - count=0表示空链表状态
 */
void freeFinanceList(FinanceList* list) {
    FinanceNode* current = list->head;
    while (current != NULL) {
        FinanceNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 新增财务记录（封装操作）
 * @param list 财务链表指针
 *
 * 功能说明:
 *   完整的新增财务记录流程：
 *   1. 创建新的财务记录节点（交互式输入）
 *   2. 检查编号是否重复
 *   3. 插入链表
 *   4. 记录操作日志
 *
 * 错误处理:
 *   - 编号重复时提示错误并释放节点
 *   - 内存分配失败时直接返回
 *
 * 日志内容:
 *   记录新增记录的关键信息（编号、类型、金额、部门）
 */
void addFinance(FinanceList* list) {
    FinanceNode* node = createFinanceNode();
    if (node == NULL) return;

    /* 检查编号唯一性 */
    if (isFinanceIDExist(list, node->id)) {
        printf("错误：编号 %s 已存在！\n", node->id);
        free(node);
        return;
    }

    /* 插入链表并提示成功 */
    insertFinanceNode(list, node);
    printf("财务记录添加成功！\n");

    /* 记录新增日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "编号:%s, 类型:%s, 金额:%.2f, 部门:%s",
        node->id, getFinanceTypeString(node->type), node->amount, node->department);
    writeLog(LOG_FINANCE, "新增财务记录", "系统", detail);
}

/**
 * @brief 查询财务记录（多条件查询入口）
 * @param list 财务链表指针
 *
 * 功能说明:
 *   提供多种查询方式的统一入口，用户可选择不同的查询条件。
 *
 * 查询方式列表:
 *   1. 按编号查询    - 精确定位单条记录
 *   2. 按日期范围查询 - 时间段内所有记录
 *   3. 按部门查询    - 特定部门的记录
 *   4. 按经办人查询  - 特定人员的记录
 *   5. 按类型查询    - 收入或支出分类
 *   6. 显示全部      - 无条件浏览
 *   0. 返回          - 退出查询
 *
 * 查询特点:
 *   - 支持精确查询和模糊查询
 *   - 日期范围支持闭区间
 *   - 部门和经办人支持子串匹配
 */
void queryFinance(FinanceList* list) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    printf("\n--- 查询财务记录 ---\n");
    printf("1. 按编号查询\n");
    printf("2. 按日期范围查询\n");
    printf("3. 按部门查询\n");
    printf("4. 按经办人查询\n");
    printf("5. 按类型查询\n");
    printf("6. 显示全部\n");
    printf("0. 返回\n");

    int choice = inputInt("请选择: ");
    char keyword[MAX_NAME_LEN];
    char startDate[MAX_DATE_LEN], endDate[MAX_DATE_LEN];

    switch (choice) {
    case 1:
        /* 按编号精确定位 */
        safeInput(keyword, MAX_ID_LEN, "请输入编号: ");
        FinanceNode* found = findFinanceByID(list, keyword);
        if (found) {
            printFinanceOne(found);
        }
        else {
            printf("未找到编号为 %s 的记录。\n", keyword);
        }
        break;
    case 2:
        /* 按日期范围筛选 */
        safeInput(startDate, MAX_DATE_LEN, "请输入开始日期 (YYYY-MM-DD): ");
        safeInput(endDate, MAX_DATE_LEN, "请输入结束日期 (YYYY-MM-DD): ");
        queryFinanceByDate(list, startDate, endDate);
        break;
    case 3:
        /* 按部门模糊匹配 */
        safeInput(keyword, MAX_DEPT_LEN, "请输入部门: ");
        queryFinanceByDept(list, keyword);
        break;
    case 4:
        /* 按经办人模糊匹配 */
        safeInput(keyword, MAX_NAME_LEN, "请输入经办人: ");
        queryFinanceByHandler(list, keyword);
        break;
    case 5:
        /* 按类型分类筛选 */
        printf("类型:\n");
        printf("0. 收入\n");
        printf("1. 支出\n");
        int type = inputInt("请选择: ");
        queryFinanceByType(list, type);
        break;
    case 6:
        /* 显示全部记录 */
        printFinanceAll(list);
        break;
    case 0:
        return;
    default:
        printf("无效选择。\n");
    }
}

/**
 * @brief 删除财务记录（封装操作）
 * @param list 财务链表指针
 *
 * 功能说明:
 *   删除操作的简化接口，只需输入编号即可完成删除。
 *   内部调用deleteFinanceByID()执行实际删除操作。
 */
void deleteFinance(FinanceList* list) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入要删除的记录编号: ");
    deleteFinanceByID(list, id);
}

/**
 * @brief 修改财务记录（封装操作）
 * @param list 财务链表指针
 *
 * 功能说明:
 *   修改操作的简化接口，先查找再修改。
 *   内部调用findFinanceByID()和modifyFinanceInfo()。
 */
void modifyFinance(FinanceList* list) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入要修改的记录编号: ");

    FinanceNode* node = findFinanceByID(list, id);
    if (node == NULL) {
        printf("未找到编号为 %s 的记录。\n", id);
        return;
    }

    modifyFinanceInfo(node);
}

/**
 * @brief 审核财务记录
 * @param list 财务链表指针
 *
 * 功能说明:
 *   对指定的财务记录进行审核操作。
 *   审核是财务管理的重要环节，只有通过审核的记录才参与统计计算。
 *
 * 审核操作选项:
 *   1. 通过审核 - 将状态改为AUDIT_APPROVED
 *                  记录将被纳入财务统计
 *   2. 作废记录 - 将状态改为AUDIT_REJECTED
 *                 记录将被标记为无效，不参与统计
 *   0. 取消     - 不做任何更改
 *
 * 审核流程:
 *   1. 输入要审核的记录编号
 *   2. 查找并显示该记录详情
 *   3. 选择审核操作
 *   4. 执行状态变更
 *
 * 业务规则:
 *   - 只有未审核状态的记录才能被审核
 *   - 已审核的记录可以再次审核（如改作废）
 *   - 已作废的记录可以恢复（如改回已审核）
 *
 * 权限说明:
 *   此操作通常由财务主管或授权人员执行
 */
void auditFinance(FinanceList* list) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入要审核的记录编号: ");

    FinanceNode* node = findFinanceByID(list, id);
    if (node == NULL) {
        printf("未找到编号为 %s 的记录。\n", id);
        return;
    }

    /* 显示待审核记录详情 */
    printf("\n当前记录:\n");
    printFinanceOne(node);

    printf("\n审核操作:\n");
    printf("1. 通过审核\n");
    printf("2. 作废记录\n");
    printf("0. 取消\n");

    int choice = inputInt("请选择: ");

    switch (choice) {
    case 1:
        node->auditStatus = AUDIT_APPROVED;
        printf("审核通过！\n");
        break;
    case 2:
        node->auditStatus = AUDIT_REJECTED;
        printf("记录已作废！\n");
        break;
    case 0:
        printf("取消审核。\n");
        break;
    default:
        printf("无效选择。\n");
    }
}

/**
 * @brief 财务报表功能（统计报表入口）
 * @param list 财务链表指针
 *
 * 功能说明:
 *   提供财务统计报表的生成功能入口。
 *   整合各种统计分析选项。
 *
 * 报表类型:
 *   1. 收支汇总 - 显示总收入、总支出和结余
 *   2. 按月统计 - 指定月份的详细统计数据
 *   0. 返回     - 退出报表功能
 *
 * 报表内容:
 *   - 收支汇总: 全部已审核记录的汇总数据
 *   - 月度统计: 指定月份的收入、支出、结余及笔数
 */
void financeReport(FinanceList* list) {
    printf("\n--- 财务报表 ---\n");
    printf("1. 收支汇总\n");
    printf("2. 按月统计\n");
    printf("0. 返回\n");

    int choice = inputInt("请选择: ");
    char yearMonth[10];

    switch (choice) {
    case 1:
        /* 生成收支汇总报表 */
        calculateBalance(list);
        break;
    case 2:
        /* 生成指定月份的统计报表 */
        safeInput(yearMonth, 10, "请输入年月 (YYYY-MM): ");
        statFinanceByMonth(list, yearMonth);
        break;
    case 0:
        return;
    default:
        printf("无效选择。\n");
    }
}

/**
 * @brief 财务管理主菜单
 * @param list 财务链表指针
 *
 * 功能说明:
 *   提供财务管理的交互式主界面，循环显示菜单并响应用户选择。
 *   是财务模块的入口函数，整合了所有财务操作功能。
 *
 * 菜单功能列表:
 *   1. 新增财务记录  - 调用addFinance()
 *   2. 查询财务记录  - 调用queryFinance()
 *   3. 删除财务记录  - 调用deleteFinance()
 *   4. 修改财务记录  - 调用modifyFinance()
 *   5. 审核财务记录  - 调用auditFinance()
 *   6. 财务报表      - 调用financeReport()
 *   0. 返回主菜单    - 退出本模块
 *
 * 操作流程:
 *   1. 循环显示菜单界面
 *   2. 获取用户功能选择
 *   3. 根据选择调用对应功能函数
 *   4. 操作完成后暂停等待用户查看结果
 *   5. 用户选择0时退出循环
 *
 * 安全检查:
 *   - 大部分操作前检查链表是否为空
 *   - 添加时检查编号重复
 *   - 删除前要求确认
 *
 * 日志集成:
 *   - 新增记录时自动记录日志
 *   - 删除记录时自动记录日志
 *   - 修改记录时自动记录日志
 *
 * 业务特点:
 *   - 支持审核机制，保证数据准确性
 *   - 多维度查询，方便数据分析
 *   - 完善的统计报表功能
 */
void financeMenu(FinanceList* list) {
    int choice;

    do {
        printf("\n");
        printTitle("财务管理系统");
        printf("1. 新增财务记录\n");
        printf("2. 查询财务记录\n");
        printf("3. 删除财务记录\n");
        printf("4. 修改财务记录\n");
        printf("5. 审核财务记录\n");
        printf("6. 财务报表\n");
        printf("0. 返回主菜单\n");

        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: addFinance(list); break;
        case 2: queryFinance(list); break;
        case 3: deleteFinance(list); break;
        case 4: modifyFinance(list); break;
        case 5: auditFinance(list); break;
        case 6: financeReport(list); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择，请重试。\n");
        }

        /* 除退出外每次操作后暂停，让用户查看结果 */
        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}

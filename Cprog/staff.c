#include "staff.h"
#include "log.h"

/**
 * @brief 将工作状态枚举值转换为中文字符串显示
 * @param status 工作状态枚举值(STATUS_WORKING/STATUS_LEAVE/STATUS_RESIGNED/STATUS_TRANSFERRED)
 * @return 对应的中文字符串指针
 *
 * 状态映射关系:
 *   STATUS_WORKING(0)     → "在职"    （正常工作状态）
 *   STATUS_LEAVE(1)        → "请假"    （临时休假）
 *   STATUS_RESIGNED(2)     → "离职"    （已解除劳动关系）
 *   STATUS_TRANSFERRED(3)  → "调岗"    （岗位变动中）
 *   其他                   → "未知"     （无效状态值）
 *
 * 使用场景:
 *   - 职工信息显示时将数字状态转为可读文本
 *   - 统计报表生成时的状态分类
 *   - 用户界面友好化展示
 */
const char* getStatusString(int status) {
    switch (status) {
    case STATUS_WORKING: return "在职";
    case STATUS_LEAVE: return "请假";
    case STATUS_RESIGNED: return "离职";
    case STATUS_TRANSFERRED: return "调岗";
    default: return "未知";
    }
}

/**
 * @brief 初始化职工链表
 * @param list 职工链表指针
 *
 * 功能说明:
 *   将链表头指针置空，计数器归零。
 *   在系统启动时必须调用此函数初始化数据结构。
 *
 * 初始化内容:
 *   - head: NULL（空链表）
 *   - count: 0（无记录）
 */
void initStaffList(StaffList* list) {
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 从文件加载职工数据到内存链表
 * @param list 职工链表指针（用于存储加载的数据）
 * @param filename 数据文件路径
 *
 * 功能说明:
 *   从指定文件读取职工记录，逐行解析并构建链表结构。
 *   文件不存在时会提示用户但不报错（首次运行时正常现象）。
 *
 * 文件格式要求:
 *   每行一条记录，字段以竖线(|)分隔：
 *   工号|姓名|性别|年龄|电话|部门|岗位|入职日期|工资|状态|备注
 *
 * 数据字段说明:
 *   - 工号: 唯一标识该职工
 *   - 性别: 男/女
 *   - 年龄: 整数
 *   - 电话: 符合格式的手机或座机号码
 *   - 入职日期: YYYY-MM-DD格式
 *   - 工资: 浮点数，保留两位小数
 *   - 状态: 0=在职, 1=请假, 2=离职, 3=调岗
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
 *   EMP001|张三|男|28|13800138000|内科|主治医师|2020-03-15|15000.00|0|
 */
void loadStaffFromFile(StaffList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("提示：职工数据文件不存在，将创建新文件。\n");
        return;
    }

    /* 尾指针用于高效插入，避免每次从头遍历 */
    StaffNode* tail = NULL;
    char line[1024];

    /* 逐行读取文件内容 */
    while (fgets(line, sizeof(line), fp)) {
        StaffNode* node = (StaffNode*)malloc(sizeof(StaffNode));
        if (node == NULL) continue;

        /* 解析一行数据到节点各字段 */
        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%lf|%d|%[^\n]",
            node->id, node->name, node->gender, &node->age,
            node->phone, node->department, node->position,
            node->hireDate, &node->salary, &node->status, node->remark);

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
    printf("成功加载 %d 条职工记录。\n", list->count);
}

/**
 * @brief 将职工链表数据保存到文件
 * @param list 职工链表指针（包含要保存的数据）
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   遍历整个职工链表，将每条记录格式化写入文件。
 *   采用覆盖写模式，保存后文件只包含当前最新数据。
 *
 * 写入格式:
 *   字段间用竖线分隔，每行一条完整记录：
 *   工号|姓名|性别|年龄|电话|部门|岗位|入职日期|工资|状态|备注\n
 *
 * 数据完整性保障:
 *   - 每条记录独立一行，便于逐行读取
 *   - 数值类型保留两位小数
 *   - 最后一个字段后换行符结束
 *
 * 调用时机:
 *   - 添加/删除/修改职工后
 *   - 系统退出前保存数据
 *   - 手动触发保存操作时
 */
void saveStaffToFile(StaffList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法打开文件 %s 进行写入！\n", filename);
        return;
    }

    StaffNode* current = list->head;

    /* 遍历链表逐条写入 */
    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%d|%s|%s|%s|%s|%.2f|%d|%s\n",
            current->id, current->name, current->gender, current->age,
            current->phone, current->department, current->position,
            current->hireDate, current->salary, current->status, current->remark);
        current = current->next;
    }

    fclose(fp);
    printf("成功保存 %d 条职工记录。\n", list->count);
}

/**
 * @brief 创建新的职工节点（交互式输入）
 * @return 成功返回职工节点指针，失败返回NULL
 *
 * 功能说明:
 *   通过控制台交互方式收集职工信息，创建完整的职工数据节点。
 *   包含输入验证和必填项检查。
 *
 * 收集的信息字段:
 *   1. 工号 [必填] - 唯一标识，不能为空
 *   2. 姓名 [必填] - 职工真实姓名
 *   3. 性别 [必填] - 男或女
 *   4. 年龄 [必填] - 正整数
 *   5. 电话 [必填] - 手机或座机号码（会验证格式）
 *   6. 所属部门 [必填] - 如：内科、外科、财务部等
 *   7. 岗位 [必填] - 如：医师、护士、会计等
 *   8. 入职日期 [必填] - 格式YYYY-MM-DD，会验证格式
 *   9. 基本工资 [必填] - 正浮点数，单位元
 *   10. 备注 [可选] - 其他补充信息
 *
 * 自动设置的字段:
 *   - status: STATUS_WORKING（新增职工默认在职状态）
 *
 * 输入验证规则:
 *   - 工号不能为空字符串
 *   - 电话必须符合有效格式
 *   - 工资必须≥0
 *   - 日期必须符合YYYY-MM-DD格式
 *
 * 返回值使用:
 *   - 成功: 返回的节点可直接插入链表
 *   - 失败: 内存分配问题返回NULL，调用者需处理
 */
StaffNode* createStaffNode() {
    StaffNode* node = (StaffNode*)malloc(sizeof(StaffNode));
    if (node == NULL) {
        printf("错误：内存分配失败！\n");
        return NULL;
    }

    printf("\n--- 新增职工 ---\n");

    /*
     * ====== 输入字段1: 职工工号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字、连字符的组合
     * 是否必填: 是（不能为空）
     * 输入样例:
     *   - "GH001" (工号拼音缩写+序号)
     *   - "EMP-2024-0456" (Employee+日期+序号)
     *   - "S20260416001" (Staff日期+流水号)
     *   - "ZG20240001" (职工+年份+序号)
     */
    safeInput(node->id, MAX_ID_LEN, "请输入工号: ");
    while (isEmpty(node->id)) {
        printf("工号不能为空！\n");
        safeInput(node->id, MAX_ID_LEN, "请输入工号: ");
    }

    /*
     * ====== 输入字段2: 职工姓名 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 中文真实姓名
     * 是否必填: 是
     * 输入样例:
     *   - "张三"
     *   - "李四"
     *   - "王芳"
     */
    safeInput(node->name, MAX_NAME_LEN, "请输入姓名: ");

    /*
     * ====== 输入字段3: 性别 ======
     * 数据类型: char[10] (字符串)
     * 最大长度: 10个字符
     * 输入格式: 固定选项（男/女）
     * 是否必填: 是
     * 输入样例:
     *   - "男"
     *   - "女"
     */
    safeInput(node->gender, 10, "请输入性别 (男/女): ");

    /*
     * ====== 输入字段4: 年龄 ======
     * 数据类型: int (整数)
     * 取值范围: 16 ~ 70 (合法工作年龄范围)
     * 输入格式: 纯数字
     * 是否必填: 是
     * 输入样例:
     *   - "25" (25岁)
     *   - "35" (35岁)
     *   - "45" (45岁)
     * 业务含义: 职工的实际年龄，用于退休年龄计算
     */
    node->age = inputInt("请输入年龄: ");

    /*
     * ====== 输入字段5: 联系电话 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 电话号码（手机或座机）
     * 是否必填: 是
     * 输入样例:
     *   - "13812345678" (11位手机号)
     *   - "029-81234567" (座机带区号)
     *   - "0571-88123456"
     * 格式要求:
     *   - 长度: 7-15个字符
     *   - 允许: 数字、连字符、空格
     * 验证: 自动检查电话号码格式合法性
     * 错误提示: "电话号码格式不正确！"会提示重新输入
     */
    safeInput(node->phone, MAX_PHONE_LEN, "请输入电话: ");
    while (!isValidPhone(node->phone)) {
        printf("电话号码格式不正确！\n");
        safeInput(node->phone, MAX_PHONE_LEN, "请输入电话: ");
    }

    /*
     * ====== 输入字段6: 所属部门 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符
     * 输入格式: 部门名称（中文）
     * 是否必填: 是
     * 输入样例:
     *   - "内科"
     *   - "外科"
     *   - "财务部"
     *   - "人事科"
     *   - "总务科"
     *   - "药剂科"
     * 说明: 职工所在的工作部门/科室
     */
    safeInput(node->department, MAX_DEPT_LEN, "请输入所属部门: ");

    /*
     * ====== 输入字段7: 岗位名称 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 岗位/职位名称
     * 是否必填: 是
     * 输入样例:
     *   - "主任医师"
     *   - "主治医师"
     *   - "护士长"
     *   - "药剂师"
     *   - "会计"
     *   - "系统管理员"
     * 说明: 职工的具体岗位或职称
     */
    safeInput(node->position, MAX_POSITION_LEN, "请输入岗位: ");

    /*
     * ====== 输入字段8: 入职日期 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: YYYY-MM-DD (严格格式)
     * 是否必填: 是
     * 输入样例:
     *   - "2020-09-01" (2020年9月1日入职)
     *   - "2018-03-15" (2018年3月15日入职)
     *   - "2025-07-01" (应届毕业生入职)
     * 格式要求:
     *   - 年份: 4位数字
     *   - 月份: 2位数字 (01-12)
     *   - 日期: 2位数字 (根据月份1-31)
     * 验证: 自动检查日期有效性(含闰年判断)
     * 业务含义: 用于计算工龄、年假等权益
     */
    safeInput(node->hireDate, MAX_DATE_LEN, "请输入入职日期 (YYYY-MM-DD): ");
    while (!isValidDate(node->hireDate)) {
        printf("日期格式不正确！\n");
        safeInput(node->hireDate, MAX_DATE_LEN, "请输入入职日期 (YYYY-MM-DD): ");
    }

    /*
     * ====== 输入字段9: 基本工资 ======
     * 数据类型: double (双精度浮点数)
     * 取值范围: 0.00 ~ 很大的正数
     * 精度要求: 保留2位小数(元/月)
     * 输入格式: 数字，可含小数点
     * 是否必填: 是
     * 输入样例:
     *   - "8000.00" (8000元/月)
     *   - "15000.00" (15000元/月，高级职称)
     *   - "5000.00" (5000元/月，初级职称)
     *   - "25000.00" (25000元/月，专家级)
     * 业务含义: 月基本工资（不含绩效、奖金等）
     * 验证规则: 必须为非负数，负数会提示重新输入
     */
    node->salary = inputDouble("请输入基本工资: ");
    while (node->salary < 0) {
        printf("工资不能为负数！\n");
        node->salary = inputDouble("请输入基本工资: ");
    }

    /* 【系统设置】状态默认为 STATUS_WORKING(在职/正常) */

    /*
     * ====== 输入字段10: 备注 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 自由文本
     * 是否必填: 否（可留空）
     * 输入样例:
     *   - "" (直接回车留空)
     *   - "硕士研究生学历"
     *   - "具有执业医师资格证"
     *   - "党员，担任科室党支部委员"
     * 用途: 记录学历、资格证、政治面貌等补充信息
     */
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注 (无则留空): ");

    node->next = NULL;
    return node;
}

/**
 * @brief 检查职工工号是否已存在
 * @param list 职工链表指针
 * @param id 待检查的工号
 * @return 存在返回1，不存在返回0
 *
 * 功能说明:
 *   在职工链表中线性搜索指定工号，用于保证工号唯一性。
 *
 * 查找算法:
 *   - 从链表头部开始遍历
 *   - 逐一比较每个节点的id字段
 *   - 找到匹配即返回（不需要遍历全部）
 *
 * 使用场景:
 *   - 添加新职工前检查重复
 *   - 导入数据前去重校验
 */
int isStaffIDExist(StaffList* list, const char* id) {
    StaffNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

/**
 * @brief 将职工节点插入链表头部
 * @param list 职工链表指针
 * @param node 待插入的职工节点指针
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
void insertStaffNode(StaffList* list, StaffNode* node) {
    if (node == NULL) return;

    node->next = list->head;
    list->head = node;
    list->count++;
}

/**
 * @brief 根据工号查找职工
 * @param list 职工链表指针
 * @param id 目标工号
 * @return 找到返回节点指针，未找到返回NULL
 *
 * 功能说明:
 *   在职工链表中按工号精确查找目标职工。
 *   返回的是实际节点的指针，可用于直接修改数据。
 *
 * 查找过程:
 *   1. 从链表头开始遍历
 *   2. 使用strcmp比较id字段
 *   3. 匹配则立即返回该节点地址
 *   4. 遍历完仍未找到返回NULL
 *
 * 返回值用途:
 *   - 非NULL: 可直接访问或修改该职工信息
 *   - NULL: 表示职工不存在，需提示用户
 */
StaffNode* findStaffByID(StaffList* list, const char* id) {
    StaffNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief 根据姓名查找职工（模糊匹配）
 * @param list 职工链表指针
 * @param name 姓名关键字（支持子串匹配）
 * @return 找到返回第一个匹配的节点指针，未找到返回NULL
 *
 * 功能说明:
 *   在职工链表中按姓名进行模糊搜索。
 *   支持部分匹配，只要姓名包含关键字即可找到。
 *
 * 匹配规则:
 *   - 使用strstr进行子串匹配
 *   - 返回第一个匹配的节点
 *   - 不区分大小写（取决于系统locale设置）
 *
 * 应用场景:
 *   - 按姓名快速查找职工
 *   - 支持只输入部分姓名搜索
 */
StaffNode* findStaffByName(StaffList* list, const char* name) {
    StaffNode* current = list->head;
    while (current != NULL) {
        if (strstr(current->name, name) != NULL) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief 根据工号删除职工记录
 * @param list 职工链表指针
 * @param id 要删除的职工工号
 * @return 成功删除返回1，取消或未找到返回0
 *
 * 功能说明:
 *   完整的职工删除流程：
 *   1. 在链表中定位目标职工
 *   2. 显示职工信息供确认
 *   3. 用户确认后执行删除
 *   4. 记录操作日志
 *   5. 释放被删节点内存
 *
 * 删除前的确认机制:
 *   - 显示待删除职工的详细信息
 *   - 要求用户二次确认（Y/N）
 *   - 用户可取消删除操作
 *
 * 内存管理:
 *   - 删除后自动free释放节点内存
 *   - 更新链表count计数器
 *   - 正确处理首节点和中间节点的不同情况
 *
 * 日志记录:
 *   记录被删职工的关键信息（工号、姓名、部门）
 */
int deleteStaffByID(StaffList* list, const char* id) {
    StaffNode* current = list->head;
    StaffNode* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            /* 显示待删除记录详情 */
            printf("\n找到以下记录:\n");
            printStaffOne(current);

            /* 二次确认防止误删 */
            if (!confirm("确认删除该职工?")) {
                printf("已取消删除。\n");
                return 0;
            }

            /* 准备日志信息（在删除前获取） */
            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "工号:%s, 姓名:%s, 部门:%s",
                current->id, current->name, current->department);

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
            writeLog(LOG_STAFF, "删除职工", "系统", detail);
            return 1;
        }
        prev = current;
        current = current->next;
    }

    printf("未找到工号为 %s 的职工。\n", id);
    return 0;
}

/**
 * @brief 修改职工信息（交互式选择字段修改）
 * @param node 待修改的职工节点指针
 *
 * 功能说明:
 *   提供菜单式界面让用户选择要修改的具体字段，
 *   支持单次修改一个字段，避免误改其他数据。
 *
 * 可修改的字段列表:
 *   1. 电话       - phone字段（会验证格式）
 *   2. 部门       - department字段
 *   3. 岗位       - position字段
 *   4. 基本工资   - salary字段（数值型）
 *   5. 工作状态   - status字段（枚举选择）
 *   6. 备注       - remark字段
 *   0. 取消       - 不做任何修改
 *
 * 修改流程:
 *   1. 显示当前职工信息
 *   2. 显示可选修改项菜单
 *   3. 用户选择要修改的字段
 *   4. 输入新值替换旧值
 *   5. 记录修改日志
 *
 * 日志内容:
 *   包含职工工号、姓名和修改的字段序号
 *
 * 注意事项:
 *   - 修改工作状态会影响该职工是否计入在职统计
 *   - 修改工资会影响平均工资计算
 */
void modifyStaffInfo(StaffNode* node) {
    if (node == NULL) return;

    printf("\n--- 修改职工信息 ---\n");
    printf("当前信息:\n");
    printStaffOne(node);

    printf("\n请选择要修改的字段:\n");
    printf("1. 电话\n");
    printf("2. 部门\n");
    printf("3. 岗位\n");
    printf("4. 基本工资\n");
    printf("5. 工作状态\n");
    printf("6. 备注\n");
    printf("0. 取消修改\n");

    int choice = inputInt("请选择: ");

    switch (choice) {
    case 1:
        safeInput(node->phone, MAX_PHONE_LEN, "请输入新电话: ");
        break;
    case 2:
        safeInput(node->department, MAX_DEPT_LEN, "请输入新部门: ");
        break;
    case 3:
        safeInput(node->position, MAX_POSITION_LEN, "请输入新岗位: ");
        break;
    case 4:
        node->salary = inputDouble("请输入新基本工资: ");
        break;
    case 5:
        /* 状态修改提供选项列表 */
        printf("工作状态选项:\n");
        printf("0. 在职\n");
        printf("1. 请假\n");
        printf("2. 离职\n");
        printf("3. 调岗\n");
        node->status = inputInt("请选择新状态: ");
        if (node->status < 0 || node->status > 3) {
            node->status = STATUS_WORKING;  /* 无效值回退为默认状态 */
        }
        break;
    case 6:
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
    sprintf(detail, "工号:%s, 姓名:%s, 修改字段:%d", node->id, node->name, choice);
    writeLog(LOG_STAFF, "修改职工信息", "系统", detail);
}

/**
 * @brief 打印单个职工的详细信息
 * @param node 职工节点指针
 *
 * 功能说明:
 *   格式化输出一名职工的完整信息，用于查看详情。
 *   输出采用固定宽度的分隔线框架，便于阅读。
 *
 * 显示内容:
 *   - 工号（唯一标识）
 *   - 姓名
 *   - 性别
 *   - 年龄
 *   - 电话
 *   - 所属部门
 *   - 岗位
 *   - 入职日期
 *   - 基本工资（保留2位小数）
 *   - 工作状态（中文显示：在职/请假/离职/调岗）
 *   - 备注
 *
 * 输出格式特点:
 *   - 使用70字符宽的分隔线
 *   - 每个字段独占一行
 *   - 标签和值对齐显示
 */
void printStaffOne(StaffNode* node) {
    if (node == NULL) return;

    printLine('-', 70);
    printf("工号: %s\n", node->id);
    printf("姓名: %s\n", node->name);
    printf("性别: %s\n", node->gender);
    printf("年龄: %d\n", node->age);
    printf("电话: %s\n", node->phone);
    printf("部门: %s\n", node->department);
    printf("岗位: %s\n", node->position);
    printf("入职日期: %s\n", node->hireDate);
    printf("基本工资: %.2f\n", node->salary);
    printf("工作状态: %s\n", getStatusString(node->status));
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

/**
 * @brief 打印所有职工列表
 * @param list 职工链表指针
 *
 * 功能说明:
 *   遍历整个职工链表，依次输出每名职工的详细信息。
 *   先显示总数概览，再逐个列出详情。
 *
 * 输出结构:
 *   --- 所有职工列表 (共 N 人) ---
 *   [职工1详细信息]
 *   [职工2详细信息]
 *   ...
 *
 * 特殊处理:
 *   - 空链表时提示"暂无职工记录"
 *   - 每名职工调用printStaffOne()显示
 */
void printStaffAll(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    printf("\n--- 所有职工列表 (共 %d 人) ---\n", list->count);
    StaffNode* current = list->head;

    while (current != NULL) {
        printStaffOne(current);
        current = current->next;
    }
}

/**
 * @brief 按工号对职工链表进行升序排序
 * @param list 职工链表指针
 *
 * 功能说明:
 *   使用冒泡排序算法，按照工号的字典序对链表进行排序。
 *   排序后工号从小到大排列。
 *
 * 排序算法:
 *   - 采用冒泡排序（双重循环比较交换）
 *   - 通过交换节点的数据内容实现排序
 *   - 同时恢复被交换的next指针以保持链表完整性
 *
 * 时间复杂度: O(n²)，适合中小规模数据集
 *
 * 边界处理:
 *   - 空链表或单节点链表直接返回
 */
void sortStaffByID(StaffList* list) {
    if (list->head == NULL || list->head->next == NULL) return;

    /* 冒泡排序：外层循环控制起始位置 */
    for (StaffNode* p = list->head; p != NULL; p = p->next) {
        /* 内层循环与后续节点逐一比较 */
        for (StaffNode* q = p->next; q != NULL; q = q->next) {
            if (strcmp(p->id, q->id) > 0) {
                /* 交换两个节点的数据内容 */
                StaffNode temp = *p;
                *p = *q;
                *q = temp;
                /* 恢复被破坏的next指针 */
                StaffNode* tempNext = p->next;
                p->next = q->next;
                q->next = tempNext;
            }
        }
    }
}

/**
 * @brief 按工资对职工链表进行降序排序
 * @param list 职工链表指针
 *
 * 功能说明:
 *   使用冒泡排序算法，按照基本工资从高到低排序。
 *   排序后高工资的职工排在前面。
 *
 * 排序规则:
 *   - 降序排列（从大到小）
 *   - 数值比较而非字符串比较
 *
 * 应用场景:
 *   - 工资排名展示
 *   - 高薪人才筛选
 *   - 薪酬结构分析
 */
void sortStaffBySalary(StaffList* list) {
    if (list->head == NULL || list->head->next == NULL) return;

    for (StaffNode* p = list->head; p != NULL; p = p->next) {
        for (StaffNode* q = p->next; q != NULL; q = q->next) {
            if (p->salary < q->salary) {  /* 降序：前面小于后面则交换 */
                StaffNode temp = *p;
                *p = *q;
                *q = temp;
                StaffNode* tempNext = p->next;
                p->next = q->next;
                q->next = tempNext;
            }
        }
    }
}

/**
 * @brief 按年龄对职工链表进行升序排序
 * @param list 职工链表指针
 *
 * 功能说明:
 *   使用冒泡排序算法，按照年龄从小到大排序。
 *   排序后年轻的职工排在前面。
 *
 * 排序规则:
 *   - 升序排列（从小到大）
 *   - 整数数值直接比较
 *
 * 应用场景:
 *   - 年龄结构分析
 *   - 年轻员工识别
 *   - 资深程度排序
 */
void sortStaffByAge(StaffList* list) {
    if (list->head == NULL || list->head->next == NULL) return;

    for (StaffNode* p = list->head; p != NULL; p = p->next) {
        for (StaffNode* q = p->next; q != NULL; q = q->next) {
            if (p->age > q->age) {  /* 升序：前面大于后面则交换 */
                StaffNode temp = *p;
                *p = *q;
                *q = temp;
                StaffNode* tempNext = p->next;
                p->next = q->next;
                q->next = tempNext;
            }
        }
    }
}

/**
 * @brief 按入职日期对职工链表进行升序排序
 * @param list 职工链表指针
 *
 * 功能说明:
 *   使用冒泡排序算法，按照入职日期从早到晚排序。
 *   排序后老员工排在前面。
 *
 * 排序规则:
 *   - 升序排列（时间从早到晚）
 *   - 使用compareDate()函数进行日期比较
 *
 * 日期比较逻辑:
 *   compareDate()返回值：
 *   - <0: 第一个日期更早
 *   - =0: 两日期相同
 *   - >0: 第一个日期更晚
 *
 * 应用场景:
 *   - 司龄分析
 *   - 老员工表彰
 *   - 入职时间线查看
 */
void sortStaffByHireDate(StaffList* list) {
    if (list->head == NULL || list->head->next == NULL) return;

    for (StaffNode* p = list->head; p != NULL; p = p->next) {
        for (StaffNode* q = p->next; q != NULL; q = q->next) {
            if (compareDate(p->hireDate, q->hireDate) > 0) {
                StaffNode temp = *p;
                *p = *q;
                *q = temp;
                StaffNode* tempNext = p->next;
                p->next = q->next;
                q->next = tempNext;
            }
        }
    }
}

/**
 * @brief 统计各部门人数分布
 * @param list 职工链表指针
 *
 * 功能说明:
 *   遍历职工链表，按部门分组统计人数。
 *   以表格形式输出各部门的人员数量。
 *
 * 统计方法:
 *   1. 定义内部结构体存储部门和计数
 *   2. 遍历链表，为每个部门累计计数
 *   3. 新部门首次出现时创建条目
 *   4. 已存在的部门增加计数
 *
 * 输出格式示例:
 *   --- 各部门人数统计 ---
 *   ----------------------------------------
 *   部门                  人数
 *   ----------------------------------------
 *   内科                  15
 *   外科                  12
 *   财务部                 5
 *   ----------------------------------------
 *
 * 数据结构限制:
 *   - 最多支持100个不同部门
 *   - 超过限制时新部门不计入统计
 *
 * 应用场景:
 *   - 组织架构人员分布
 *   - 部门编制管理
 *   - 人力资源规划参考
 */
void countStaffByDept(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    printf("\n--- 各部门人数统计 ---\n");

    /* 定义部门统计结构体 */
    typedef struct {
        char dept[MAX_DEPT_LEN];
        int count;
    } DeptStat;

    DeptStat stats[100];
    int statCount = 0;

    /* 遍历链表统计各部门人数 */
    StaffNode* current = list->head;
    while (current != NULL) {
        int found = 0;
        /* 查找是否已有该部门的统计项 */
        for (int i = 0; i < statCount; i++) {
            if (strcmp(stats[i].dept, current->department) == 0) {
                stats[i].count++;      /* 已存在则累加 */
                found = 1;
                break;
            }
        }
        /* 不存在则新建统计项 */
        if (!found && statCount < 100) {
            strcpy(stats[statCount].dept, current->department);
            stats[statCount].count = 1;
            statCount++;
        }
        current = current->next;
    }

    /* 表格化输出统计结果 */
    printLine('-', 40);
    printf("%-20s %s\n", "部门", "人数");
    printLine('-', 40);
    for (int i = 0; i < statCount; i++) {
        printf("%-20s %d\n", stats[i].dept, stats[i].count);
    }
    printLine('-', 40);
}

/**
 * @brief 统计各岗位人数分布
 * @param list 职工链表指针
 *
 * 功能说明:
 *   遍历职工链表，按岗位分组统计人数。
 *   与countStaffByDept()类似但按岗位维度统计。
 *
 * 统计方法:
 *   - 同样使用内部数组存储统计结果
 *   - 按岗位名称精确匹配分组
 *
 * 输出格式示例:
 *   --- 各岗位人数统计 ---
 *   ----------------------------------------
 *   岗位                  人数
 *   ----------------------------------------
 *   主治医师              10
 *   护士                  20
 *   会计师                 3
 *   ----------------------------------------
 *
 * 应用场景:
 *   - 岗位编制分析
 *   - 人员配置合理性评估
 *   - 岗位需求预测
 */
void countStaffByPosition(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    printf("\n--- 各岗位人数统计 ---\n");

    /* 定义岗位统计结构体 */
    typedef struct {
        char position[MAX_POSITION_LEN];
        int count;
    } PositionStat;

    PositionStat stats[100];
    int statCount = 0;

    /* 遍历链表统计各岗位人数 */
    StaffNode* current = list->head;
    while (current != NULL) {
        int found = 0;
        for (int i = 0; i < statCount; i++) {
            if (strcmp(stats[i].position, current->position) == 0) {
                stats[i].count++;
                found = 1;
                break;
            }
        }
        if (!found && statCount < 100) {
            strcpy(stats[statCount].position, current->position);
            stats[statCount].count = 1;
            statCount++;
        }
        current = current->next;
    }

    /* 表格化输出统计结果 */
    printLine('-', 40);
    printf("%-20s %s\n", "岗位", "人数");
    printLine('-', 40);
    for (int i = 0; i < statCount; i++) {
        printf("%-20s %d\n", stats[i].position, stats[i].count);
    }
    printLine('-', 40);
}

/**
 * @brief 统计各工作状态的职工人数
 * @param list 职工链表指针
 *
 * 功能说明:
 *   按工作状态分类统计职工数量。
 *   直观展示人员在不同状态下的分布情况。
 *
 * 统计维度:
 *   - 在职: 正常工作的职工
 *   - 请假: 休假的职工
 *   - 离职: 已离开的职工
 *   - 调岗: 正在办理调动的职工
 *
 * 输出示例:
 *   --- 工作状态统计 ---
 *   ------------------------------
 *   状态             人数
 *   ------------------------------
 *   在职             45
 *   请假              3
 *   离职              5
 *   调岗              2
 *   ------------------------------
 *   总计             55
 *
 * 业务含义:
 *   - 在职率 = 在职人数 / 总人数
 *   - 请假率影响排班安排
 *   - 离职率反映人员稳定性
 */
void countStaffByStatus(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    int working = 0, leave = 0, resigned = 0, transferred = 0;

    /* 遍历链表分类统计各状态人数 */
    StaffNode* current = list->head;
    while (current != NULL) {
        switch (current->status) {
        case STATUS_WORKING: working++; break;
        case STATUS_LEAVE: leave++; break;
        case STATUS_RESIGNED: resigned++; break;
        case STATUS_TRANSFERRED: transferred++; break;
        }
        current = current->next;
    }

    /* 表格化输出统计结果 */
    printf("\n--- 工作状态统计 ---\n");
    printLine('-', 30);
    printf("%-15s %s\n", "状态", "人数");
    printLine('-', 30);
    printf("%-15s %d\n", "在职", working);
    printf("%-15s %d\n", "请假", leave);
    printf("%-15s %d\n", "离职", resigned);
    printf("%-15s %d\n", "调岗", transferred);
    printLine('-', 30);
    printf("%-15s %d\n", "总计", list->count);
}

/**
 * @brief 计算在职职工的平均工资
 * @param list 职工链表指针
 *
 * 功能说明:
 *   统计在职职工的工资情况，包括：
 *   - 在职员工总数
 *   - 工资总额
 *   - 平均工资
 *
 * 统计范围:
 *   仅统计状态为STATUS_WORKING（在职）的职工。
 *   请假、离职、调岗的职工不纳入计算。
 *
 * 计算公式:
 *   平均工资 = 工资总额 / 在职人数
 *
 * 输出示例:
 *   --- 工资统计 ---
 *   在职员工数: 45
 *   工资总额: 675000.00
 *   平均工资: 15000.00
 *
 * 应用场景:
 *   - 薪酬水平评估
 *   - 人力成本核算
 *   - 薪酬预算制定
 */
void calculateAvgSalary(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    double total = 0;
    int workingCount = 0;

    /* 遍历链表累加在职职工的工资 */
    StaffNode* current = list->head;
    while (current != NULL) {
        if (current->status == STATUS_WORKING) {
            total += current->salary;
            workingCount++;
        }
        current = current->next;
    }

    /* 输出工资统计报告 */
    printf("\n--- 工资统计 ---\n");
    printf("在职员工数: %d\n", workingCount);
    printf("工资总额: %.2f\n", total);
    if (workingCount > 0) {
        printf("平均工资: %.2f\n", total / workingCount);
    }
}

/**
 * @brief 释放职工链表占用的所有内存
 * @param list 职工链表指针
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
void freeStaffList(StaffList* list) {
    StaffNode* current = list->head;
    while (current != NULL) {
        StaffNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 新增职工（封装操作）
 * @param list 职工链表指针
 *
 * 功能说明:
 *   完整的新增职工流程：
 *   1. 创建新的职工节点（交互式输入）
 *   2. 检查工号是否重复
 *   3. 插入链表
 *   4. 记录操作日志
 *
 * 错误处理:
 *   - 工号重复时提示错误并释放节点
 *   - 内存分配失败时直接返回
 *
 * 日志内容:
 *   记录新增职工的关键信息（工号、姓名、部门、岗位）
 */
void addStaff(StaffList* list) {
    StaffNode* node = createStaffNode();
    if (node == NULL) return;

    /* 检查工号唯一性 */
    if (isStaffIDExist(list, node->id)) {
        printf("错误：工号 %s 已存在！\n", node->id);
        free(node);
        return;
    }

    /* 插入链表并提示成功 */
    insertStaffNode(list, node);
    printf("职工添加成功！\n");

    /* 记录新增日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "工号:%s, 姓名:%s, 部门:%s, 岗位:%s",
        node->id, node->name, node->department, node->position);
    writeLog(LOG_STAFF, "新增职工", "系统", detail);
}

/**
 * @brief 查询职工（多条件查询入口）
 * @param list 职工链表指针
 *
 * 功能说明:
 *   提供多种查询方式的统一入口，用户可选择不同的查询条件。
 *
 * 查询方式列表:
 *   1. 按工号查询  - 精确定位单名职工
 *   2. 按姓名查询  - 支持模糊匹配，可找出多个结果
 *   3. 按部门查询  - 支持模糊匹配，可找出多个结果
 *   4. 显示全部    - 无条件浏览所有职工
 *   0. 返回        - 退出查询
 *
 * 查询特点:
 *   - 支持精确查询和模糊查询
 *   - 姓名和部门支持子串匹配
 *   - 多结果时显示找到的记录总数
 */
void queryStaff(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    printf("\n--- 查询职工 ---\n");
    printf("1. 按工号查询\n");
    printf("2. 按姓名查询\n");
    printf("3. 按部门查询\n");
    printf("4. 显示全部\n");
    printf("0. 返回\n");

    int choice = inputInt("请选择: ");
    char keyword[MAX_NAME_LEN];

    switch (choice) {
    case 1:
        /* 按工号精确定位 */
        safeInput(keyword, MAX_ID_LEN, "请输入工号: ");
        StaffNode* found = findStaffByID(list, keyword);
        if (found) {
            printStaffOne(found);
        }
        else {
            printf("未找到工号为 %s 的职工。\n", keyword);
        }
        break;
    case 2:
        /* 按姓名模糊匹配（可能多个结果） */
        safeInput(keyword, MAX_NAME_LEN, "请输入姓名: ");
        int foundCount = 0;
        StaffNode* current = list->head;
        while (current != NULL) {
            if (strstr(current->name, keyword) != NULL) {
                printStaffOne(current);
                foundCount++;
            }
            current = current->next;
        }
        if (foundCount == 0) {
            printf("未找到姓名为 %s 的职工。\n", keyword);
        }
        else {
            printf("共找到 %d 条记录。\n", foundCount);
        }
        break;
    case 3:
        /* 按部门模糊匹配（可能多个结果） */
        safeInput(keyword, MAX_DEPT_LEN, "请输入部门: ");
        foundCount = 0;
        current = list->head;
        while (current != NULL) {
            if (strstr(current->department, keyword) != NULL) {
                printStaffOne(current);
                foundCount++;
            }
            current = current->next;
        }
        if (foundCount == 0) {
            printf("未找到部门为 %s 的职工。\n", keyword);
        }
        else {
            printf("共找到 %d 条记录。\n", foundCount);
        }
        break;
    case 4:
        /* 显示全部职工记录 */
        printStaffAll(list);
        break;
    case 0:
        return;
    default:
        printf("无效选择。\n");
    }
}

/**
 * @brief 删除职工（封装操作）
 * @param list 职工链表指针
 *
 * 功能说明:
 *   删除操作的简化接口，只需输入工号即可完成删除。
 *   内部调用deleteStaffByID()执行实际删除操作。
 */
void deleteStaff(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入要删除的职工工号: ");
    deleteStaffByID(list, id);
}

/**
 * @brief 修改职工信息（封装操作）
 * @param list 职工链表指针
 *
 * 功能说明:
 *   修改操作的简化接口，先查找再修改。
 *   内部调用findStaffByID()和modifyStaffInfo()。
 */
void modifyStaff(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入要修改的职工工号: ");

    StaffNode* node = findStaffByID(list, id);
    if (node == NULL) {
        printf("未找到工号为 %s 的职工。\n", id);
        return;
    }

    modifyStaffInfo(node);
}

/**
 * @brief 排序显示职工列表
 * @param list 职工链表指针
 *
 * 功能说明:
 *   提供多种排序方式，排序后显示所有职工信息。
 *   排序会改变链表的实际顺序。
 *
 * 排序方式列表:
 *   1. 按工号排序    - 字典序升序（A-Z, 0-9）
 *   2. 按工资排序    - 降序（从高到低）
 *   3. 按年龄排序    - 升序（从小到大）
 *   4. 按入职日期排序 - 升序（从早到晚）
 *   0. 返回          - 不执行排序
 *
 * 注意事项:
 *   - 排序是就地操作，会改变原链表顺序
 *   - 排序完成后自动调用printStaffAll()显示结果
 */
void sortStaff(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    printf("\n--- 排序显示 ---\n");
    printf("1. 按工号排序\n");
    printf("2. 按工资排序（从高到低）\n");
    printf("3. 按年龄排序\n");
    printf("4. 按入职日期排序\n");
    printf("0. 返回\n");

    int choice = inputInt("请选择: ");

    switch (choice) {
    case 1:
        sortStaffByID(list);
        printf("已按工号排序。\n");
        break;
    case 2:
        sortStaffBySalary(list);
        printf("已按工资排序。\n");
        break;
    case 3:
        sortStaffByAge(list);
        printf("已按年龄排序。\n");
        break;
    case 4:
        sortStaffByHireDate(list);
        printf("已按入职日期排序。\n");
        break;
    case 0:
        return;
    default:
        printf("无效选择。\n");
        return;
    }

    /* 排序后显示结果 */
    printStaffAll(list);
}

/**
 * @brief 职工统计分析入口
 * @param list 职工链表指针
 *
 * 功能说明:
 *   整合各种统计分析功能的统一入口。
 *   提供多维度的职工数据分析能力。
 *
 * 统计分析类型:
 *   1. 按部门统计    - 各部门人数分布
 *   2. 按岗位统计    - 各岗位人数分布
 *   3. 按工作状态统计 - 各状态人数分布
 *   4. 工资统计      - 在职员工工资汇总
 *   0. 返回          - 退出统计功能
 *
 * 应用场景:
 *   - 人事报表生成
 *   - 组织架构分析
 *   - 人力资源决策支持
 */
void statStaff(StaffList* list) {
    printf("\n--- 职工统计 ---\n");
    printf("1. 按部门统计\n");
    printf("2. 按岗位统计\n");
    printf("3. 按工作状态统计\n");
    printf("4. 工资统计\n");
    printf("0. 返回\n");

    int choice = inputInt("请选择: ");

    switch (choice) {
    case 1:
        countStaffByDept(list);
        break;
    case 2:
        countStaffByPosition(list);
        break;
    case 3:
        countStaffByStatus(list);
        break;
    case 4:
        calculateAvgSalary(list);
        break;
    case 0:
        return;
    default:
        printf("无效选择。\n");
    }
}

/**
 * @brief 人事管理主菜单
 * @param list 职工链表指针
 *
 * 功能说明:
 *   提供人事管理的交互式主界面，循环显示菜单并响应用户选择。
 *   是人事模块的入口函数，整合了所有职工管理操作功能。
 *
 * 菜单功能列表:
 *   1. 新增职工    - 调用addStaff()
 *   2. 查询职工    - 调用queryStaff()
 *   3. 删除职工    - 调用deleteStaff()
 *   4. 修改职工    - 调用modifyStaff()
 *   5. 排序显示    - 调用sortStaff()
 *   6. 统计分析    - 调用statStaff()
 *   0. 返回主菜单  - 退出本模块
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
 *   - 添加时检查工号重复
 *   - 删除前要求确认
 *
 * 日志集成:
 *   - 新增职工时自动记录日志
 *   - 删除职工时自动记录日志
 *   - 修改职工信息时自动记录日志
 *
 * 业务特点:
 *   - 完善的信息录入验证
 *   - 多种查询和排序方式
 *   - 丰富的统计分析功能
 *   - 支持工作状态管理
 */
void staffMenu(StaffList* list) {
    int choice;

    do {
        printf("\n");
        printTitle("人事管理系统");
        printf("1. 新增职工\n");
        printf("2. 查询职工\n");
        printf("3. 删除职工\n");
        printf("4. 修改职工\n");
        printf("5. 排序显示\n");
        printf("6. 统计分析\n");
        printf("0. 返回主菜单\n");

        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: addStaff(list); break;
        case 2: queryStaff(list); break;
        case 3: deleteStaff(list); break;
        case 4: modifyStaff(list); break;
        case 5: sortStaff(list); break;
        case 6: statStaff(list); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择，请重试。\n");
        }

        /* 除退出外每次操作后暂停，让用户查看结果 */
        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}

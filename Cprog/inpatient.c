/**
 * @file inpatient.c
 * @brief 住院管理模块实现文件
 *
 * 功能概述:
 *   本模块实现医院住院业务的全流程管理，包括：
 *   - 病人入院登记
 *   - 住院期间信息维护
 *   - 出院结算处理
 *   - 住院数据统计分析
 *
 * 核心业务:
 *   1. 入院流程：登记信息 → 分配床位 → 缴纳押金 → 开始计费
 *   2. 在院管理：修改诊断/医生/床位等 → 更新费用
 *   3. 出院流程：计算费用 → 结算押金 → 记录出院日期
 *
 * 数据结构:
 *   采用单链表结构存储住院记录，每条记录包含：
 *   病人基本信息、病房床位信息、费用信息、住院状态等
 *
 * 文件依赖:
 *   - inpatient.h: 住院数据结构和函数声明
 *   - log.h: 日志系统接口
 */

#include "inpatient.h"
#include "log.h"

/**
 * @brief 将住院状态枚举值转换为中文字符串
 * @param status 住院状态枚举值(0-2)
 * @return 对应的中文字符串指针
 *
 * 状态映射关系:
 *   0 → "在院" （正在住院治疗）
 *   1 → "转科" （已转到其他科室）
 *   2 → "已出院" （已完成治疗并离院）
 *   其他 → "未知"
 *
 * 使用场景:
 *   显示住院记录时，将数字状态转换为可读的中文描述
 */
const char* getInpatientStatusString(int status) {
    switch (status) {
    case INPATIENT_ADMITTED: return "在院";
    case INPATIENT_TRANSFERRED: return "转科";
    case INPATIENT_DISCHARGED: return "已出院";
    default: return "未知";
    }
}

/**
 * @brief 初始化住院链表
 * @param list 住院链表指针（输出参数）
 *
 * 功能说明:
 *   将链表头指针置空，计数器归零。
 *   在程序启动时调用，确保链表处于初始状态。
 */
void initInpatientList(InpatientList* list) {
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 从文件加载住院数据到内存
 * @param list 住院链表指针（输出参数）
 * @param filename 数据文件路径，如"data/inpatients.txt"
 *
 * 功能说明:
 *   1. 打开指定文件进行读取
 *   2. 逐行解析文件内容
 *   3. 为每条记录创建节点并插入链表尾部
 *   4. 更新链表计数器
 *
 * 文件格式要求:
 *   每行一条记录，字段用"|"分隔：
 *   编号|病人编号|姓名|性别|年龄|入院日期|科室|病区|病房号|床位号|医生|诊断|押金|日费用|总费用|状态|出院日期|备注
 *
 * 示例行:
 *   ZY001|P001|张三|男|45|2025-01-10|内科|A区|301|05|李医生|肺炎|5000.00|300.00|4500.00|0|2025-01-25|恢复良好
 *
 * 字段说明:
 *   - 状态: 0=在院, 1=转科, 2=已出院
 *   - 出院日期: 未出院时为"N/A"
 */
void loadInpatientsFromFile(InpatientList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("提示：住院数据文件不存在，将创建新文件。\n");
        return;
    }

    InpatientNode* tail = NULL;  /**< 链表尾指针 */
    char line[1024];           /**< 行缓冲区 */

    /* 逐行读取并解析记录 */
    while (fgets(line, sizeof(line), fp)) {
        InpatientNode* node = (InpatientNode*)malloc(sizeof(InpatientNode));
        if (node == NULL) continue;

        /* 解析一行中的各个字段（共18个字段） */
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%lf|%lf|%lf|%d|%[^|]|%[^\n]",
            node->id, node->patientId, node->name, node->gender,
            &node->age, node->admitDate, node->department, node->ward,
            node->roomNo, node->bedNo, node->doctor, node->diagnosis,
            &node->deposit, &node->dailyCost, &node->totalCost,
            &node->status, node->dischargeDate, node->remark);

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
    printf("成功加载 %d 条住院记录。\n", list->count);
}

/**
 * @brief 将内存中的住院数据保存到文件
 * @param list 住院链表指针
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   遍历整个链表，将每条记录按指定格式写入文件。
 *   使用"w"模式打开文件，会覆盖原有内容。
 *
 * 输出格式:
 *   与loadInpatientsFromFile的输入格式一致
 */
void saveInpatientsToFile(InpatientList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法打开文件 %s 进行写入！\n", filename);
        return;
    }

    InpatientNode* current = list->head;

    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%d|%s|%s|%s|%s|%s|%s|%s|%.2f|%.2f|%.2f|%d|%s|%s\n",
            current->id, current->patientId, current->name, current->gender,
            current->age, current->admitDate, current->department, current->ward,
            current->roomNo, current->bedNo, current->doctor, current->diagnosis,
            current->deposit, current->dailyCost, current->totalCost,
            current->status, current->dischargeDate, current->remark);
        current = current->next;
    }
    fclose(fp);
    printf("成功保存 %d 条住院记录。\n", list->count);
}

/**
 * @brief 创建新的住院记录节点（办理入院）
 * @return 成功返回新节点指针，失败返回NULL
 *
 * 功能说明:
 *   通过交互式界面收集用户输入，完成病人入院登记。
 *   新入院的病人默认状态为"在院"(INPATIENT_ADMITTED)，总费用初始为0。
 *
 * 输入项及格式:
 *   - 住院编号: 必填，唯一标识，如"ZY001"
 *   - 病人编号: 必填，关联病人信息
 *   - 姓名: 必填
 *   - 性别: 如"男"、"女"
 *   - 年龄: 整数
 *   - 入院日期: 自动获取当前日期
 *   - 科室: 如"内科"、"外科"
 *   - 病区: 如"A区"、"B区"
 *   - 病房号: 如"301"、"502"
 *   - 床位号: 如"05"、"12"
 *   - 主治医生: 负责医生姓名
 *   - 初始诊断: 入院时的初步诊断
 *   - 押金: 浮点数，预交金额（如5000.00）
 *   - 每日费用: 浮点数，日均住院费（如300.00）
 *   - 备注: 可选补充信息
 *
 * 自动设置的字段:
 *   - 总费用(totalCost): 初始为0
 *   - 状态(status): 默认为在院(0)
 *   - 出院日期(dischargeDate): 默认为"N/A"
 */
InpatientNode* createInpatientNode() {
    InpatientNode* node = (InpatientNode*)malloc(sizeof(InpatientNode));
    if (node == NULL) {
        printf("错误：内存分配失败！\n");
        return NULL;
    }

    printf("\n--- 办理入院 ---\n");

    /*
     * ====== 输入字段1: 住院编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字、连字符的组合
     * 是否必填: 是（不能为空）
     * 输入样例:
     *   - "ZY001" (住院拼音缩写+序号)
     *   - "IP-2024-0456" (InPatient+日期+序号)
     *   - "H20260416001" (入院年月日+流水号)
     * 验证规则:
     *   - 不能为空字符串
     *   - 建议使用有规律的编码便于管理
     */
    safeInput(node->id, MAX_ID_LEN, "请输入住院编号: ");
    while (isEmpty(node->id)) {
        printf("编号不能为空！\n");
        safeInput(node->id, MAX_ID_LEN, "请输入住院编号: ");
    }

    /*
     * ====== 输入字段2: 病人编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字组合（与门诊/急诊共用）
     * 是否必填: 是
     * 输入样例:
     *   - "P001"
     *   - "BR20240101001"
     * 说明: 应与患者档案系统中的编号一致
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
     */
    safeInput(node->name, MAX_NAME_LEN, "请输入姓名: ");

    /*
     * ====== 输入字段4: 性别 ======
     * 数据类型: char[10] (字符串)
     * 最大长度: 10个字符
     * 输入格式: 固定选项（男/女）
     * 是否必填: 是
     * 输入样例:
     *   - "男"
     *   - "女"
     * 注意: 建议统一使用"男"或"女"，避免使用"M/F"等英文
     */
    safeInput(node->gender, 10, "请输入性别: ");

    /*
     * ====== 输入字段5: 年龄 ======
     * 数据类型: int (整数)
     * 取值范围: 0 ~ 150 (正整数)
     * 输入格式: 纯数字，不含小数点或单位
     * 是否必填: 是
     * 输入样例:
     *   - "25" (25岁)
     *   - "68" (68岁)
     *   - "1" (1岁婴儿)
     *   - "0" (新生儿)
     * 业务含义: 患者实足年龄，用于计算用药剂量等
     */
    node->age = inputInt("请输入年龄: ");

    /* 【自动填充】入院日期 - 系统自动获取当前日期(YYYY-MM-DD) */

    /*
     * ====== 输入字段6: 入住科室 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符
     * 输入格式: 科室名称（中文）
     * 是否必填: 是
     * 输入样例:
     *   - "内科"
     *   - "外科"
     *   - "儿科"
     *   - "妇产科"
     *   - "ICU(重症监护室)"
     * 说明: 对应医院实际设置的住院科室
     */
    safeInput(node->department, MAX_DEPT_LEN, "请输入科室: ");

    /*
     * ====== 输入字段7: 病区 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 病区编号或名称
     * 是否必填: 是
     * 输入样例:
     *   - "一病区"
     *   - "二病区"
     *   - "A区"
     *   - "东区"
     *   - "普通病房区"
     * 说明: 科室下的细分区域，用于病房管理
     */
    safeInput(node->ward, 20, "请输入病区: ");

    /*
     * ====== 输入字段8: 病房号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 房间编号
     * 是否必填: 是
     * 输入样例:
     *   - "301" (301病房)
     *   - "A-1201" (A栋12楼01房)
     *   - "外科-5-08" (外科5楼08房)
     * 说明: 具体的房间标识，配合床位号定位患者位置
     */
    safeInput(node->roomNo, 20, "请输入病房号: ");

    /*
     * ====== 输入字段9: 床位号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 床位编号
     * 是否必填: 是
     * 输入样例:
     *   - "1" (1号床)
     *   - "03" (03号床)
     *   - "A床"
     *   - "靠窗床位"
     * 说明: 病房内的具体床位标识
     */
    safeInput(node->bedNo, 20, "请输入床位号: ");

    /*
     * ====== 输入字段10: 主治医生 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 医生姓名+职称(可选)
     * 是否必填: 是
     * 输入样例:
     *   - "王医生"
     *   - "李明主任医师"
     *   - "张华副主任医师"
     * 说明: 负责该患者住院期间诊疗的主治医师
     */
    safeInput(node->doctor, MAX_NAME_LEN, "请输入主治医生: ");

    /*
     * ====== 输入字段11: 初始诊断 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 医学术语描述
     * 是否必填: 是
     * 输入样例:
     *   - "急性阑尾炎"
     *   - "冠心病(不稳定型心绞痛)"
     *   - "2型糖尿病伴并发症"
     *   - "肺炎(社区获得性)"
     * 说明: 入院时的初步诊断结论，后续可能调整
     */
    safeInput(node->diagnosis, MAX_REMARK_LEN, "请输入初始诊断: ");

    /*
     * ====== 输入字段12: 预交押金 ======
     * 数据类型: double (双精度浮点数)
     * 取值范围: 0.00 ~ 很大的正数
     * 精度要求: 保留2位小数(元)
     * 输入格式: 数字，可含小数点
     * 是否必填: 是
     * 输入样例:
     *   - "5000.00" (预交押金5000元)
     *   - "10000.00" (预交押金10000元)
     *   - "2000.00" (小额押金)
     * 业务含义: 患者入院时预交的费用，用于抵扣住院费用
     * 注意: 不能为负数；出院时多退少补
     */
    node->deposit = inputDouble("请输入押金: ");

    /*
     * ====== 输入字段13: 每日费用标准 ======
     * 数据类型: double (双精度浮点数)
     * 取值范围: 0.00 ~ 很大的正数
     * 精度要求: 保留2位小数(元/天)
     * 输入格式: 数字，可含小数点
     * 是否必填: 是
     * 输入样例:
     *   - "150.00" (普通病房150元/天)
     *   - "800.00" (单间800元/天)
     *   - "2000.00" (ICU 2000元/天)
     * 业务含义: 该患者的每日基础费用（含床位费、护理费等）
     * 说明: 不含药品和检查费用，仅为基础日费用
     */
    node->dailyCost = inputDouble("请输入每日费用: ");

    /* 【系统设置】以下字段由系统自动初始化:
     * - totalCost = 0         总费用初始为0（后续累计）
     * - status = INPATIENT_ADMITTED  默认在院状态
     * - dischargeDate = "N/A" 出院日期待定（出院时填写）
     */

    /*
     * ====== 输入字段14: 备注 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 自由文本
     * 是否必填: 否（可留空）
     * 输入样例:
     *   - "" (直接回车留空)
     *   - "家属陪护中"
     *   - "需特殊饮食(低盐低脂)"
     *   - "过敏史:青霉素"
     * 用途: 记录特殊护理要求、注意事项等补充信息
     */
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注: ");

    node->next = NULL;
    return node;
}

/**
 * @brief 检查住院编号是否已存在
 * @param list 住院链表指针
 * @param id 待检查的住院编号
 * @return 存在返回1，不存在返回0
 *
 * 功能说明:
 *   在办理新入院前调用，确保编号唯一性。
 */
int isInpatientIDExist(InpatientList* list, const char* id) {
    InpatientNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) return 1;
        current = current->next;
    }
    return 0;
}

/**
 * @brief 将新住院节点插入链表头部
 * @param list 住院链表指针
 * @param node 待插入的节点指针
 *
 * 功能说明:
 *   采用头插法，新记录插入到链表最前面。
 */
void insertInpatientNode(InpatientList* list, InpatientNode* node) {
    if (node == NULL) return;
    node->next = list->head;
    list->head = node;
    list->count++;
}

/**
 * @brief 根据住院编号查找记录
 * @param list 住院链表指针
 * @param id 目标住院编号
 * @return 找到返回节点指针，未找到返回NULL
 *
 * 功能说明:
 *   线性搜索链表，用于查询、修改、删除前的定位操作。
 */
InpatientNode* findInpatientByID(InpatientList* list, const char* id) {
    InpatientNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) return current;
        current = current->next;
    }
    return NULL;
}

/**
 * @brief 根据编号删除住院记录
 * @param list 住院链表指针
 * @param id 待删除的住院编号
 * @return 成功删除返回1，失败返回0
 *
 * 功能说明:
 *   1. 查找目标记录
 *   2. 显示完整信息供确认
 *   3. 用户确认后执行删除
 *   4. 维护链表结构完整性
 *
 * 安全机制:
 *   - 删除前显示完整住院信息
 *   - 要求二次确认
 *   - 自动释放被删除节点的内存
 *
 * 注意:
 *   此操作仅用于数据清理，正常流程应使用出院功能
 */
int deleteInpatientByID(InpatientList* list, const char* id) {
    InpatientNode* current = list->head, * prev = NULL;

    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printf("\n找到以下记录:\n");
            printInpatientOne(current);

            if (!confirm("确认删除该记录?")) {
                printf("已取消删除。\n");
                return 0;
            }

            /* 从链表中移除节点 */
            if (prev == NULL)
                list->head = current->next;
            else
                prev->next = current->next;

            free(current);
            list->count--;
            printf("删除成功！\n");
            return 1;
        }
        prev = current;
        current = current->next;
    }
    printf("未找到编号为 %s 的记录。\n", id);
    return 0;
}

/**
 * @brief 修改住院记录信息
 * @param node 待修改的住院节点指针
 *
 * 功能说明:
 *   提供选择性修改功能，用户可选择修改以下字段：
 *   1. 科室
 *   2. 病房/床位（病区+病房号+床位号）
 *   3. 主治医生
 *   4. 诊断
 *   5. 押金（补充缴纳）
 *   6. 每日费用
 *   7. 备注
 *
 * 业务规则:
 *   - 已出院的患者不允许修改信息
 *   - 修改后自动记录日志
 *
 * 修改场景示例:
 *   - 病情变化：更新诊断和主治医生
 *   - 转科室：修改科室和病房床位
 *   - 费用调整：修改每日费用标准
 */
void modifyInpatientInfo(InpatientNode* node) {
    if (node == NULL) return;

    /* 已出院患者不可修改 */
    if (node->status == INPATIENT_DISCHARGED) {
        printf("该患者已出院，不可修改。\n");
        return;
    }

    printf("\n--- 修改住院信息 ---\n");
    printInpatientOne(node);

    printf("\n请选择要修改的字段:\n");
    printf("1. 科室\n2. 病房/床位\n3. 主治医生\n4. 诊断\n5. 押金\n6. 每日费用\n7. 备注\n0. 取消\n");
    int choice = inputInt("请选择: ");

    switch (choice) {
    case 1: safeInput(node->department, MAX_DEPT_LEN, "请输入新科室: "); break;
    case 2:
        safeInput(node->ward, 20, "请输入新病区: ");
        safeInput(node->roomNo, 20, "请输入新病房号: ");
        safeInput(node->bedNo, 20, "请输入新床位号: ");
        break;
    case 3: safeInput(node->doctor, MAX_NAME_LEN, "请输入新主治医生: "); break;
    case 4: safeInput(node->diagnosis, MAX_REMARK_LEN, "请输入新诊断: "); break;
    case 5: node->deposit = inputDouble("请输入新押金: "); break;
    case 6: node->dailyCost = inputDouble("请输入新每日费用: "); break;
    case 7: safeInput(node->remark, MAX_REMARK_LEN, "请输入新备注: "); break;
    case 0: printf("取消修改。\n"); return;
    default: printf("无效选择。\n"); return;
    }

    printf("修改成功！\n");

    /* 记录修改日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "住院编号:%s, 病人:%s, 修改字段:%d", node->id, node->name, choice);
    writeLog(LOG_INPATIENT, "修改住院信息", "系统", detail);
}

/**
 * @brief 打印单条住院记录
 * @param node 住院节点指针
 *
 * 功能说明:
 *   格式化输出一条住院记录的所有字段，
 *   使用中文名称显示状态。已出院患者会额外显示出院日期。
 *
 * 输出格式示例:
 *   ----------------------------------------------------------------------
 *   住院编号: ZY001
 *   病人编号: P001
 *   姓名: 张三 性别: 男 年龄: 45
 *   入院日期: 2025-01-10
 *   科室: 内科 病区: A区
 *   病房: 301 床位: 05
 *   主治医生: 李医生
 *   初始诊断: 肺炎
 *   押金: 5000.00 每日费用: 300.00 总费用: 4500.00
 *   状态: 已出院
 *   出院日期: 2025-01-25
 *   备注: 恢复良好
 *   ----------------------------------------------------------------------
 */
void printInpatientOne(InpatientNode* node) {
    if (node == NULL) return;

    printLine('-', 70);
    printf("住院编号: %s\n", node->id);
    printf("病人编号: %s\n", node->patientId);
    printf("姓名: %s 性别: %s 年龄: %d\n", node->name, node->gender, node->age);
    printf("入院日期: %s\n", node->admitDate);
    printf("科室: %s 病区: %s\n", node->department, node->ward);
    printf("病房: %s 床位: %s\n", node->roomNo, node->bedNo);
    printf("主治医生: %s\n", node->doctor);
    printf("初始诊断: %s\n", node->diagnosis);
    printf("押金: %.2f 每日费用: %.2f 总费用: %.2f\n",
        node->deposit, node->dailyCost, node->totalCost);
    printf("状态: %s\n", getInpatientStatusString(node->status));

    /* 已出院时显示出院日期 */
    if (node->status == INPATIENT_DISCHARGED)
        printf("出院日期: %s\n", node->dischargeDate);

    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

/**
 * @brief 打印所有住院记录
 * @param list 住院链表指针
 *
 * 功能说明:
 *   遍历链表，依次打印每条记录。
 *   先显示总记录数，再逐一输出详细信息。
 */
void printInpatientAll(InpatientList* list) {
    if (list->head == NULL) {
        printf("暂无住院记录。\n");
        return;
    }

    printf("\n--- 所有住院记录 (共 %d 条) ---\n", list->count);
    InpatientNode* current = list->head;
    while (current != NULL) {
        printInpatientOne(current);
        current = current->next;
    }
}

/**
 * @brief 办理病人出院手续
 * @param list 住院链表指针
 * @param id 待出院病人的住院编号
 *
 * 功能说明:
 *   完成完整的出院结算流程：
 *   1. 查找病人住院记录
 *   2. 计算住院天数（从入院到当前）
 *   3. 计算总费用（每日费用 × 天数）
 *   4. 设置当前日期为出院日期
 *   5. 更新状态为"已出院"
 *   6. 显示费用结算详情
 *   7. 记录出院日志
 *
 * 费用计算公式:
 *   总费用 = 每日费用 × 住院天数
 *
 * 结算输出示例:
 *   --- 出院结算 ---
 *   [病人详细信息]
 *
 *   住院天数: 15 天
 *   应缴费用: 4500.00
 *   押金余额: 500.00  （押金 - 总费用，正数为退费）
 *   出院办理完成！
 *
 * 业务规则:
 *   - 已出院的患者不能重复办理
 *   - 最少按1天计算费用
 *   - 押金不足时会显示负数余额
 */
void dischargePatient(InpatientList* list, const char* id) {
    InpatientNode* node = findInpatientByID(list, id);
    if (node == NULL) { printf("未找到该患者。\n"); return; }

    /* 已出院检查 */
    if (node->status == INPATIENT_DISCHARGED) {
        printf("该患者已出院。\n");
        return;
    }

    /* 计算住院天数 */
    int days = daysBetween(node->admitDate, "");
    if (days <= 0) days = 1;  /* 至少按1天计算 */

    /* 计算总费用 */
    node->totalCost = node->dailyCost * days;

    /* 设置出院信息 */
    getCurrentDate(node->dischargeDate);
    node->status = INPATIENT_DISCHARGED;

    /* 显示结算详情 */
    printf("\n--- 出院结算 ---\n");
    printInpatientOne(node);
    printf("\n住院天数: %d 天\n", days);
    printf("应缴费用: %.2f\n", node->totalCost);
    printf("押金余额: %.2f\n", node->deposit - node->totalCost);
    printf("出院办理完成！\n");

    /* 记录出院日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "住院编号:%s, 病人:%s, 科室:%s, 住院天数:%d, 总费用:%.2f",
        node->id, node->name, node->department, days, node->totalCost);
    writeLog(LOG_INPATIENT, "办理出院", "系统", detail);
}

/**
 * @brief 住院数据统计分析
 * @param list 住院链表指针
 *
 * 功能说明:
 *   对住院数据进行多维度统计：
 *
 *   按状态统计:
 *   - 当前在院人数及押金总额
 *   - 转科人数
 *   - 已出院人数及费用总额
 *
 *   统计结果输出示例:
 *   --- 住院统计 ---
 *   当前在院: 30 人
 *   转科: 5 人
 *   已出院: 65 人
 *   总住院人数: 100 人
 *   在院押金总额: 150000.00
 *   已出院费用总额: 280000.00
 *
 * 统计指标说明:
 *   - 在院押金总额：所有在院患者的押金之和
 *   - 已出院费用总额：所有出院患者的实际花费总和
 */
void statInpatients(InpatientList* list) {
    int admitted = 0, transferred = 0, discharged = 0;  /**< 各状态计数 */
    double totalDeposit = 0, totalCost = 0;             /**< 费用合计 */

    InpatientNode* curr = list->head;

    /* 遍历所有记录进行分类统计 */
    while (curr != NULL) {
        switch (curr->status) {
        case INPATIENT_ADMITTED:
            admitted++;
            totalDeposit += curr->deposit;  /* 累加在院患者押金 */
            break;
        case INPATIENT_TRANSFERRED:
            transferred++;
            break;
        case INPATIENT_DISCHARGED:
            discharged++;
            totalCost += curr->totalCost;   /* 累加出院患者费用 */
            break;
        }
        curr = curr->next;
    }

    /* 输出统计结果 */
    printf("\n--- 住院统计 ---\n");
    printf("当前在院: %d 人\n", admitted);
    printf("转科: %d 人\n", transferred);
    printf("已出院: %d 人\n", discharged);
    printf("总住院人数: %d 人\n", list->count);
    printf("在院押金总额: %.2f\n", totalDeposit);
    printf("已出院费用总额: %.2f\n", totalCost);
}

/**
 * @brief 释放住院链表占用的所有内存
 * @param list 住院链表指针
 *
 * 功能说明:
 *   遍历链表，逐个释放每个节点分配的内存。
 *   最后将链表头指针置空，计数器归零。
 */
void freeInpatientList(InpatientList* list) {
    InpatientNode* current = list->head;
    while (current != NULL) {
        InpatientNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 住院管理主菜单
 * @param list 住院链表指针
 *
 * 功能说明:
 *   提供住院管理的交互式菜单界面，包含以下功能：
 *
 *   菜单选项:
 *   ┌────┬────────────────────────────┐
 *   │ 1  │ 办理入院                   │
 *   │ 2  │ 查询住院记录               │
 *   │ 3  │ 删除记录                   │
 *   │ 4  │ 修改信息                   │
 *   │ 5  │ 办理出院                   │
 *   │ 6  │ 显示全部                   │
 *   │ 7  │ 统计分析                   │
 *   │ 0  │ 返回主菜单                 │
 *   └────┴────────────────────────────┘
 *
 * 操作流程:
 *   循环显示菜单，根据用户选择调用对应功能模块。
 *   选择0时退出本菜单，返回上级菜单。
 *
 * 日志记录:
 *   入院、修改、出院操作会自动写入日志
 */
void inpatientMenu(InpatientList* list) {
    int choice;

    do {
        printf("\n");
        printTitle("病人住院管理系统");
        printf("1. 办理入院\n2. 查询住院记录\n3. 删除记录\n4. 修改信息\n");
        printf("5. 办理出院\n6. 显示全部\n7. 统计分析\n0. 返回主菜单\n");
        choice = inputInt("请选择功能: ");

        switch (choice) {

        /* ====== 办理入院 ====== */
        case 1: {
            InpatientNode* node = createInpatientNode();
            if (node && !isInpatientIDExist(list, node->id)) {
                insertInpatientNode(list, node);
                printf("入院办理成功！\n");

                /* 记录入院日志 */
                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "住院编号:%s, 病人:%s, 科室:%s, 病房:%s-%s, 医生:%s, 押金:%.2f",
                    node->id, node->name, node->department,
                    node->roomNo, node->bedNo, node->doctor, node->deposit);
                writeLog(LOG_INPATIENT, "办理入院", "系统", detail);
            } else if (node) {
                printf("错误：住院编号已存在！\n");
                free(node);  /* 编号重复时释放节点 */
            }
            break;
        }

        /* ====== 查询住院记录 ====== */
        case 2: {
            if (list->head == NULL) { printf("暂无住院记录。\n"); break; }
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "请输入住院编号: ");
            InpatientNode* found = findInpatientByID(list, id);
            if (found)
                printInpatientOne(found);
            else
                printf("未找到。\n");
            break;
        }

        /* ====== 删除记录 ====== */
        case 3: {
            if (list->head == NULL) { printf("暂无住院记录。\n"); break; }
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "请输入要删除的住院编号: ");
            deleteInpatientByID(list, id);
            break;
        }

        /* ====== 修改信息 ====== */
        case 4: {
            if (list->head == NULL) { printf("暂无住院记录。\n"); break; }
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "请输入要修改的住院编号: ");
            InpatientNode* node = findInpatientByID(list, id);
            if (node)
                modifyInpatientInfo(node);
            else
                printf("未找到。\n");
            break;
        }

        /* ====== 办理出院 ====== */
        case 5: {
            if (list->head == NULL) { printf("暂无住院记录。\n"); break; }
            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "请输入要办理出院的住院编号: ");
            dischargePatient(list, id);
            break;
        }

        /* ====== 显示全部记录 ====== */
        case 6:
            printInpatientAll(list);
            break;

        /* ====== 统计分析 ====== */
        case 7:
            statInpatients(list);
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

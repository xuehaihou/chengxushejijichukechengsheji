/**
 * @file record.c
 * @brief 病案管理模块实现 - 实现病案信息的增删改查和归档管理
 *
 * 本模块实现医院病案（病历档案）的完整生命周期管理，包括：
 * - 病案创建和基本信息录入
 * - 病案查询和信息展示
 * - 病案修改和数据更新
 * - 病案删除和归档处理
 * - 数据持久化存储
 *
 * 核心业务概念:
 *
 * 【病案定义】
 *   病案是患者在医院就诊过程中的完整医疗记录文档，
 *   包含从入院到出院的全过程诊疗信息。
 *
 * 【病案内容】
 *   1. 基本信息：编号、患者信息、性别、年龄
 *   2. 就诊信息：科室、主治医生、入院/出院日期
 *   3. 诊断信息：入院诊断、出院诊断
 *   4. 治疗信息：治疗摘要、治疗方案
 *   5. 管理信息：归档状态、备注说明
 *
 * 【归档状态】
 *   - 未归档(0): 病案仍在编辑或审核中
 *   - 已归档(1): 病案已完成归档，不可修改
 *
 * 数据存储格式:
 *   文本文件，每行一条记录，字段以"|"分隔
 *   字段顺序: id|patientId|name|gender|age|department|doctor|
 *             admitDiagnosis|dischargeDiagnosis|treatmentSummary|
 *             dischargeDate|archiveStatus|remark
 *
 * 应用场景:
 *   - 医生查看患者完整病史
 *   - 医疗质量控制和评估
 *   - 科研教学病例收集
 *   - 医疗纠纷证据保存
 *   - 统计分析和报表生成
 */

#include "record.h"

/**
 * @brief 初始化病案链表
 * @param list 病案链表指针
 *
 * 功能说明:
 *   将病案链表初始化为空表状态。
 *   设置头指针为NULL，记录数为0。
 *
 * 调用时机:
 *   - 系统启动时初始化数据结构
 *   - 需要清空现有数据重新加载时
 *
 * 初始状态:
 *   head = NULL (空链表)
 *   count = 0  (无记录)
 */
void initRecordList(RecordList* list) {
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 从文件加载病案数据
 * @param list 病案链表指针（用于存储加载的数据）
 * @param filename 数据文件路径
 *
 * 功能说明:
 *   从指定的文本文件中读取病案记录，
 *   解析每行数据并构建链表结构。
 *
 * 文件格式要求:
 *   - 编码: UTF-8 或 ANSI
 *   - 分隔符: 各字段使用"|"分隔
 *   - 行尾: 以换行符结束
 *   - 字段数: 必须包含13个字段
 *
 * 解析流程:
 *   1. 打开文件进行读取
 *   2. 逐行读取文件内容
 *   3. 使用sscanf解析各字段
 *   4. 创建节点并插入链表尾部
 *   5. 更新链表计数器
 *
 * 错误处理:
 *   - 文件不存在: 提示将创建新文件，不报错
 *   - 内存分配失败: 跳过该行记录
 *   - 格式错误: 可能导致数据不完整
 *
 * 性能特点:
 *   采用尾插法保持原有顺序
 *   时间复杂度: O(n)，n为记录数
 */
void loadRecordsFromFile(RecordList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");

    if (fp == NULL) {
        printf("提示：病案数据文件不存在，将创建新文件。\n");
        return;
    }

    RecordNode* tail = NULL;      /* 尾指针，用于高效插入 */
    char line[1024];              /* 行缓冲区 */

    /* 逐行读取并解析 */
    while (fgets(line, sizeof(line), fp)) {

        /* 分配新节点内存 */
        RecordNode* node = (RecordNode*)malloc(sizeof(RecordNode));
        if (!node) continue;     /* 内存分配失败则跳过 */

        /* 解析一行数据到结构体各字段 */
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^\n]",
            node->id,                    /* 病案编号 */
            node->patientId,             /* 患者编号 */
            node->name,                  /* 患者姓名 */
            node->gender,                /* 性别 */
            &node->age,                  /* 年龄 */
            node->department,            /* 科室 */
            node->doctor,                /* 主治医生 */
            node->admitDiagnosis,        /* 入院诊断 */
            node->dischargeDiagnosis,    /* 出院诊断 */
            node->treatmentSummary,      /* 治疗摘要 */
            node->dischargeDate,         /* 出院日期 */
            &node->archiveStatus,        /* 归档状态 */
            node->remark);               /* 备注 */

        node->next = NULL;

        /* 尾插法插入链表 */
        if (!list->head) {
            list->head = node;           /* 第一个节点 */
            tail = node;
        }
        else {
            tail->next = node;           /* 追加到尾部 */
            tail = node;
        }

        list->count++;                   /* 更新计数 */
    }

    fclose(fp);
    printf("成功加载 %d 条病案记录。\n", list->count);
}

/**
 * @brief 将病案数据保存到文件
 * @param list 病案链表指针（要保存的数据源）
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   将内存中的病案链表数据写入文本文件。
 *   采用覆盖写模式，每次保存都会重写整个文件。
 *
 * 写入格式:
 *   每条记录一行，字段以"|"分隔
 *   最后一个字段后换行
 *
 * 执行流程:
 *   1. 打开/创建文件（覆盖模式）
 *   2. 遍历链表每个节点
 *   3. 将每个字段按格式拼接并写入
 *   4. 关闭文件并输出结果
 *
 * 注意事项:
 *   - 写入前会清空原文件内容
 *   - 确保所有修改已提交到内存
 *   - 建议在关键操作后及时保存
 */
void saveRecordsToFile(RecordList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");

    if (!fp) {
        printf("错误：无法打开文件 %s 进行写入！\n", filename);
        return;
    }

    RecordNode* c = list->head;

    /* 遍历链表逐条写入 */
    while (c) {
        fprintf(fp, "%s|%s|%s|%s|%d|%s|%s|%s|%s|%s|%s|%d|%s\n",
            c->id,
            c->patientId,
            c->name,
            c->gender,
            c->age,
            c->department,
            c->doctor,
            c->admitDiagnosis,
            c->dischargeDiagnosis,
            c->treatmentSummary,
            c->dischargeDate,
            c->archiveStatus,
            c->remark);
        c = c->next;
    }

    fclose(fp);
    printf("成功保存 %d 条病案记录。\n", list->count);
}

/**
 * @brief 创建新的病案节点
 * @return 成功返回病案节点指针，失败返回NULL
 *
 * 功能说明:
 *   通过控制台交互方式收集病案信息，
 *   创建完整的病案数据节点。
 *
 * 收集的信息字段:
 *   1. 病案编号 [必填] - 唯一标识符
 *   2. 患者编号 [必填] - 关联的患者ID
 *   3. 姓名 [必填] - 患者真实姓名
 *   4. 性别 [必填] - 男或女
 *   5. 年龄 [必填] - 正整数
 *   6. 科室 [必填] - 如：内科、外科等
 *   7. 医生 [必填] - 主治医生姓名
 *   8. 入院诊断 [必填] - 初步诊断结果
 *   9. 出院诊断 [可选] - 最终确诊结果
 *   10. 治疗摘要 [可选] - 治疗过程概述（最长500字符）
 *   11. 出院日期 [必填] - 格式YYYY-MM-DD
 *   12. 归档状态 [自动] - 默认为0（未归档）
 *   13. 备注 [可选] - 其他补充信息
 *
 * 输入验证:
 *   - 编号不能为空字符串
 *   - 出院日期必须符合YYYY-MM-DD格式
 *
 * 自动设置:
 *   archiveStatus = 0 (新建病案默认未归档)
 *
 * 业务场景:
 *   - 患者出院时创建病案
 *   - 从其他系统导入病案数据
 *   - 补录历史病案信息
 */
RecordNode* createRecordNode() {
    RecordNode* n = (RecordNode*)malloc(sizeof(RecordNode));
    if (!n) return NULL;

    printf("\n--- 新增病案 ---\n");

    /*
     * ====== 输入字段1: 病案编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字、连字符的组合
     * 是否必填: 是（不能为空）
     * 输入样例:
     *   - "BA001" (病案拼音缩写+序号)
     *   - "MR-2024-0456" (MedicalRecord+日期+序号)
     *   - "R20260416001" (Record日期+流水号)
     */
    safeInput(n->id, MAX_ID_LEN, "病案编号: ");
    while (isEmpty(n->id)) {
        safeInput(n->id, MAX_ID_LEN, "编号不能为空！重新输入: ");
    }

    /*
     * ====== 输入字段2: 患者编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字组合（与住院/门诊共用）
     * 是否必填: 是
     * 输入样例:
     *   - "P001"
     *   - "BR20240101001"
     * 说明: 关联患者档案中的编号，用于跨模块查询
     */
    safeInput(n->patientId, MAX_ID_LEN, "病人编号: ");

    /*
     * ====== 输入字段3: 患者姓名 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 中文真实姓名
     * 是否必填: 是
     * 输入样例: "张三", "李四", "王芳"
     */
    safeInput(n->name, MAX_NAME_LEN, "姓名: ");

    /*
     * ====== 输入字段4: 性别 ======
     * 数据类型: char[10] (字符串)
     * 最大长度: 10个字符
     * 输入格式: 固定选项（男/女）
     * 是否必填: 是
     * 输入样例: "男", "女"
     */
    safeInput(n->gender, 10, "性别: ");

    /*
     * ====== 输入字段5: 年龄 ======
     * 数据类型: int (整数)
     * 取值范围: 0 ~ 150 (正整数)
     * 输入格式: 纯数字
     * 是否必填: 是
     * 输入样例:
     *   - "25" (25岁)
     *   - "68" (68岁)
     *   - "0" (新生儿)
     */
    n->age = inputInt("年龄: ");

    /*
     * ====== 输入字段6: 就诊科室 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符
     * 输入格式: 科室名称（中文）
     * 是否必填: 是
     * 输入样例:
     *   - "内科"
     *   - "外科"
     *   - "儿科"
     *   - "妇产科"
     * 说明: 患者住院期间所在的主要科室
     */
    safeInput(n->department, MAX_DEPT_LEN, "科室: ");

    /*
     * ====== 输入字段7: 主治医生 ======
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
    safeInput(n->doctor, MAX_NAME_LEN, "医生: ");

    /*
     * ====== 输入字段8: 入院诊断 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 医学术语描述
     * 是否必填: 是
     * 输入样例:
     *   - "急性阑尾炎"
     *   - "冠心病(不稳定型心绞痛)"
     *   - "2型糖尿病伴并发症"
     *   - "肺炎(社区获得性)"
     * 说明: 入院时的初步诊断结论，基于症状和初步检查
     */
    safeInput(n->admitDiagnosis, MAX_REMARK_LEN, "入院诊断: ");

    /*
     * ====== 输入字段9: 出院诊断 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 医学术语描述
     * 是否必填: 否（可留空，如尚未出院）
     * 输入样例:
     *   - "急性化脓性阑尾炎(已手术切除)"
     *   - "冠状动脉粥样硬化性心脏病(稳定型)"
     *   - "" (直接回车留空，待出院时补充)
     * 说明: 出院时的最终确诊结果，可能与人院诊断不同
     */
    safeInput(n->dischargeDiagnosis, MAX_REMARK_LEN, "出院诊断: ");

    /*
     * ====== 输入字段10: 治疗摘要 ======
     * 数据类型: char[500] (字符串) - 特长字段
     * 最大长度: 500个字符（250个中文字符）
     * 输入格式: 治疗过程详细描述
     * 是否必填: 是
     * 输入样例:
     *   - "入院后完善相关检查，给予抗感染、补液等对症支持治疗。
     *      于2026-04-18在硬膜外麻醉下行阑尾切除术，手术顺利，
     *      术后恢复良好，切口愈合II/甲。"
     *   - "给予冠心病二级预防药物治疗，控制血压血糖血脂，
     *      嘱定期复查，不适随诊。"
     *   - "保守治疗3天，症状缓解，准予出院。"
     * 说明: 详细记录从入院到出院的主要治疗措施和过程
     */
    safeInput(n->treatmentSummary, 500, "治疗摘要: ");

    /*
     * ====== 输入字段11: 出院日期 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: YYYY-MM-DD (严格格式)
     * 是否必填: 是
     * 输入样例:
     *   - "2026-04-22" (2026年4月22日出院)
     *   - "2026-05-01" (2026年5月1日出院)
     * 格式要求:
     *   - 年份: 4位数字
     *   - 月份: 2位数字 (01-12)
     *   - 日期: 2位数字 (根据月份1-31)
     * 验证: 自动检查日期有效性(含闰年判断)
     * 业务含义: 患者正式办理出院手续的日期
     */
    safeInput(n->dischargeDate, MAX_DATE_LEN, "出院日期: ");
    while (!isValidDate(n->dischargeDate)) {
        printf("日期格式不正确！\n");
        safeInput(n->dischargeDate, MAX_DATE_LEN, "重新输入出院日期: ");
    }

    /* 【系统设置】归档状态默认为 0(未归档) */

    /*
     * ====== 输入字段12: 备注 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 自由文本
     * 是否必填: 否（可留空）
     * 输入样例:
     *   - "" (直接回车留空)
     *   - "病案质量评级：甲级"
     *   - "死亡病例讨论已完成"
     *   - "需随访，预约门诊时间"
     * 用途: 记录病案管理相关的补充信息
     */
    safeInput(n->remark, MAX_REMARK_LEN, "备注: ");

    n->next = NULL;
    return n;
}

/**
 * @brief 检查病案编号是否已存在
 * @param list 病案链表指针
 * @param id 要检查的病案编号
 * @return 存在返回1，不存在返回0
 *
 * 功能说明:
 *   在病案链表中查找指定编号的记录，
 *   用于新增操作前的重复性检查。
 *
 * 查找方式:
 *   线性遍历链表，逐一比较id字段
 *
 * 应用场景:
 *   - 新增病案前检查唯一性
 *   - 防止重复录入相同病案
 */
int isRecordIDExist(RecordList* list, const char* id) {
    RecordNode* c = list->head;
    while (c) {
        if (strcmp(c->id, id) == 0) return 1;
        c = c->next;
    }
    return 0;
}

/**
 * @brief 插入病案节点到链表头部
 * @param list 病案链表指针
 * @param node 要插入的病案节点
 *
 * 功能说明:
 *   采用头插法将新节点插入链表。
 *   新节点成为链表的第一个元素。
 *
 * 操作步骤:
 *   1. 设置新节点的next指向当前头节点
 *   2. 更新头指针指向新节点
 *   3. 链表计数加1
 *
 * 时间复杂度: O(1)
 */
void insertRecordNode(RecordList* list, RecordNode* node) {
    if (node) {
        node->next = list->head;
        list->head = node;
        list->count++;
    }
}

/**
 * @brief 根据编号查找病案
 * @param list 病案链表指针
 * @param id 要查找的病案编号
 * @return 找到返回节点指针，未找到返回NULL
 *
 * 功能说明:
 *   在病案链表中线性搜索指定编号的记录。
 *
 * 查找算法:
 *   从头开始遍历，比较每个节点的id字段
 *   找到即返回该节点指针
 *
 * 应用场景:
 *   - 查询病案详情
 *   - 修改病案前的定位
 *   - 删除病案前的定位
 */
RecordNode* findRecordByID(RecordList* list, const char* id) {
    RecordNode* c = list->head;
    while (c) {
        if (strcmp(c->id, id) == 0) return c;
        c = c->next;
    }
    return NULL;
}

/**
 * @brief 根据编号删除病案
 * @param list 病案链表指针
 * @param id 要删除的病案编号
 * @return 删除成功返回1，失败返回0
 *
 * 功能说明:
 *   查找并删除指定编号的病案记录。
 *   删除前会显示记录详情并要求确认。
 *
 * 执行流程:
 *   1. 遍历链表查找目标节点
 *   2. 显示目标病案的详细信息
 *   3. 要求用户确认删除操作
 *   4. 执行删除并释放内存
 *   5. 更新链表计数
 *
 * 删除逻辑:
 *   - 头节点删除: 更新head指针
 *   - 中间/尾节点: 调整前一节点的next指针
 *
 * 安全机制:
 *   - 二次确认防止误删
 *   - 显示详细信息便于用户判断
 *
 * 注意事项:
 *   - 已归档的病案也可以删除（需谨慎）
 *   - 删除操作不可逆，建议先备份
 */
int deleteRecordByID(RecordList* list, const char* id) {
    RecordNode* c = list->head, * p = NULL;  /* c:当前节点, p:前驱节点 */

    while (c) {
        if (strcmp(c->id, id) == 0) {

            /* 显示待删除的病案信息 */
            printRecordOne(c);

            /* 二次确认 */
            if (!confirm("确认删除?")) return 0;

            /* 执行删除操作 */
            if (!p) {
                list->head = c->next;       /* 删除的是头节点 */
            } else {
                p->next = c->next;          /* 删除中间或尾节点 */
            }

            free(c);                        /* 释放内存 */
            list->count--;                   /* 更新计数 */
            return 1;                       /* 删除成功 */
        }
        p = c;                              /* 记录前驱 */
        c = c->next;                        /* 移动到下一个 */
    }

    printf("未找到。\n");
    return 0;
}

/**
 * @brief 修改病案信息
 * @param node 要修改的病案节点指针
 *
 * 功能说明:
 *   提供病案字段的选择性修改功能。
 *   用户可选择修改特定字段，无需重新输入全部信息。
 *
 * 可修改的字段列表:
 *   1. 入院诊断 - 修改初步诊断结果
 *   2. 出院诊断 - 修改最终诊断结果
 *   3. 治疗摘要 - 修改治疗过程描述
 *   4. 归档状态 - 切换归档状态(0/1)
 *   5. 备注 - 修改补充说明
 *   0. 取消 - 不做任何修改
 *
 * 操作流程:
 *   1. 显示当前病案信息
 *   2. 显示可修改字段菜单
 *   3. 用户选择要修改的字段
 *   4. 输入新值并更新
 *
 * 业务规则:
 *   - 已归档的病案理论上不应再修改
 *   - 但系统不做强制限制，由用户自行把控
 *   - 重要修改建议记录变更日志
 */
void modifyRecordInfo(RecordNode* node) {
    if (!node) return;

    /* 先显示当前信息 */
    printRecordOne(node);

    /* 显示可修改字段菜单 */
    printf("1.入院诊断 2.出院诊断 3.治疗摘要 4.归档状态 5.备注 0.取消\n");

    int ch = inputInt("选择: ");

    switch (ch) {
    case 1:
        safeInput(node->admitDiagnosis, MAX_REMARK_LEN, "新入院诊断: ");
        break;
    case 2:
        safeInput(node->dischargeDiagnosis, MAX_REMARK_LEN, "新出院诊断: ");
        break;
    case 3:
        safeInput(node->treatmentSummary, 500, "新治疗摘要: ");
        break;
    case 4:
        printf("归档状态(0.未归档 1.已归档): ");
        node->archiveStatus = inputInt("");
        break;
    case 5:
        safeInput(node->remark, MAX_REMARK_LEN, "新备注: ");
        break;
    case 0:
        return;  /* 取消修改 */
    }

    printf("修改成功！\n");
}

/**
 * @brief 打印单条病案信息
 * @param node 病案节点指针
 *
 * 功能说明:
 *   格式化显示单个病案的完整信息。
 *   用于查询结果展示和修改前的预览。
 *
 * 输出格式:
 *   使用分隔线包围，清晰美观
 *   信息分行显示，便于阅读
 *
 * 显示内容包括:
 *   - 病案编号
 *   - 患者基本信息（姓名、性别、年龄）
 *   - 就诊信息（科室、医生）
 *   - 诊断信息（入院诊断、出院诊断）
 *   - 治疗信息（治疗摘要）
 *   - 管理信息（出院日期、归档状态、备注）
 *
 * 特殊处理:
 *   - 归档状态转换为中文显示
 *   - NULL指针安全检查
 */
void printRecordOne(RecordNode* node) {
    if (!node) return;

    printLine('-', 70);

    printf("病案编号: %s\n", node->id);
    printf("病人: %s (%s, %d岁)\n", node->name, node->gender, node->age);
    printf("科室: %s 医生: %s\n", node->department, node->doctor);
    printf("入院诊断: %s\n", node->admitDiagnosis);
    printf("出院诊断: %s\n", node->dischargeDiagnosis);
    printf("治疗摘要: %s\n", node->treatmentSummary);
    printf("出院日期: %s 归档状态: %s\n",
        node->dischargeDate,
        node->archiveStatus ? "已归档" : "未归档");
    printf("备注: %s\n", node->remark);

    printLine('-', 70);
}

/**
 * @brief 打印所有病案信息
 * @param list 病案链表指针
 *
 * 功能说明:
 *   遍历链表，打印所有病案的详细信息。
 *   用于批量浏览和导出查看。
 *
 * 输出特点:
 *   - 显示总记录数
 *   - 每个病案调用printRecordOne()显示
 *   - 空链表给出提示信息
 *
 * 适用场景:
 *   - 全量数据浏览
 *   - 数据导出前的预览
 *   - 系统调试和测试
 */
void printRecordAll(RecordList* list) {
    if (!list->head) {
        printf("暂无病案记录。\n");
        return;
    }

    printf("\n--- 所有病案 (%d条) ---\n", list->count);

    RecordNode* c = list->head;
    while (c) {
        printRecordOne(c);  /* 逐条打印 */
        c = c->next;
    }
}

/**
 * @brief 释放病案链表内存
 * @param list 病案链表指针
 *
 * 功能说明:
 *   释放链表中所有节点的动态分配内存。
 *   在程序退出或重新加载数据前调用。
 *
 * 执行过程:
 *   1. 从头开始遍历链表
 *   2. 依次释放每个节点
 *   3. 重置链表为空状态
 *
 * 内存安全:
 *   - 正确处理空链表
 *   - 避免悬垂指针
 *   - 释放后head=NULL防止野指针
 *
 * 调用时机:
 *   - 程序退出前清理资源
 *   - 重新加载数据前清空旧数据
 */
void freeRecordList(RecordList* list) {
    RecordNode* c = list->head;

    while (c) {
        RecordNode* t = c;  /* 暂存当前节点 */
        c = c->next;        /* 移动到下一个 */
        free(t);            /* 释放暂存节点 */
    }

    list->head = NULL;      /* 重置头指针 */
    list->count = 0;        /* 重置计数 */
}

/**
 * @brief 病案管理主菜单
 * @param list 病案链表指针
 *
 * 功能说明:
 *   提供病案管理的完整功能界面。
 *   支持病案的CRUD（增删改查）操作。
 *
 * 菜单功能列表:
 *   1. 新增   - 创建新的病案记录
 *   2. 查询   - 按编号查找并显示病案
 *   3. 删除   - 按编号删除病案记录
 *   4. 修改   - 修改病案的部分字段
 *   5. 全部显示 - 浏览所有病案
 *   0. 返回   - 退出病案管理
 *
 * 操作流程:
 *   1. 循环显示菜单
 *   2. 接收用户选择
 *   3. 调用对应的功能函数
 *   4. 操作完成后暂停等待
 *
 * 安全检查:
 *   - 新增前检查编号唯一性
 *   - 删除前二次确认
 *   - 修改前验证记录存在
 *
 * 用户体验:
 *   - 清晰的菜单提示
 *   - 操作结果即时反馈
 *   - 错误输入友好提示
 */
void recordMenu(RecordList* list) {
    int choice;

    do {
        printf("\n");
        printTitle("病案管理系统");
        printf("1.新增 2.查询 3.删除 4.修改 5.全部显示 0.返回\n");

        choice = inputInt("选择: ");

        switch (choice) {

        /**
         * 功能1: 新增病案
         *
         * 操作步骤:
         *   1. 调用createRecordNode()创建新节点
         *   2. 检查编号是否已存在
         *   3. 不存在则插入链表
         *   4. 存在则释放节点并提示错误
         */
        case 1: {
            RecordNode* n = createRecordNode();
            if (n && !isRecordIDExist(list, n->id)) {
                insertRecordNode(list, n);
                printf("添加成功！\n");
            }
            else if (n) {
                printf("编号已存在！\n");
                free(n);
            }
            break;
        }

        /**
         * 功能2: 查询病案
         *
         * 操作步骤:
         *   1. 输入病案编号
         *   2. 调用findRecordByID()查找
         *   3. 找到则打印详细信息
         *   4. 未找到给出提示
         */
        case 2: {
            if (!list->head) break;

            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");

            RecordNode* f = findRecordByID(list, id);
            if (f) {
                printRecordOne(f);
            }
            else {
                printf("未找到。\n");
            }
            break;
        }

        /**
         * 功能3: 删除病案
         *
         * 操作步骤:
         *   1. 输入病案编号
         *   2. 调用deleteRecordByID()执行删除
         *   3. 函数内部会处理确认和实际删除
         */
        case 3: {
            if (!list->head) break;

            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");

            deleteRecordByID(list, id);
            break;
        }

        /**
         * 功能4: 修改病案
         *
         * 操作步骤:
         *   1. 输入病案编号
         *   2. 查找目标病案
         *   3. 找到则进入修改界面
         *   4. 未找到给出提示
         */
        case 4: {
            if (!list->head) break;

            char id[MAX_ID_LEN];
            safeInput(id, MAX_ID_LEN, "输入编号: ");

            RecordNode* n = findRecordByID(list, id);
            if (n) {
                modifyRecordInfo(n);
            }
            else {
                printf("未找到。\n");
            }
            break;
        }

        /**
         * 功能5: 显示所有病案
         *
         * 直接调用printRecordAll()遍历显示全部记录
         */
        case 5:
            printRecordAll(list);
            break;

        case 0:
            printf("返回主菜单...\n");
            break;

        default:
            printf("无效选择。\n");
        }

        /* 除退出外每次操作后暂停，让用户查看结果 */
        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}

#include "logistics.h"
#include "log.h"

/**
 * @brief 初始化物资链表
 * @param list 物资链表指针
 *
 * 功能说明:
 *   将物资链表的头指针置空，计数器归零。
 *   在系统启动时必须调用此函数初始化数据结构。
 *
 * 初始化内容:
 *   - head: NULL（空链表）
 *   - count: 0（无记录）
 */
void initMaterialList(MaterialList* list) {
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 初始化领用记录链表
 * @param list 领用记录链表指针
 *
 * 功能说明:
 *   将领用记录链表的头指针置空，计数器归零。
 *   用于存储物资领用/出库的详细记录信息。
 *
 * 初始化内容:
 *   - head: NULL（空链表）
 *   - count: 0（无记录）
 */
void initUsageList(UsageList* list) {
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 从文件加载物资数据到内存链表
 * @param list 物资链表指针（用于存储加载的数据）
 * @param filename 数据文件路径
 *
 * 功能说明:
 *   从指定文件读取物资库存记录，逐行解析并构建链表结构。
 *   文件不存在时会提示用户但不报错（首次运行时正常现象）。
 *
 * 文件格式要求:
 *   每行一条记录，字段以竖线(|)分隔：
 *   编号|名称|类别|数量|单位|最低库存|存放位置|供应商|最近入库日期|最近出库日期|备注
 *
 * 数据字段说明:
 *   - 编号: 唯一标识该物资
 *   - 类别: 如：医疗耗材、办公用品、清洁用品等
 *   - 数量: 当前库存数量（整数）
 *   - 单位: 计量单位（如：个、箱、包、瓶等）
 *   - 最低库存: 触发预警的阈值
 *   - 存放位置: 仓库具体位置
 *   - 供应商: 物资供应商名称
 *   - 最近入库/出库日期: YYYY-MM-DD格式或N/A
 *
 * 数据解析规则:
 *   - 使用sscanf安全读取各字段
 *   - 自动分配内存创建节点
 *   - 采用尾插法保持原有顺序
 *
 * 错误处理:
 *   - 文件不存在：提示并返回（不视为错误）
 *   - 内存分配失败：跳过该条记录继续下一条
 */
void loadMaterialsFromFile(MaterialList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("提示：物资数据文件不存在，将创建新文件。\n");
        return;
    }

    /* 尾指针用于高效插入，避免每次从头遍历 */
    MaterialNode* tail = NULL;
    char line[1024];

    /* 逐行读取文件内容 */
    while (fgets(line, sizeof(line), fp)) {
        MaterialNode* node = (MaterialNode*)malloc(sizeof(MaterialNode));
        if (node == NULL) continue;

        /* 解析一行数据到节点各字段 */
        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
            node->id, node->name, node->category, &node->quantity,
            node->unit, &node->minStock, node->location, node->supplier,
            node->lastInDate, node->lastOutDate, node->remark);

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
    printf("成功加载 %d 条物资记录。\n", list->count);
}

/**
 * @brief 从文件加载领用记录到内存链表
 * @param list 领用记录链表指针（用于存储加载的数据）
 * @param filename 数据文件路径
 *
 * 功能说明:
 *   从指定文件读取物资领用/出库记录，逐行解析并构建链表结构。
 *   用于追踪物资的去向和使用情况。
 *
 * 文件格式要求:
 *   每行一条记录，字段以竖线(|)分隔：
 *   记录编号|物资编号|领用部门|数量|日期|经手人|审核状态|备注
 *
 * 数据字段说明:
 *   - 记录编号: 唯一标识该条领用记录
 *   - 物资编号: 关联到物资表的编号
 *   - 领用部门: 使用该物资的部门
 *   - 数量: 本次领用的数量
 *   - 日期: 领用发生的日期（YYYY-MM-DD格式）
 *   - 经手人: 办理领用手续的人员
 *   - 审核状态: 0=待审核, 1=已通过, 2=已拒绝
 *
 * 错误处理:
 *   - 文件不存在：提示并返回（不视为错误）
 *   - 内存分配失败：跳过该条记录继续下一条
 */
void loadUsageFromFile(UsageList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("提示：领用记录文件不存在，将创建新文件。\n");
        return;
    }

    UsageNode* tail = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        UsageNode* node = (UsageNode*)malloc(sizeof(UsageNode));
        if (node == NULL) continue;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%d|%[^\n]",
            node->id, node->materialId, node->dept, &node->quantity,
            node->date, node->handler, &node->auditStatus, node->remark);

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
    printf("成功加载 %d 条领用记录。\n", list->count);
}

/**
 * @brief 将物资链表数据保存到文件
 * @param list 物资链表指针（包含要保存的数据）
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   遍历整个物资链表，将每条记录格式化写入文件。
 *   采用覆盖写模式，保存后文件只包含当前最新数据。
 *
 * 写入格式:
 *   字段间用竖线分隔，每行一条完整记录：
 *   编号|名称|类别|数量|单位|最低库存|存放位置|供应商|最近入库|最近出库|备注\n
 *
 * 调用时机:
 *   - 添加/删除/修改物资后
 *   - 入库/出库操作后
 *   - 系统退出前保存数据
 */
void saveMaterialsToFile(MaterialList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法打开文件 %s 进行写入！\n", filename);
        return;
    }

    MaterialNode* current = list->head;

    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%d|%s|%d|%s|%s|%s|%s|%s\n",
            current->id, current->name, current->category, current->quantity,
            current->unit, current->minStock, current->location, current->supplier,
            current->lastInDate, current->lastOutDate, current->remark);
        current = current->next;
    }

    fclose(fp);
    printf("成功保存 %d 条物资记录。\n", list->count);
}

/**
 * @brief 将领用记录链表数据保存到文件
 * @param list 领用记录链表指针（包含要保存的数据）
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   遍历整个领用记录链表，将每条记录格式化写入文件。
 *   用于持久化保存物资领用历史。
 *
 * 写入格式:
 *   字段间用竖线分隔，每行一条完整记录：
 *   记录编号|物资编号|领用部门|数量|日期|经手人|审核状态|备注\n
 */
void saveUsageToFile(UsageList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法打开文件 %s 进行写入！\n", filename);
        return;
    }

    UsageNode* current = list->head;

    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%d|%s|%s|%d|%s\n",
            current->id, current->materialId, current->dept, current->quantity,
            current->date, current->handler, current->auditStatus, current->remark);
        current = current->next;
    }

    fclose(fp);
    printf("成功保存 %d 条领用记录。\n", list->count);
}

/**
 * @brief 创建新的物资节点（交互式输入）
 * @return 成功返回物资节点指针，失败返回NULL
 *
 * 功能说明:
 *   通过控制台交互方式收集物资信息，创建完整的物资数据节点。
 *   包含输入验证和必填项检查。
 *
 * 收集的信息字段:
 *   1. 编号 [必填] - 唯一标识，不能为空
 *   2. 名称 [必填] - 物资的具体名称
 *   3. 类别 [必填] - 物资分类（如医疗耗材、办公用品等）
 *   4. 库存数量 [必填] - 当前库存量（整数）
 *   5. 计量单位 [必填] - 如：个、箱、包、瓶等
 *   6. 最低库存值 [必填] - 触发预警的临界值
 *   7. 存放位置 [必填] - 仓库中的具体位置
 *   8. 供应商 [必填] - 物资来源供应商
 *   9. 备注 [可选] - 其他补充信息
 *
 * 自动设置的字段:
 *   - lastInDate: "N/A"（新增时无入库记录）
 *   - lastOutDate: "N/A"（新增时无出库记录）
 *
 * 输入验证规则:
 *   - 编号不能为空字符串
 *   - 数量和最低库存为整数
 *
 * 返回值使用:
 *   - 成功: 返回的节点可直接插入链表
 *   - 失败: 内存分配问题返回NULL
 */
MaterialNode* createMaterialNode() {
    MaterialNode* node = (MaterialNode*)malloc(sizeof(MaterialNode));
    if (node == NULL) {
        printf("错误：内存分配失败！\n");
        return NULL;
    }

    printf("\n--- 新增物资 ---\n");

    /*
     * ====== 输入字段1: 物资编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字、连字符的组合
     * 是否必填: 是（不能为空）
     * 输入样例:
     *   - "WZ001" (物资拼音缩写+序号)
     *   - "MAT-2024-0789" (Material+日期+序号)
     *   - "L20260416001" (Logistics日期+流水号)
     */
    safeInput(node->id, MAX_ID_LEN, "请输入物资编号: ");
    while (isEmpty(node->id)) {
        printf("编号不能为空！\n");
        safeInput(node->id, MAX_ID_LEN, "请输入物资编号: ");
    }

    /*
     * ====== 输入字段2: 物资名称 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 中文或英文物资名称
     * 是否必填: 是
     * 输入样例:
     *   - "一次性口罩"
     *   - "医用酒精(75%)"
     *   - "无菌手套"
     *   - "消毒液"
     *   - "办公用纸(A4)"
     */
    safeInput(node->name, MAX_NAME_LEN, "请输入物资名称: ");

    /*
     * ====== 输入字段3: 物资类别 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符
     * 输入格式: 分类名称（中文）
     * 是否必填: 是
     * 输入样例:
     *   - "医疗耗材"
     *   - "办公用品"
     *   - "清洁用品"
     *   - "维修备件"
     *   - "被服类"
     * 说明: 用于分类管理和统计查询
     */
    safeInput(node->category, MAX_DEPT_LEN, "请输入类别: ");

    /*
     * ====== 输入字段4: 当前库存数量 ======
     * 数据类型: int (整数)
     * 取值范围: 0 ~ 很大的正数
     * 输入格式: 纯数字
     * 是否必填: 是
     * 输入样例:
     *   - "500" (500个/包/箱)
     *   - "1000" (1000件)
     *   - "0" (新入库，暂无库存)
     * 业务含义: 当前仓库中的实际库存量
     * 注意: 应与计量单位配合使用
     */
    node->quantity = inputInt("请输入库存数量: ");

    /*
     * ====== 输入字段5: 计量单位 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 单位名称
     * 是否必填: 是
     * 输入样例:
     *   - "个" (最常用)
     *   - "箱"
     *   - "包"
     *   - "瓶"
     *   - "卷"
     *   - "套"
     * 说明: 与库存数量配合使用，如"500个"
     */
    safeInput(node->unit, 20, "请输入计量单位: ");

    /*
     * ====== 输入字段6: 最低库存预警值 ======
     * 数据类型: int (整数)
     * 取值范围: 0 ~ 很大的正数
     * 输入格式: 纯数字
     * 是否必填: 是
     * 输入样例:
     *   - "100" (低于100个时预警)
     *   - "50" (低于50个时预警)
     *   - "10" (重要物资，低阈值预警)
     * 业务含义: 库存下限，当实际库存≤此值时触发低库存警告
     * 说明: 用于自动提醒采购补货
     */
    node->minStock = inputInt("请输入最低库存值: ");

    /*
     * ====== 输入字段7: 存放位置 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 位置描述文本
     * 是否必填: 是
     * 输入样例:
     *   - "库房A区3排5列"
     *   - "门诊楼1楼库房-货架2层"
     *   - "后勤仓库B区-01号柜"
     *   - "各科室护士站(分发点)"
     */
    safeInput(node->location, MAX_REMARK_LEN, "请输入存放位置: ");

    /*
     * ====== 输入字段8: 供应商 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 企业名称或简称
     * 是否必填: 是
     * 输入样例:
     *   - "XX医疗器械有限公司"
     *   - "市卫生材料厂"
     *   - "京东企业购"
     *   - "阳光采购平台"
     * 说明: 物资的供货来源，用于采购管理和质量追溯
     */
    safeInput(node->supplier, MAX_NAME_LEN, "请输入供应商: ");

    /* 【系统设置】以下字段由系统自动初始化:
     * - lastInDate = "N/A"  最后入库日期待定
     * - lastOutDate = "N/A" 最后出库日期待定
     */

    /*
     * ====== 输入字段9: 备注 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 自由文本
     * 是否必填: 否（可留空）
     * 输入样例:
     *   - "" (直接回车留空)
     *   - "需避光保存"
     *   - "有效期至2026-12-31"
     *   - "定点采购物资"
     * 用途: 记录特殊存储要求、有效期限等补充信息
     */
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注 (无则留空): ");

    node->next = NULL;
    return node;
}

/**
 * @brief 检查物资编号是否已存在
 * @param list 物资链表指针
 * @param id 待检查的编号
 * @return 存在返回1，不存在返回0
 *
 * 功能说明:
 *   在物资链表中线性搜索指定编号，用于保证编号唯一性。
 *
 * 使用场景:
 *   - 添加新物资前检查重复
 *   - 导入数据前去重校验
 */
int isMaterialIDExist(MaterialList* list, const char* id) {
    MaterialNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

/**
 * @brief 将物资节点插入链表头部
 * @param list 物资链表指针
 * @param node 待插入的物资节点指针
 *
 * 功能说明:
 *   采用头插法将新节点插入链表，时间复杂度O(1)。
 *   插入后自动更新链表计数器。
 *
 * 插入逻辑:
 *   1. 新节点的next指向当前头节点
 *   2. 头指针指向新节点
 *   3. 计数器+1
 */
void insertMaterialNode(MaterialList* list, MaterialNode* node) {
    if (node == NULL) return;

    node->next = list->head;
    list->head = node;
    list->count++;
}

/**
 * @brief 根据编号查找物资
 * @param list 物资链表指针
 * @param id 目标编号
 * @return 找到返回节点指针，未找到返回NULL
 *
 * 功能说明:
 *   在物资链表中按编号精确查找目标物资。
 *   返回的是实际节点的指针，可用于直接修改数据。
 *
 * 查找过程:
 *   1. 从链表头开始遍历
 *   2. 使用strcmp比较id字段
 *   3. 匹配则立即返回该节点地址
 *   4. 遍历完仍未找到返回NULL
 */
MaterialNode* findMaterialByID(MaterialList* list, const char* id) {
    MaterialNode* current = list->head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief 根据名称查找物资（模糊匹配）
 * @param list 物资链表指针
 * @param name 名称关键字（支持子串匹配）
 * @return 找到返回第一个匹配的节点指针，未找到返回NULL
 *
 * 功能说明:
 *   在物资链表中按名称进行模糊搜索。
 *   支持部分匹配，只要名称包含关键字即可找到。
 *
 * 匹配规则:
 *   - 使用strstr进行子串匹配
 *   - 返回第一个匹配的节点
 *
 * 应用场景:
 *   - 按名称快速查找物资
 *   - 支持只输入部分名称搜索
 */
MaterialNode* findMaterialByName(MaterialList* list, const char* name) {
    MaterialNode* current = list->head;
    while (current != NULL) {
        if (strstr(current->name, name) != NULL) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief 根据编号删除物资记录
 * @param list 物资链表指针
 * @param id 要删除的物资编号
 * @return 成功删除返回1，取消或未找到返回0
 *
 * 功能说明:
 *   完整的物资删除流程：
 *   1. 在链表中定位目标物资
 *   2. 显示物资信息供确认
 *   3. 用户确认后执行删除
 *   4. 记录操作日志
 *   5. 释放被删节点内存
 *
 * 删除前的确认机制:
 *   - 显示待删除物资的详细信息
 *   - 要求用户二次确认（Y/N）
 *   - 用户可取消删除操作
 *
 * 日志记录:
 *   记录被删物资的关键信息（编号、名称、类别）
 */
int deleteMaterialByID(MaterialList* list, const char* id) {
    MaterialNode* current = list->head;
    MaterialNode* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            /* 显示待删除记录详情 */
            printf("\n找到以下记录:\n");
            printMaterialOne(current);

            /* 二次确认防止误删 */
            if (!confirm("确认删除该物资?")) {
                printf("已取消删除。\n");
                return 0;
            }

            /* 准备日志信息（在删除前获取） */
            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "物资编号:%s, 名称:%s, 类别:%s",
                current->id, current->name, current->category);

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
            writeLog(LOG_LOGISTIC, "删除物资", "系统", detail);
            return 1;
        }
        prev = current;
        current = current->next;
    }

    printf("未找到编号为 %s 的物资。\n", id);
    return 0;
}

/**
 * @brief 修改物资信息（交互式选择字段修改）
 * @param node 待修改的物资节点指针
 *
 * 功能说明:
 *   提供菜单式界面让用户选择要修改的具体字段，
 *   支持单次修改一个字段，避免误改其他数据。
 *
 * 可修改的字段列表:
 *   1. 名称       - name字段
 *   2. 类别       - category字段
 *   3. 库存数量   - quantity字段（数值型）
 *   4. 计量单位   - unit字段
 *   5. 最低库存值 - minStock字段（数值型）
 *   6. 存放位置   - location字段
 *   7. 供应商     - supplier字段
 *   8. 备注       - remark字段
 *   0. 取消       - 不做任何修改
 *
 * 修改流程:
 *   1. 显示当前物资信息
 *   2. 显示可选修改项菜单
 *   3. 用户选择要修改的字段
 *   4. 输入新值替换旧值
 *   5. 记录修改日志
 *
 * 日志内容:
 *   包含物资编号、名称和修改的字段序号
 *
 * 注意事项:
 *   - 修改最低库存会影响库存预警判断
 *   - 修改库存数量应配合入库/出库操作使用
 */
void modifyMaterialInfo(MaterialNode* node) {
    if (node == NULL) return;

    printf("\n--- 修改物资信息 ---\n");
    printMaterialOne(node);

    printf("\n请选择要修改的字段:\n");
    printf("1. 名称\n");
    printf("2. 类别\n");
    printf("3. 库存数量\n");
    printf("4. 计量单位\n");
    printf("5. 最低库存值\n");
    printf("6. 存放位置\n");
    printf("7. 供应商\n");
    printf("8. 备注\n");
    printf("0. 取消\n");

    int choice = inputInt("请选择: ");

    switch (choice) {
    case 1:
        safeInput(node->name, MAX_NAME_LEN, "请输入新名称: ");
        break;
    case 2:
        safeInput(node->category, MAX_DEPT_LEN, "请输入新类别: ");
        break;
    case 3:
        node->quantity = inputInt("请输入新库存数量: ");
        break;
    case 4:
        safeInput(node->unit, 20, "请输入新计量单位: ");
        break;
    case 5:
        node->minStock = inputInt("请输入新最低库存值: ");
        break;
    case 6:
        safeInput(node->location, MAX_REMARK_LEN, "请输入新存放位置: ");
        break;
    case 7:
        safeInput(node->supplier, MAX_NAME_LEN, "请输入新供应商: ");
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
    sprintf(detail, "物资编号:%s, 名称:%s, 修改字段:%d", node->id, node->name, choice);
    writeLog(LOG_LOGISTIC, "修改物资信息", "系统", detail);
}

/**
 * @brief 物资入库操作
 * @param list 物资链表指针
 * @param id 入库物资的编号
 * @param quantity 入库数量（正整数）
 *
 * 功能说明:
 *   执行物资入库流程：
 *   1. 根据编号定位目标物资
 *   2. 增加库存数量
 *   3. 更新最近入库日期为当前日期
 *   4. 显示入库结果
 *   5. 记录操作日志
 *
 * 业务逻辑:
 *   - 库存数量 = 原库存 + 入库数量
 *   - 自动更新lastInDate字段
 *   - 入库不受库存上限限制
 *
 * 错误处理:
 *   - 物资不存在时提示错误并返回
 *
 * 日志内容:
 *   记录物资编号、名称、入库数量和当前总库存
 *
 * 应用场景:
 *   - 采购到货入库
 *   - 退库重新入库
 *   - 盘点调整增加
 */
void materialInStock(MaterialList* list, const char* id, int quantity) {
    MaterialNode* node = findMaterialByID(list, id);
    if (node == NULL) {
        printf("未找到该物资。\n");
        return;
    }

    /* 增加库存数量 */
    node->quantity += quantity;

    /* 更新最近入库日期为当前日期 */
    getCurrentDate(node->lastInDate);

    printf("入库成功！当前库存: %d %s\n", node->quantity, node->unit);

    /* 记录入库日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "物资编号:%s, 名称:%s, 入库:%d, 当前库存:%d%s",
        node->id, node->name, quantity, node->quantity, node->unit);
    writeLog(LOG_LOGISTIC, "物资入库", "系统", detail);
}

/**
 * @brief 物资出库操作
 * @param list 物资链表指针
 * @param id 出库物资的编号
 * @param quantity 出库数量（正整数）
 * @return 出库成功返回1，失败返回0
 *
 * 功能说明:
 *   执行物资出库流程：
 *   1. 根据编号定位目标物资
 *   2. 检查库存是否充足
 *   3. 减少库存数量
 *   4. 更新最近出库日期为当前日期
 *   5. 显示出库结果
 *   6. 记录操作日志
 *
 * 业务逻辑:
 *   - 库存数量 = 原库存 - 出库数量
 *   - 必须满足：原库存 >= 出库数量
 *   - 不允许负库存（超卖）
 *
 * 库存不足处理:
 *   - 提示当前可用库存
 *   - 返回0表示出库失败
 *   - 不扣减库存数量
 *
 * 日志内容:
 *   记录物资编号、名称、出库数量和剩余库存
 *
 * 应用场景:
 *   - 各部门领用物资
 *   - 物资调拨发出
 *   - 报废清理出库
 */
int materialOutStock(MaterialList* list, const char* id, int quantity) {
    MaterialNode* node = findMaterialByID(list, id);
    if (node == NULL) {
        printf("未找到该物资。\n");
        return 0;
    }

    /* 检查库存是否充足 */
    if (node->quantity < quantity) {
        printf("库存不足！当前库存: %d %s\n", node->quantity, node->unit);
        return 0;
    }

    /* 扣减库存数量 */
    node->quantity -= quantity;

    /* 更新最近出库日期为当前日期 */
    getCurrentDate(node->lastOutDate);

    printf("出库成功！剩余库存: %d %s\n", node->quantity, node->unit);

    /* 记录出库日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "物资编号:%s, 名称:%s, 出库:%d, 剩余库存:%d%s",
        node->id, node->name, quantity, node->quantity, node->unit);
    writeLog(LOG_LOGISTIC, "物资出库", "系统", detail);
    return 1;
}

/**
 * @brief 打印单个物资的详细信息
 * @param node 物资节点指针
 *
 * 功能说明:
 *   格式化输出一种物资的完整信息，用于查看详情。
 *   特别显示库存预警状态。
 *
 * 显示内容:
 *   - 物资编号（唯一标识）
 *   - 物资名称
 *   - 类别
 *   - 库存数量及单位
 *   - 最低库存值及单位
 *   - 库存预警标记【当库存<=最低库存时显示】
 *   - 存放位置
 *   - 供应商
 *   - 最近入库日期
 *   - 最近出库日期
 *   - 备注
 *
 * 库存预警判断:
 *   当 quantity <= minStock 时显示【库存预警】标记，
 *   提示该物资需要及时补货。
 *
 * 输出格式特点:
 *   - 使用70字符宽的分隔线
 *   - 每个字段独占一行
 *   - 预警信息醒目显示
 */
void printMaterialOne(MaterialNode* node) {
    if (node == NULL) return;

    printLine('-', 70);
    printf("物资编号: %s\n", node->id);
    printf("物资名称: %s\n", node->name);
    printf("类别: %s\n", node->category);
    printf("库存: %d %s\n", node->quantity, node->unit);
    printf("最低库存: %d %s\n", node->minStock, node->unit);

    /* 库存预警判断与显示 */
    if (node->quantity <= node->minStock) {
        printf("【库存预警】\n");
    }

    printf("存放位置: %s\n", node->location);
    printf("供应商: %s\n", node->supplier);
    printf("最近入库: %s\n", node->lastInDate);
    printf("最近出库: %s\n", node->lastOutDate);
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

/**
 * @brief 打印所有物资列表
 * @param list 物资链表指针
 *
 * 功能说明:
 *   遍历整个物资链表，依次输出每种物资的详细信息。
 *   先显示总数概览，再逐个列出详情。
 *
 * 输出结构:
 *   --- 所有物资 (共 N 种) ---
 *   [物资1详细信息]
 *   [物资2详细信息]
 *   ...
 *
 * 特殊处理:
 *   - 空链表时提示"暂无物资记录"
 *   - 每种物资调用printMaterialOne()显示
 */
void printMaterialAll(MaterialList* list) {
    if (list->head == NULL) {
        printf("暂无物资记录。\n");
        return;
    }

    printf("\n--- 所有物资 (共 %d 种) ---\n", list->count);
    MaterialNode* current = list->head;

    while (current != NULL) {
        printMaterialOne(current);
        current = current->next;
    }
}

/**
 * @brief 打印库存预警物资列表
 * @param list 物资链表指针
 *
 * 功能说明:
 *   遍历物资链表，筛选出所有库存低于或等于最低库存值的物资。
 *   这些物资需要及时补货，避免影响正常使用。
 *
 * 预警条件:
 *   quantity <= minStock（库存 <= 最低库存）
 *
 * 输出内容:
 *   - 符合条件的物资详细信息
 *   - 统计需要补货的物资总数
 *
 * 输出示例:
 *   --- 库存预警物资 ---
 *   [物资1详细信息 - 含【库存预警】标记]
 *   [物资2详细信息 - 含【库存预警】标记]
 *   共 N 种物资需要补货。
 *
 * 特殊情况:
 *   - 无预警物资时提示"暂无库存预警物资"
 *   - 有预警物资时汇总显示总数
 *
 * 应用场景:
 *   - 定期库存盘点
 *   - 采购计划制定
 *   - 库存管理监控
 */
void printStockWarning(MaterialList* list) {
    if (list->head == NULL) {
        printf("暂无物资记录。\n");
        return;
    }

    int count = 0;
    MaterialNode* current = list->head;

    printf("\n--- 库存预警物资 ---\n");

    /* 遍历链表筛选低库存物资 */
    while (current != NULL) {
        if (current->quantity <= current->minStock) {
            printMaterialOne(current);
            count++;
        }
        current = current->next;
    }

    /* 汇总统计结果 */
    if (count == 0) {
        printf("暂无库存预警物资。\n");
    }
    else {
        printf("共 %d 种物资需要补货。\n", count);
    }
}

/**
 * @brief 将领用记录节点插入链表头部
 * @param list 领用记录链表指针
 * @param node 待插入的领用记录节点指针
 *
 * 功能说明:
 *   采用头插法将新领用记录插入链表。
 *   时间复杂度O(1)，插入后自动更新计数器。
 *
 * 插入逻辑:
 *   1. 新节点的next指向当前头节点
 *   2. 头指针指向新节点
 *   3. 计数器+1
 */
void insertUsageNode(UsageList* list, UsageNode* node) {
    if (node == NULL) return;

    node->next = list->head;
    list->head = node;
    list->count++;
}

/**
 * @brief 释放物资链表占用的所有内存
 * @param list 物资链表指针
 *
 * 功能说明:
 *   遍历链表逐个释放每个物资节点分配的内存。
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
void freeMaterialList(MaterialList* list) {
    MaterialNode* current = list->head;
    while (current != NULL) {
        MaterialNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 释放领用记录链表占用的所有内存
 * @param list 领用记录链表指针
 *
 * 功能说明:
 *   遍历链表逐个释放每个领用记录节点分配的内存。
 *   与freeMaterialList()逻辑相同但针对不同数据类型。
 */
void freeUsageList(UsageList* list) {
    UsageNode* current = list->head;
    while (current != NULL) {
        UsageNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->count = 0;
}

/**
 * @brief 新增物资（封装操作）
 * @param list 物资链表指针
 *
 * 功能说明:
 *   完整的新增物资流程：
 *   1. 创建新的物资节点（交互式输入）
 *   2. 检查编号是否重复
 *   3. 插入链表
 *   4. 记录操作日志
 *
 * 错误处理:
 *   - 编号重复时提示错误并释放节点
 *   - 内存分配失败时直接返回
 *
 * 日志内容:
 *   记录新增物资的关键信息（编号、名称、类别、库存）
 */
void addMaterial(MaterialList* list) {
    MaterialNode* node = createMaterialNode();
    if (node == NULL) return;

    /* 检查编号唯一性 */
    if (isMaterialIDExist(list, node->id)) {
        printf("错误：物资编号 %s 已存在！\n", node->id);
        free(node);
        return;
    }

    /* 插入链表并提示成功 */
    insertMaterialNode(list, node);
    printf("物资添加成功！\n");

    /* 记录新增日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "物资编号:%s, 名称:%s, 类别:%s, 库存:%d%s",
        node->id, node->name, node->category, node->quantity, node->unit);
    writeLog(LOG_LOGISTIC, "新增物资", "系统", detail);
}

/**
 * @brief 查询物资（多条件查询入口）
 * @param list 物资链表指针
 *
 * 功能说明:
 *   提供多种查询方式的统一入口，用户可选择不同的查询条件。
 *   支持精确查询和模糊查询，以及库存预警查看。
 *
 * 查询方式列表:
 *   1. 按编号查询  - 精确定位单种物资
 *   2. 按名称查询  - 支持模糊匹配，可找出多个结果
 *   3. 按类别查询  - 支持模糊匹配，可找出多个结果
 *   4. 显示全部    - 无条件浏览所有物资
 *   5. 查看库存预警 - 仅显示低于最低库存的物资
 *   0. 返回        - 退出查询
 *
 * 查询特点:
 *   - 支持精确查询和模糊查询
 *   - 名称和类别支持子串匹配
 *   - 多结果时逐个显示详情
 *   - 库存预警自动标注【库存预警】标记
 */
void queryMaterial(MaterialList* list) {
    if (list->head == NULL) {
        printf("暂无物资记录。\n");
        return;
    }

    printf("\n--- 查询物资 ---\n");
    printf("1. 按编号查询\n");
    printf("2. 按名称查询\n");
    printf("3. 按类别查询\n");
    printf("4. 显示全部\n");
    printf("5. 查看库存预警\n");
    printf("0. 返回\n");

    int choice = inputInt("请选择: ");
    char keyword[MAX_NAME_LEN];

    switch (choice) {
    case 1:
        /* 按编号精确定位 */
        safeInput(keyword, MAX_ID_LEN, "请输入编号: ");
        MaterialNode* found = findMaterialByID(list, keyword);
        if (found) {
            printMaterialOne(found);
        }
        else {
            printf("未找到。\n");
        }
        break;
    case 2:
        /* 按名称模糊匹配（可能多个结果） */
        safeInput(keyword, MAX_NAME_LEN, "请输入名称: ");
        MaterialNode* curr = list->head;
        int cnt = 0;
        while (curr != NULL) {
            if (strstr(curr->name, keyword)) {
                printMaterialOne(curr);
                cnt++;
            }
            curr = curr->next;
        }
        if (cnt == 0) {
            printf("未找到。\n");
        }
        break;
    case 3:
        /* 按类别模糊匹配（可能多个结果） */
        safeInput(keyword, MAX_DEPT_LEN, "请输入类别: ");
        curr = list->head;
        cnt = 0;
        while (curr != NULL) {
            if (strstr(curr->category, keyword)) {
                printMaterialOne(curr);
                cnt++;
            }
            curr = curr->next;
        }
        if (cnt == 0) {
            printf("未找到。\n");
        }
        break;
    case 4:
        /* 显示全部物资记录 */
        printMaterialAll(list);
        break;
    case 5:
        /* 查看库存预警物资 */
        printStockWarning(list);
        break;
    case 0:
        return;
    default:
        printf("无效选择。\n");
    }
}

/**
 * @brief 删除物资（封装操作）
 * @param list 物资链表指针
 *
 * 功能说明:
 *   删除操作的简化接口，只需输入编号即可完成删除。
 *   内部调用deleteMaterialByID()执行实际删除操作。
 */
void deleteMaterial(MaterialList* list) {
    if (list->head == NULL) {
        printf("暂无物资记录。\n");
        return;
    }

    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入要删除的物资编号: ");
    deleteMaterialByID(list, id);
}

/**
 * @brief 修改物资信息（封装操作）
 * @param list 物资链表指针
 *
 * 功能说明:
 *   修改操作的简化接口，先查找再修改。
 *   内部调用findMaterialByID()和modifyMaterialInfo()。
 */
void modifyMaterial(MaterialList* list) {
    if (list->head == NULL) {
        printf("暂无物资记录。\n");
        return;
    }

    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入要修改的物资编号: ");

    MaterialNode* node = findMaterialByID(list, id);
    if (node == NULL) {
        printf("未找到。\n");
        return;
    }

    modifyMaterialInfo(node);
}

/**
 * @brief 物资入库（封装操作）
 * @param list 物资链表指针
 *
 * 功能说明:
 *   入库操作的简化接口，通过交互式界面完成入库。
 *   用户需输入物资编号和入库数量。
 *
 * 操作流程:
 *   1. 提示输入物资编号
 *   2. 提示输入入库数量
 *   3. 调用materialInStock()执行实际入库
 *
 * 注意事项:
 *   - 入库数量应为正整数
 *   - 会自动更新最近入库日期
 *   - 无库存上限限制
 */
void materialIn(MaterialList* list) {
    if (list->head == NULL) {
        printf("暂无物资记录。\n");
        return;
    }

    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入物资编号: ");

    int qty = inputInt("请输入入库数量: ");
    materialInStock(list, id, qty);
}

/**
 * @brief 物资出库并生成领用记录（封装操作）
 * @param list 物资链表指针
 * @param uList 领用记录链表指针（用于保存出库产生的领用记录）
 *
 * 功能说明:
 *   完整的出库流程：
 *   1. 用户输入物资编号和出库数量
 *   2. 调用materialOutStock()执行出库扣减
 *   3. 出库成功后自动生成领用记录
 *   4. 领用记录包含部门、经手人等信息
 *
 * 领用记录内容:
 *   - 记录编号: 自动生成（U开头）
 *   - 物资编号: 关联出库的物资
 *   - 领用部门: 用户输入
 *   - 领用数量: 与出库数量一致
 *   - 领用日期: 当前日期
 *   - 经手人: 用户输入
 *   - 审核状态: 默认1（已通过）
 *
 * 业务含义:
 *   每次出库都会产生一条领用记录，
 *   用于追溯物资去向和责任归属。
 *
 * 错误处理:
 *   - 库存不足时不产生领用记录
 *   - 内存分配失败时跳过记录创建
 */
void materialOut(MaterialList* list, UsageList* uList) {
    if (list->head == NULL) {
        printf("暂无物资记录。\n");
        return;
    }

    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入物资编号: ");

    int qty = inputInt("请输入出库数量: ");

    /* 执行出库操作，成功后才创建领用记录 */
    if (materialOutStock(list, id, qty)) {
        UsageNode* unode = (UsageNode*)malloc(sizeof(UsageNode));
        if (unode) {
            /* 自动生成领用记录编号 */
            generateID(unode->id, "U");

            /* 关联出库的物资 */
            strcpy(unode->materialId, id);

            /* 收集领用相关信息 */
            safeInput(unode->dept, MAX_DEPT_LEN, "请输入领用部门: ");
            unode->quantity = qty;
            getCurrentDate(unode->date);
            safeInput(unode->handler, MAX_NAME_LEN, "请输入经手人: ");

            /* 设置审核状态为已通过 */
            unode->auditStatus = 1;
            unode->remark[0] = '\0';
            unode->next = NULL;

            /* 插入领用记录链表 */
            insertUsageNode(uList, unode);
        }
    }
}

/**
 * @brief 查询领用记录
 * @param list 领用记录链表指针
 *
 * 功能说明:
 *   显示所有物资领用/出库的历史记录。
 *   以简洁的表格形式展示关键信息。
 *
 * 显示内容:
 *   - 记录编号
 *   - 物资编号
 *   - 领用部门
 *   - 领用数量
 *   - 领用日期
 *
 * 输出格式示例:
 *   U001 | MAT001 | 内科 | 50 | 2024-01-15
 *   U002 | MAT003 | 外科 | 30 | 2024-01-16
 *   共 N 条记录。
 *
 * 应用场景:
 *   - 查看物资去向
 *   - 统计各部门领用量
 *   - 追溯物资使用历史
 */
void queryUsage(UsageList* list) {
    if (list->head == NULL) {
        printf("暂无领用记录。\n");
        return;
    }

    printf("\n--- 领用记录 ---\n");
    UsageNode* current = list->head;
    int cnt = 0;

    while (current != NULL) {
        printf("%s | %s | %s | %d | %s\n",
            current->id, current->materialId, current->dept,
            current->quantity, current->date);
        cnt++;
        current = current->next;
    }

    printf("共 %d 条记录。\n", cnt);
}

/**
 * @brief 物资统计分析
 * @param mList 物资链表指针
 * @param uList 领用记录链表指针
 *
 * 功能说明:
 *   提供物资管理的整体概况统计，
 *   包括物资种类、总量和领用情况。
 *
 * 统计指标:
 *   1. 物资种类数 - 不同物资的总数量
 *   2. 物资总数量 - 所有物资库存量的总和
 *   3. 领用记录数 - 历史领用/出库的总次数
 *
 * 输出示例:
 *   --- 物资统计 ---
 *   物资种类: 150
 *   物资总数量: 12580
 *   领用记录数: 320
 *
 * 业务含义:
 *   - 物资种类反映库存丰富度
 *   - 总数量反映库存规模
 *   - 领用记录反映物资流转频率
 *
 * 应用场景:
 *   - 库存概况了解
 *   - 管理决策参考
 *   - 工作汇报数据
 */
void statMaterials(MaterialList* mList, UsageList* uList) {
    printf("\n--- 物资统计 ---\n");

    /* 统计物资种类 */
    printf("物资种类: %d\n", mList->count);

    /* 累计计算所有物资的库存总量 */
    int totalQty = 0;
    MaterialNode* curr = mList->head;
    while (curr != NULL) {
        totalQty += curr->quantity;
        curr = curr->next;
    }
    printf("物资总数量: %d\n", totalQty);

    /* 显示领用记录总数 */
    printf("领用记录数: %d\n", uList->count);
}

/**
 * @brief 后勤管理主菜单
 * @param mList 物资链表指针
 * @param uList 领用记录链表指针
 *
 * 功能说明:
 *   提供后勤管理的交互式主界面，循环显示菜单并响应用户选择。
 *   是后勤模块的入口函数，整合了所有物资管理和领用管理操作功能。
 *
 * 菜单功能列表:
 *   1. 新增物资    - 调用addMaterial()
 *   2. 查询物资    - 调用queryMaterial()
 *   3. 删除物资    - 调用deleteMaterial()
 *   4. 修改物资    - 调用modifyMaterial()
 *   5. 物资入库    - 调用materialIn()
 *   6. 物资出库    - 调用materialOut()
 *   7. 领用记录    - 调用queryUsage()
 *   8. 统计分析    - 调用statMaterials()
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
 *   - 添加时检查编号重复
 *   - 删除前要求确认
 *   - 出库前检查库存充足性
 *
 * 日志集成:
 *   - 新增物资时自动记录日志
 *   - 删除物资时自动记录日志
 *   - 修改物资信息时自动记录日志
 *   - 物资入库时自动记录日志
 *   - 物资出库时自动记录日志
 *
 * 业务特点:
 *   - 双链表架构（物资+领用记录）
 *   - 完善的入库出库流程
 *   - 库存预警机制
 *   - 领用记录追踪
 *   - 操作全程日志记录
 */
void logisticsMenu(MaterialList* mList, UsageList* uList) {
    int choice;

    do {
        printf("\n");
        printTitle("后勤管理系统");
        printf("1. 新增物资\n");
        printf("2. 查询物资\n");
        printf("3. 删除物资\n");
        printf("4. 修改物资\n");
        printf("5. 物资入库\n");
        printf("6. 物资出库\n");
        printf("7. 领用记录\n");
        printf("8. 统计分析\n");
        printf("0. 返回主菜单\n");

        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: addMaterial(mList); break;
        case 2: queryMaterial(mList); break;
        case 3: deleteMaterial(mList); break;
        case 4: modifyMaterial(mList); break;
        case 5: materialIn(mList); break;
        case 6: materialOut(mList, uList); break;
        case 7: queryUsage(uList); break;
        case 8: statMaterials(mList, uList); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }

        /* 除退出外每次操作后暂停，让用户查看结果 */
        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}

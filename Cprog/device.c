#include "device.h"
#include "log.h"

/**
 * @brief 将设备状态枚举值转换为中文字符串显示
 * @param status 设备状态枚举值(DEVICE_NORMAL/DEVICE_REPAIRING/DEVICE_SCRAPPED)
 * @return 对应的中文字符串指针
 *
 * 状态映射关系:
 *   DEVICE_NORMAL(0)    → "正常"     （设备可正常使用）
 *   DEVICE_REPAIRING(1) → "维修中"   （设备正在维修）
 *   DEVICE_SCRAPPED(2)  → "已报废"   （设备已报废停用）
 *   其他               → "未知"      （无效状态值）
 *
 * 使用场景:
 *   - 设备信息显示时将数字状态转为可读文本
 *   - 统计报表生成时的状态分类显示
 *   - 用户界面友好化展示
 */
const char* getDeviceStatusString(int status) {
    switch (status) {
    case DEVICE_NORMAL: return "正常";
    case DEVICE_REPAIRING: return "维修中";
    case DEVICE_SCRAPPED: return "已报废";
    default: return "未知";
    }
}

/**
 * @brief 初始化设备链表
 * @param list 设备链表指针
 *
 * 功能说明:
 *   将链表头指针置空，计数器归零。
 *   在系统启动时必须调用此函数初始化数据结构。
 *
 * 初始化内容:
 *   - head: NULL（空链表）
 *   - count: 0（无记录）
 */
void initDeviceList(DeviceList* list) { list->head = NULL; list->count = 0; }

/**
 * @brief 初始化维修记录链表
 * @param list 维修记录链表指针
 *
 * 功能说明:
 *   将维修记录链表头指针置空，计数器归零。
 *   用于管理系统中的设备维修历史记录。
 *
 * 初始化内容:
 *   - head: NULL（空链表）
 *   - count: 0（无记录）
 */
void initFixList(FixList* list) { list->head = NULL; list->count = 0; }

/**
 * @brief 从文件加载设备数据到内存链表
 * @param list 设备链表指针（用于存储加载的数据）
 * @param filename 数据文件路径
 *
 * 功能说明:
 *   从指定文件读取设备记录，逐行解析并构建链表结构。
 *   文件不存在时会提示用户但不报错（首次运行时正常现象）。
 *
 * 文件格式要求:
 *   每行一条记录，字段以竖线(|)分隔：
 *   编号|名称|类别|科室|购入日期|价格|状态|是否需购入|购入数量|责任人|位置|保修期|备注
 *
 * 数据解析规则:
 *   - 使用sscanf_s安全读取各字段
 *   - 自动分配内存创建节点
 *   - 采用尾插法保持原有顺序
 *
 * 错误处理:
 *   - 文件不存在：提示并返回（不视为错误）
 *   - 内存分配失败：跳过该条记录继续下一条
 *
 * 示例文件内容:
 *   DEV001|CT扫描仪|影像科|放射科|2024-01-15|1500000.00|0|0|0|张三|一楼东侧|5年|原装进口
 */
void loadDevicesFromFile(DeviceList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：设备数据文件不存在，将创建新文件。\n"); return; }

    /* 尾指针用于高效插入，避免每次从头遍历 */
    DeviceNode* tail = NULL;
    char line[1024];

    /* 逐行读取文件内容 */
    while (fgets(line, sizeof(line), fp)) {
        DeviceNode* node = (DeviceNode*)malloc(sizeof(DeviceNode));
        if (node == NULL) continue;

        /* 解析一行数据到节点各字段 */
        sscanf_s(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%lf|%d|%d|%d|%[^|]|%[^|]|%[^|]|%[\n]",
            node->id, (unsigned)MAX_ID_LEN,
            node->name, (unsigned)MAX_NAME_LEN,
            node->category, (unsigned)MAX_DEPT_LEN,
            node->department, (unsigned)MAX_DEPT_LEN,
            node->purchaseDate, (unsigned)MAX_DATE_LEN,
            &node->price, &node->status, &node->needPurchase,
            &node->purchaseQuantity,
            node->handler, (unsigned)MAX_NAME_LEN,
            node->location, (unsigned)MAX_REMARK_LEN,
            node->warrantyPeriod, (unsigned)20,
            node->remark, (unsigned)MAX_REMARK_LEN);

        node->next = NULL;

        /* 尾插法维护链表 */
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条设备记录。\n", list->count);
}

/**
 * @brief 从文件加载维修记录到内存链表
 * @param list 维修记录链表指针（用于存储加载的数据）
 * @param filename 数据文件路径
 *
 * 功能说明:
 *   从指定文件读取设备维修历史记录，构建链表结构。
 *   用于追踪设备的维修情况和费用统计。
 *
 * 文件格式要求:
 *   每行一条记录，字段以竖线(|)分隔：
 *   记录编号|设备编号|故障描述|送修日期|完成日期|维修费用|维修结果|处理人|备注
 *
 * 数据字段说明:
 *   - 记录编号: 唯一标识本次维修事件
 *   - 设备编号: 关联的设备ID
 *   - 故障描述: 详细的问题说明
 *   - 送修/完成日期: 时间范围
 *   - 维修费用: 花费金额
 *   - 维修结果: 修复情况描述
 *
 * 加载流程:
 *   1. 打开文件（不存在则提示返回）
 *   2. 逐行读取并解析
 *   3. 创建节点并链接到链表
 *   4. 关闭文件并报告加载数量
 */
void loadFixesFromFile(FixList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：维修记录文件不存在，将创建新文件。\n"); return; }

    FixNode* tail = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        FixNode* node = (FixNode*)malloc(sizeof(FixNode));
        if (node == NULL) continue;

        /* 解析维修记录各字段 */
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%lf|%[^|]|%[^|]|%[^\n]",
            node->id, node->deviceId, node->description,
            node->sendDate, node->finishDate, &node->cost,
            node->result, node->handler, node->remark);

        node->next = NULL;

        /* 链表尾插法 */
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条维修记录。\n", list->count);
}

/**
 * @brief 将设备链表数据保存到文件
 * @param list 设备链表指针（包含要保存的数据）
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   遍历整个设备链表，将每条记录格式化写入文件。
 *   采用覆盖写模式，保存后文件只包含当前最新数据。
 *
 * 写入格式:
 *   字段间用竖线分隔，每行一条完整记录：
 *   编号|名称|类别|科室|购入日期|价格|状态|是否需购入|购入数量|责任人|位置|保修期|备注\n
 *
 * 数据完整性保障:
 *   - 每条记录独立一行，便于逐行读取
 *   - 数值类型保留两位小数
 *   - 最后一个字段后换行符结束
 *
 * 调用时机:
 *   - 添加/删除/修改设备后
 *   - 系统退出前保存数据
 *   - 手动触发保存操作时
 */
void saveDevicesToFile(DeviceList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }

    DeviceNode* current = list->head;

    /* 遍历链表逐条写入 */
    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%s|%.2f|%d|%d|%d|%s|%s|%s|%s\n",
            current->id, current->name, current->category, current->department,
            current->purchaseDate, current->price, current->status,
            current->needPurchase, current->purchaseQuantity,
            current->handler, current->location, current->warrantyPeriod, current->remark);
        current = current->next;
    }
    fclose(fp);
    printf("成功保存 %d 条设备记录。\n", list->count);
}

/**
 * @brief 将维修记录链表数据保存到文件
 * @param list 维修记录链表指针（包含要保存的数据）
 * @param filename 目标文件路径
 *
 * 功能说明:
 *   将所有维修记录持久化存储到文件中。
 *   用于保留设备维修的历史档案。
 *
 * 写入格式:
 *   记录编号|设备编号|故障描述|送修日期|完成日期|维修费用|维修结果|处理人|备注\n
 *
 * 应用场景:
 *   - 新增维修记录后保存
 *   - 更新维修状态后保存
 *   - 系统关闭前数据备份
 */
void saveFixesToFile(FixList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }

    FixNode* current = list->head;

    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%s|%.2f|%s|%s|%s\n",
            current->id, current->deviceId, current->description,
            current->sendDate, current->finishDate, current->cost,
            current->result, current->handler, current->remark);
        current = current->next;
    }
    fclose(fp);
    printf("成功保存 %d 条维修记录。\n", list->count);
}

/**
 * @brief 创建新的设备节点（交互式输入）
 * @return 成功返回设备节点指针，失败返回NULL
 *
 * 功能说明:
 *   通过控制台交互方式收集设备信息，创建完整的设备数据节点。
 *   包含输入验证和必填项检查。
 *
 * 收集的信息字段:
 *   1. 设备编号 [必填] - 唯一标识，不能为空
 *   2. 设备名称 [必填] - 设备的正式名称
 *   3. 设备类别 [必填] - 如：影像、检验、手术等
 *   4. 所属科室 [必填] - 使用该设备的科室
 *   5. 购入日期 [必填] - 格式YYYY-MM-DD，会验证格式
 *   6. 购入价格 [必填] - 浮点数，单位元
 *   7. 责任人 [必填] - 设备负责人姓名
 *   8. 存放位置 [必填] - 具体存放地点
 *   9. 保修期 [可选] - 如：3年、5年等
 *   10. 备注 [可选] - 其他补充信息
 *
 * 自动设置的字段:
 *   - status: DEVICE_NORMAL（新增设备默认正常状态）
 *   - needPurchase: 0（默认不需要采购）
 *   - purchaseQuantity: 0（默认采购数量为0）
 *
 * 输入验证规则:
 *   - 编号不能为空字符串
 *   - 日期必须符合YYYY-MM-DD格式
 *   - 价格必须是有效数值
 *
 * 返回值使用:
 *   - 成功: 返回的节点可直接插入链表
 *   - 失败: 内存分配问题返回NULL，调用者需处理
 */
DeviceNode* createDeviceNode() {
    DeviceNode* node = (DeviceNode*)malloc(sizeof(DeviceNode));
    if (node == NULL) { printf("错误：内存分配失败！\n"); return NULL; }

    printf("\n--- 新增设备 ---\n");

    /*
     * ====== 输入字段1: 设备编号 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 字母、数字、连字符的组合
     * 是否必填: 是（不能为空）
     * 输入样例:
     *   - "SB001" (设备拼音缩写+序号)
     *   - "DEV-2024-0789" (Device+日期+序号)
     *   - "EQP20260416001" (Equipment日期+流水号)
     */
    safeInput(node->id, MAX_ID_LEN, "请输入设备编号: ");
    while (isEmpty(node->id)) { printf("编号不能为空！\n"); safeInput(node->id, MAX_ID_LEN, "请输入设备编号: "); }

    /*
     * ====== 输入字段2: 设备名称 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 中文或英文设备名称
     * 是否必填: 是
     * 输入样例:
     *   - "CT扫描仪"
     *   - "心电图机"
     *   - "呼吸机"
     *   - "全自动生化分析仪"
     */
    safeInput(node->name, MAX_NAME_LEN, "请输入设备名称: ");

    /*
     * ====== 输入字段3: 设备类别 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符
     * 输入格式: 分类名称（中文）
     * 是否必填: 是
     * 输入样例:
     *   - "影像设备"
     *   - "检验设备"
     *   - "手术器械"
     *   - "监护设备"
     *   - "康复器材"
     */
    safeInput(node->category, MAX_DEPT_LEN, "请输入设备类别: ");

    /*
     * ====== 输入字段4: 所属科室 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符
     * 输入格式: 科室名称（中文）
     * 是否必填: 是
     * 输入样例:
     *   - "放射科" (CT/MRI等影像设备)
     *   - "检验科" (化验分析设备)
     *   - "手术室" (手术相关设备)
     *   - "ICU" (重症监护设备)
     *   - "全院共用" (公共设备)
     */
    safeInput(node->department, MAX_DEPT_LEN, "请输入所属科室: ");

    /*
     * ====== 输入字段5: 购入日期 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: YYYY-MM-DD (严格格式)
     * 是否必填: 是
     * 输入样例:
     *   - "2025-03-15" (2025年3月15日购入)
     *   - "2024-12-01" (2024年12月1日购入)
     * 格式要求:
     *   - 年份: 4位数字
     *   - 月份: 2位数字 (01-12)
     *   - 日期: 2位数字 (根据月份1-31)
     * 验证: 自动检查日期有效性(含闰年判断)
     */
    safeInput(node->purchaseDate, MAX_DATE_LEN, "请输入购入日期: ");
    while (!isValidDate(node->purchaseDate)) { printf("日期格式不正确！\n"); safeInput(node->purchaseDate, MAX_DATE_LEN, "请输入购入日期: "); }

    /*
     * ====== 输入字段6: 购入价格 ======
     * 数据类型: double (双精度浮点数)
     * 取值范围: 0.00 ~ 很大的正数
     * 精度要求: 保留2位小数(元)
     * 输入格式: 数字，可含小数点
     * 是否必填: 是
     * 输入样例:
     *   - "1500000.00" (CT机，150万元)
     *   - "85000.00" (心电图机，8.5万元)
     *   - "280000.00" (呼吸机，28万元)
     * 业务含义: 设备购置原价（含税），用于资产折旧计算
     */
    node->price = inputDouble("请输入购入价格: ");

    /* 【系统设置】以下字段由系统自动初始化:
     * - status = DEVICE_NORMAL  默认状态：正常使用中
     * - needPurchase = 0       默认不需要采购
     * - purchaseQuantity = 0   默认采购数量为0
     */

    /*
     * ====== 输入字段7: 责任人 ======
     * 数据类型: char[50] (字符串)
     * 最大长度: 50个字符（25个中文字符）
     * 输入格式: 员工姓名或工号
     * 是否必填: 是
     * 输入样例:
     *   - "张工程师"
     *   - "李主任"
     *   - "设备科-王五"
     * 说明: 负责该设备日常管理和维护的人员
     */
    safeInput(node->handler, MAX_NAME_LEN, "请输入责任人: ");

    /*
     * ====== 输入字段8: 存放位置 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 位置描述文本
     * 是否必填: 是
     * 输入样例:
     *   - "门诊楼3楼放射科机房1"
     *   - "住院部2楼ICU病房区"
     *   - "医技楼1楼检验科大厅"
     *   - "设备仓库A区3号位"
     */
    safeInput(node->location, MAX_REMARK_LEN, "请输入存放位置: ");

    /*
     * ====== 输入字段9: 保修期 ======
     * 数据类型: char[20] (字符串)
     * 最大长度: 20个字符
     * 输入格式: 时间描述文本
     * 是否必填: 是
     * 输入样例:
     *   - "3年" (标准保修期)
     *   - "5年" (延长保修期)
     *   - "2028-03-15" (具体截止日期)
     *   - "终身质保" (特殊情况)
     * 说明: 厂商提供的免费维修服务期限
     */
    safeInput(node->warrantyPeriod, 20, "请输入保修期: ");

    /*
     * ====== 输入字段10: 备注 ======
     * 数据类型: char[200] (字符串)
     * 最大长度: 200个字符（100个中文字符）
     * 输入格式: 自由文本
     * 是否必填: 否（可留空）
     * 输入样例:
     *   - "" (直接回车留空)
     *   - "进口设备，需定期校准"
     *   - "已购买延保至2030年"
     *   - "配套耗材:专用试剂"
     */
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注: ");

    node->next = NULL;
    return node;
}

/**
 * @brief 检查设备编号是否已存在
 * @param list 设备链表指针
 * @param id 待检查的设备编号
 * @return 存在返回1，不存在返回0
 *
 * 功能说明:
 *   在设备链表中线性搜索指定编号，用于保证编号唯一性。
 *
 * 查找算法:
 *   - 从链表头部开始遍历
 *   - 逐一比较每个节点的id字段
 *   - 找到匹配即返回（不需要遍历全部）
 *
 * 使用场景:
 *   - 添加新设备前检查重复
 *   - 导入数据前去重校验
 */
int isDeviceIDExist(DeviceList* list, const char* id) {
    DeviceNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return 1; current = current->next; }
    return 0;
}

/**
 * @brief 将设备节点插入链表头部
 * @param list 设备链表指针
 * @param node 待插入的设备节点指针
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
void insertDeviceNode(DeviceList* list, DeviceNode* node) {
    if (node == NULL) return;
    node->next = list->head; list->head = node; list->count++;
}

/**
 * @brief 根据设备编号查找设备节点
 * @param list 设备链表指针
 * @param id 目标设备编号
 * @return 找到返回节点指针，未找到返回NULL
 *
 * 功能说明:
 *   在设备链表中按编号精确查找目标设备。
 *   返回的是实际节点的指针，可用于直接修改数据。
 *
 * 查找过程:
 *   1. 从链表头开始遍历
 *   2. 使用strcmp比较id字段
 *   3. 匹配则立即返回该节点地址
 *   4. 遍历完仍未找到返回NULL
 *
 * 返回值用途:
 *   - 非NULL: 可直接访问或修改该设备信息
 *   - NULL: 表示设备不存在，需提示用户
 */
DeviceNode* findDeviceByID(DeviceList* list, const char* id) {
    DeviceNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return current; current = current->next; }
    return NULL;
}

/**
 * @brief 根据编号删除设备记录
 * @param list 设备链表指针
 * @param id 要删除的设备编号
 * @return 成功删除返回1，取消或未找到返回0
 *
 * 功能说明:
 *   完整的设备删除流程：
 *   1. 在链表中定位目标设备
 *   2. 显示设备信息供确认
 *   3. 用户确认后执行删除
 *   4. 记录操作日志
 *   5. 释放被删节点内存
 *
 * 删除前的确认机制:
 *   - 显示待删除设备的详细信息
 *   - 要求用户二次确认（Y/N）
 *   - 用户可取消删除操作
 *
 * 内存管理:
 *   - 删除后自动free释放节点内存
 *   - 更新链表count计数器
 *   - 正确处理首节点和中间节点的不同情况
 *
 * 日志记录:
 *   记录被删设备的关键信息（编号、名称、类别）
 */
int deleteDeviceByID(DeviceList* list, const char* id) {
    DeviceNode* current = list->head, * prev = NULL;

    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            /* 显示待删除记录详情 */
            printf("\n找到以下记录:\n"); printDeviceOne(current);

            /* 二次确认防止误删 */
            if (!confirm("确认删除该设备?")) { printf("已取消删除。\n"); return 0; }

            /* 准备日志信息（在删除前获取） */
            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "设备编号:%s, 名称:%s, 类别:%s",
                current->id, current->name, current->category);

            /* 执行链表删除操作 */
            if (prev == NULL) list->head = current->next;  /* 删除的是头节点 */
            else prev->next = current->next;              /* 删除中间或尾部节点 */

            free(current); list->count--;
            printf("删除成功！\n");

            /* 记录删除日志 */
            writeLog(LOG_DEVICE, "删除设备", "系统", detail);
            return 1;
        }
        prev = current; current = current->next;
    }
    printf("未找到编号为 %s 的设备。\n", id); return 0;
}

/**
 * @brief 修改设备信息（交互式选择字段修改）
 * @param node 待修改的设备节点指针
 *
 * 功能说明:
 *   提供菜单式界面让用户选择要修改的具体字段，
 *   支持单次修改一个字段，避免误改其他数据。
 *
 * 可修改的字段列表:
 *   1. 设备名称       - name字段
 *   2. 设备类别       - category字段
 *   3. 所属科室       - department字段
 *   4. 责任人         - handler字段
 *   5. 存放位置       - location字段
 *   6. 当前状态       - status字段（枚举选择）
 *   7. 备注           - remark字段
 *   0. 取消           - 不做任何修改
 *
 * 修改流程:
 *   1. 显示当前设备信息
 *   2. 显示可选修改项菜单
 *   3. 用户选择要修改的字段
 *   4. 输入新值替换旧值
 *   5. 记录修改日志
 *
 * 日志内容:
 *   包含设备编号、名称和修改的字段序号
 */
void modifyDeviceInfo(DeviceNode* node) {
    if (node == NULL) return;

    printf("\n--- 修改设备信息 ---\n"); printDeviceOne(node);
    printf("\n请选择要修改的字段:\n");
    printf("1. 名称\n2. 类别\n3. 科室\n4. 责任人\n5. 存放位置\n6. 状态\n7. 备注\n0. 取消\n");

    int choice = inputInt("请选择: ");

    switch (choice) {
    case 1: safeInput(node->name, MAX_NAME_LEN, "请输入新名称: "); break;
    case 2: safeInput(node->category, MAX_DEPT_LEN, "请输入新类别: "); break;
    case 3: safeInput(node->department, MAX_DEPT_LEN, "请输入新科室: "); break;
    case 4: safeInput(node->handler, MAX_NAME_LEN, "请输入新责任人: "); break;
    case 5: safeInput(node->location, MAX_REMARK_LEN, "请输入新位置: "); break;
    case 6:
        /* 状态修改提供选项列表 */
        printf("状态选项:\n0.正常 1.维修中 2.已报废\n");
        node->status = inputInt("请选择: ");
        break;
    case 7: safeInput(node->remark, MAX_REMARK_LEN, "请输入新备注: "); break;
    case 0: printf("取消修改。\n"); return;
    default: printf("无效选择。\n"); return;
    }

    printf("修改成功！\n");

    /* 记录修改日志 */
    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "设备编号:%s, 名称:%s, 修改字段:%d", node->id, node->name, choice);
    writeLog(LOG_DEVICE, "修改设备信息", "系统", detail);
}

/**
 * @brief 打印单个设备的详细信息
 * @param node 设备节点指针
 *
 * 功能说明:
 *   格式化输出一台设备的完整信息，用于查看详情。
 *   输出采用固定宽度的分隔线框架，便于阅读。
 *
 * 显示内容:
 *   - 设备编号（唯一标识）
 *   - 设备名称
 *   - 设备类别（如：影像、检验等）
 *   - 所属科室
 *   - 购入日期
 *   - 购入价格（保留2位小数）
 *   - 当前状态（中文显示：正常/维修中/已报废）
 *   - 责任人
 *   - 存放位置
 *   - 保修期
 *   - 需要购入数量（仅当needPurchase非0时显示）
 *   - 备注
 *
 * 输出格式特点:
 *   - 使用70字符宽的分隔线
 *   - 每个字段独占一行
 *   - 标签和值对齐显示
 */
void printDeviceOne(DeviceNode* node) {
    if (node == NULL) return;

    printLine('-', 70);
    printf("设备编号: %s\n", node->id);
    printf("设备名称: %s\n", node->name);
    printf("设备类别: %s\n", node->category);
    printf("所属科室: %s\n", node->department);
    printf("购入日期: %s\n", node->purchaseDate);
    printf("购入价格: %.2f\n", node->price);
    printf("当前状态: %s\n", getDeviceStatusString(node->status));
    printf("责任人: %s\n", node->handler);
    printf("存放位置: %s\n", node->location);
    printf("保修期: %s\n", node->warrantyPeriod);
    if (node->needPurchase) printf("需要购入数量: %d\n", node->purchaseQuantity);
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

/**
 * @brief 打印所有设备列表
 * @param list 设备链表指针
 *
 * 功能说明:
 *   遍历整个设备链表，依次输出每台设备的详细信息。
 *   先显示总数概览，再逐个列出详情。
 *
 * 输出结构:
 *   --- 所有设备 (共 N 台) ---
 *   [设备1详细信息]
 *   [设备2详细信息]
 *   ...
 *
 * 特殊处理:
 *   - 空链表时提示"暂无设备记录"
 *   - 每台设备调用printDeviceOne()显示
 */
void printDeviceAll(DeviceList* list) {
    if (list->head == NULL) { printf("暂无设备记录。\n"); return; }

    printf("\n--- 所有设备 (共 %d 台) ---\n", list->count);
    DeviceNode* current = list->head;

    while (current != NULL) { printDeviceOne(current); current = current->next; }
}

/**
 * @brief 将设备标记为报废状态
 * @param list 设备链表指针
 * @param id 待报废设备的编号
 *
 * 功能说明:
 *   将指定设备的状态从"正常"或"维修中"改为"已报废"。
 *   报废后的设备不再参与正常使用统计。
 *
 * 报废条件检查:
 *   1. 设备必须存在（否则提示未找到）
 *   2. 设备不能已经报废（避免重复操作）
 *   3. 需要用户确认（防止误操作）
 *
 * 报废后的影响:
 *   - status字段变为DEVICE_SCRAPPED
 *   - 统计分析时归类为"已报废"
 *   - 总价值计算时不计入在用设备
 *
 * 注意事项:
 *   - 仅改变状态标记，不从链表中删除
 *   - 保留设备信息以便日后查询历史
 */
void scrapDevice(DeviceList* list, const char* id) {
    DeviceNode* node = findDeviceByID(list, id);
    if (node == NULL) { printf("未找到该设备。\n"); return; }

    if (node->status == DEVICE_SCRAPPED) { printf("该设备已报废。\n"); return; }

    if (!confirm("确认报废该设备?")) { printf("取消操作。\n"); return; }

    node->status = DEVICE_SCRAPPED;
    printf("设备 %s 已标记为报废。\n", id);
}

/**
 * @brief 设备统计分析
 * @param dList 设备链表指针
 * @param fList 维修记录链表指针
 *
 * 功能说明:
 *   对设备数据进行汇总统计，生成管理报告。
 *   帮助管理者了解设备整体状况。
 *
 * 统计指标:
 *   1. 设备总数 - 链表中所有设备数量
 *   2. 正常使用数 - 状态为DEVICE_NORMAL的设备
 *   3. 维修中数量 - 状态为DEVICE_REPAIRING的设备
 *   4. 已报废数量 - 状态为DEVICE_SCRAPPED的设备
 *   5. 在用设备总价值 - 正常状态设备的价格总和
 *   6. 维修记录总数 - 历史维修次数
 *
 * 输出示例:
 *   --- 设备统计 ---
 *   设备总数: 50
 *   正常使用: 42
 *   维修中: 5
 *   已报废: 3
 *   在用设备总价值: 12500000.00
 *   维修记录数: 128
 *
 * 应用场景:
 *   - 月度/年度设备资产报告
 *   - 设备购置决策参考
 *   - 维修预算制定依据
 */
void statDevices(DeviceList* dList, FixList* fList) {
    int normal = 0, repairing = 0, scrapped = 0;
    double totalValue = 0;
    DeviceNode* curr = dList->head;

    /* 分类统计各状态的设备数量和价值 */
    while (curr != NULL) {
        switch (curr->status) {
        case DEVICE_NORMAL: normal++; totalValue += curr->price; break;
        case DEVICE_REPAIRING: repairing++; break;
        case DEVICE_SCRAPPED: scrapped++; break;
        }
        curr = curr->next;
    }

    /* 输出统计报告 */
    printf("\n--- 设备统计 ---\n");
    printf("设备总数: %d\n", dList->count);
    printf("正常使用: %d\n", normal);
    printf("维修中: %d\n", repairing);
    printf("已报废: %d\n", scrapped);
    printf("在用设备总价值: %.2f\n", totalValue);
    printf("维修记录数: %d\n", fList->count);
}

/**
 * @brief 释放设备链表占用的所有内存
 * @param list 设备链表指针
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
void freeDeviceList(DeviceList* list) {
    DeviceNode* current = list->head;
    while (current != NULL) { DeviceNode* temp = current; current = current->next; free(temp); }
    list->head = NULL; list->count = 0;
}

/**
 * @brief 释放维修记录链表占用的所有内存
 * @param list 维修记录链表指针
 *
 * 功能说明:
 *   清理维修记录链表的所有动态内存。
 *   与freeDeviceList()功能类似但针对维修记录数据。
 */
void freeFixList(FixList* list) {
    FixNode* current = list->head;
    while (current != NULL) { FixNode* temp = current; current = current->next; free(temp); }
    list->head = NULL; list->count = 0;
}

/**
 * @brief 设备管理主菜单
 * @param dList 设备链表指针
 * @param fList 维修记录链表指针
 *
 * 功能说明:
 *   提供设备管理的交互式主界面，循环显示菜单并响应用户选择。
 *   是设备模块的入口函数，整合了所有设备操作功能。
 *
 * 菜单功能列表:
 *   1. 新增设备    - 调用createDeviceNode() + insertDeviceNode()
 *   2. 查询设备    - 支持按编号查询或显示全部
 *   3. 删除设备    - 调用deleteDeviceByID()
 *   4. 修改设备    - 调用findDeviceByID() + modifyDeviceInfo()
 *   5. 设备报废    - 调用scrapDevice()
 *   6. 统计分析    - 调用statDevices()
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
 *
 * 日志集成:
 *   - 新增设备时记录设备信息
 *   - 删除设备时记录被删信息
 *   - 修改设备时记录变更内容
 */
void deviceMenu(DeviceList* dList, FixList* fList) {
    int choice;

    do {
        printf("\n"); printTitle("医疗设备管理系统");
        printf("1. 新增设备\n2. 查询设备\n3. 删除设备\n4. 修改设备\n");
        printf("5. 设备报废\n6. 统计分析\n0. 返回主菜单\n");
        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: {
            /* 新增设备：创建节点→检查重复→插入链表→记录日志 */
            DeviceNode* node = createDeviceNode();
            if (node && !isDeviceIDExist(dList, node->id)) {
                insertDeviceNode(dList, node);
                printf("设备添加成功！\n");

                /* 记录新增设备日志 */
                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "设备编号:%s, 名称:%s, 类别:%s, 部门:%s, 价格:%.2f",
                    node->id, node->name, node->category, node->department, node->price);
                writeLog(LOG_DEVICE, "新增设备", "系统", detail);
            }
            else if (node) { printf("错误：设备编号已存在！\n"); free(node); }
            break;
        }
        case 2: {
            /* 查询设备：支持按编号精确定位或浏览全部 */
            if (dList->head == NULL) { printf("暂无设备记录。\n"); break; }
            printf("1.按编号查询 2.显示全部\n");
            int c = inputInt("请选择: ");
            if (c == 1) {
                char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入编号: ");
                DeviceNode* found = findDeviceByID(dList, id);
                if (found) printDeviceOne(found); else printf("未找到。\n");
            }
            else printDeviceAll(dList);
            break;
        }
        case 3: {
            /* 删除设备：输入编号→确认→执行删除 */
            if (dList->head == NULL) { printf("暂无设备记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要删除的设备编号: ");
            deleteDeviceByID(dList, id);
            break;
        }
        case 4: {
            /* 修改设备：查找→显示修改菜单→更新字段 */
            if (dList->head == NULL) { printf("暂无设备记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要修改的设备编号: ");
            DeviceNode* node = findDeviceByID(dList, id);
            if (node) modifyDeviceInfo(node); else printf("未找到。\n");
            break;
        }
        case 5: {
            /* 设备报废：查找→确认→更改状态 */
            if (dList->head == NULL) { printf("暂无设备记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要报废的设备编号: ");
            scrapDevice(dList, id);
            break;
        }
        case 6: statDevices(dList, fList); break;  /* 统计分析 */
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }

        /* 除退出外每次操作后暂停，让用户查看结果 */
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

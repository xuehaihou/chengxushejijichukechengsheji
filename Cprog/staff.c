#include "staff.h"
#include "log.h"

// 获取工作状态字符串
const char* getStatusString(int status) {
    switch (status) {
    case STATUS_WORKING: return "在职";
    case STATUS_LEAVE: return "请假";
    case STATUS_RESIGNED: return "离职";
    case STATUS_TRANSFERRED: return "调岗";
    default: return "未知";
    }
}

// 初始化职工链表
void initStaffList(StaffList* list) {
    list->head = NULL;
    list->count = 0;
}

// 从文件加载职工数据
void loadStaffFromFile(StaffList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("提示：职工数据文件不存在，将创建新文件。\n");
        return;
    }

    StaffNode* tail = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        StaffNode* node = (StaffNode*)malloc(sizeof(StaffNode));
        if (node == NULL) continue;

        // 解析数据行
        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%lf|%d|%[^\n]",
            node->id, node->name, node->gender, &node->age,
            node->phone, node->department, node->position,
            node->hireDate, &node->salary, &node->status, node->remark);

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
    printf("成功加载 %d 条职工记录。\n", list->count);
}

// 保存职工数据到文件
void saveStaffToFile(StaffList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法打开文件 %s 进行写入！\n", filename);
        return;
    }

    StaffNode* current = list->head;
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

// 创建新职工结点
StaffNode* createStaffNode() {
    StaffNode* node = (StaffNode*)malloc(sizeof(StaffNode));
    if (node == NULL) {
        printf("错误：内存分配失败！\n");
        return NULL;
    }

    printf("\n--- 新增职工 ---\n");

    // 输入工号
    safeInput(node->id, MAX_ID_LEN, "请输入工号: ");
    while (isEmpty(node->id)) {
        printf("工号不能为空！\n");
        safeInput(node->id, MAX_ID_LEN, "请输入工号: ");
    }

    safeInput(node->name, MAX_NAME_LEN, "请输入姓名: ");
    safeInput(node->gender, 10, "请输入性别 (男/女): ");
    node->age = inputInt("请输入年龄: ");

    safeInput(node->phone, MAX_PHONE_LEN, "请输入电话: ");
    while (!isValidPhone(node->phone)) {
        printf("电话号码格式不正确！\n");
        safeInput(node->phone, MAX_PHONE_LEN, "请输入电话: ");
    }

    safeInput(node->department, MAX_DEPT_LEN, "请输入所属部门: ");
    safeInput(node->position, MAX_POSITION_LEN, "请输入岗位: ");

    safeInput(node->hireDate, MAX_DATE_LEN, "请输入入职日期 (YYYY-MM-DD): ");
    while (!isValidDate(node->hireDate)) {
        printf("日期格式不正确！\n");
        safeInput(node->hireDate, MAX_DATE_LEN, "请输入入职日期 (YYYY-MM-DD): ");
    }

    node->salary = inputDouble("请输入基本工资: ");
    while (node->salary < 0) {
        printf("工资不能为负数！\n");
        node->salary = inputDouble("请输入基本工资: ");
    }

    node->status = STATUS_WORKING;
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注 (无则留空): ");

    node->next = NULL;
    return node;
}

// 检查工号是否重复
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

// 插入职工结点
void insertStaffNode(StaffList* list, StaffNode* node) {
    if (node == NULL) return;

    node->next = list->head;
    list->head = node;
    list->count++;
}

// 按工号查找职工
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

// 按姓名查找职工
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

// 删除职工结点
int deleteStaffByID(StaffList* list, const char* id) {
    StaffNode* current = list->head;
    StaffNode* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printf("\n找到以下记录:\n");
            printStaffOne(current);

            if (!confirm("确认删除该职工?")) {
                printf("已取消删除。\n");
                return 0;
            }

            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "工号:%s, 姓名:%s, 部门:%s",
                current->id, current->name, current->department);

            if (prev == NULL) {
                list->head = current->next;
            }
            else {
                prev->next = current->next;
            }

            free(current);
            list->count--;
            printf("删除成功！\n");

            writeLog(LOG_STAFF, "删除职工", "系统", detail);
            return 1;
        }
        prev = current;
        current = current->next;
    }

    printf("未找到工号为 %s 的职工。\n", id);
    return 0;
}

// 修改职工信息
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
        printf("工作状态选项:\n");
        printf("0. 在职\n");
        printf("1. 请假\n");
        printf("2. 离职\n");
        printf("3. 调岗\n");
        node->status = inputInt("请选择新状态: ");
        if (node->status < 0 || node->status > 3) {
            node->status = STATUS_WORKING;
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

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "工号:%s, 姓名:%s, 修改字段:%d", node->id, node->name, choice);
    writeLog(LOG_STAFF, "修改职工信息", "系统", detail);
}

// 显示单个职工信息
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

// 显示所有职工信息
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

// 按工号排序
void sortStaffByID(StaffList* list) {
    if (list->head == NULL || list->head->next == NULL) return;

    for (StaffNode* p = list->head; p != NULL; p = p->next) {
        for (StaffNode* q = p->next; q != NULL; q = q->next) {
            if (strcmp(p->id, q->id) > 0) {
                // 交换数据
                StaffNode temp = *p;
                *p = *q;
                *q = temp;
                // 恢复指针
                StaffNode* tempNext = p->next;
                p->next = q->next;
                q->next = tempNext;
            }
        }
    }
}

// 按工资排序
void sortStaffBySalary(StaffList* list) {
    if (list->head == NULL || list->head->next == NULL) return;

    for (StaffNode* p = list->head; p != NULL; p = p->next) {
        for (StaffNode* q = p->next; q != NULL; q = q->next) {
            if (p->salary < q->salary) {
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

// 按年龄排序
void sortStaffByAge(StaffList* list) {
    if (list->head == NULL || list->head->next == NULL) return;

    for (StaffNode* p = list->head; p != NULL; p = p->next) {
        for (StaffNode* q = p->next; q != NULL; q = q->next) {
            if (p->age > q->age) {
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

// 按入职日期排序
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

// 统计各部门人数
void countStaffByDept(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    printf("\n--- 各部门人数统计 ---\n");

    // 使用简单数组存储部门统计
    typedef struct {
        char dept[MAX_DEPT_LEN];
        int count;
    } DeptStat;

    DeptStat stats[100];
    int statCount = 0;

    StaffNode* current = list->head;
    while (current != NULL) {
        int found = 0;
        for (int i = 0; i < statCount; i++) {
            if (strcmp(stats[i].dept, current->department) == 0) {
                stats[i].count++;
                found = 1;
                break;
            }
        }
        if (!found && statCount < 100) {
            strcpy(stats[statCount].dept, current->department);
            stats[statCount].count = 1;
            statCount++;
        }
        current = current->next;
    }

    printLine('-', 40);
    printf("%-20s %s\n", "部门", "人数");
    printLine('-', 40);
    for (int i = 0; i < statCount; i++) {
        printf("%-20s %d\n", stats[i].dept, stats[i].count);
    }
    printLine('-', 40);
}

// 统计各岗位人数
void countStaffByPosition(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    printf("\n--- 各岗位人数统计 ---\n");

    typedef struct {
        char position[MAX_POSITION_LEN];
        int count;
    } PositionStat;

    PositionStat stats[100];
    int statCount = 0;

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

    printLine('-', 40);
    printf("%-20s %s\n", "岗位", "人数");
    printLine('-', 40);
    for (int i = 0; i < statCount; i++) {
        printf("%-20s %d\n", stats[i].position, stats[i].count);
    }
    printLine('-', 40);
}

// 统计在职/离职人数
void countStaffByStatus(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    int working = 0, leave = 0, resigned = 0, transferred = 0;

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

// 计算平均工资
void calculateAvgSalary(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    double total = 0;
    int workingCount = 0;

    StaffNode* current = list->head;
    while (current != NULL) {
        if (current->status == STATUS_WORKING) {
            total += current->salary;
            workingCount++;
        }
        current = current->next;
    }

    printf("\n--- 工资统计 ---\n");
    printf("在职员工数: %d\n", workingCount);
    printf("工资总额: %.2f\n", total);
    if (workingCount > 0) {
        printf("平均工资: %.2f\n", total / workingCount);
    }
}

// 释放职工链表内存
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

// 新增职工
void addStaff(StaffList* list) {
    StaffNode* node = createStaffNode();
    if (node == NULL) return;

    // 检查工号是否重复
    if (isStaffIDExist(list, node->id)) {
        printf("错误：工号 %s 已存在！\n", node->id);
        free(node);
        return;
    }

    insertStaffNode(list, node);
    printf("职工添加成功！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "工号:%s, 姓名:%s, 部门:%s, 岗位:%s",
        node->id, node->name, node->department, node->position);
    writeLog(LOG_STAFF, "新增职工", "系统", detail);
}

// 查询职工
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
        printStaffAll(list);
        break;
    case 0:
        return;
    default:
        printf("无效选择。\n");
    }
}

// 删除职工
void deleteStaff(StaffList* list) {
    if (list->head == NULL) {
        printf("暂无职工记录。\n");
        return;
    }

    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入要删除的职工工号: ");
    deleteStaffByID(list, id);
}

// 修改职工
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

// 排序显示职工
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

    printStaffAll(list);
}

// 统计职工
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

// 职工管理菜单
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

        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}
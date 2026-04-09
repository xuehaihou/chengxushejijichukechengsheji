#include "finance.h"
#include "log.h"

// 获取收支类型字符串
const char* getFinanceTypeString(int type) {
    switch (type) {
    case TYPE_INCOME: return "收入";
    case TYPE_EXPENSE: return "支出";
    default: return "未知";
    }
}

// 获取审核状态字符串
const char* getAuditStatusString(int status) {
    switch (status) {
    case AUDIT_PENDING: return "未审核";
    case AUDIT_APPROVED: return "已审核";
    case AUDIT_REJECTED: return "已作废";
    default: return "未知";
    }
}

// 初始化财务链表
void initFinanceList(FinanceList* list) {
    list->head = NULL;
    list->count = 0;
}

// 从文件加载财务数据
void loadFinanceFromFile(FinanceList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("提示：财务数据文件不存在，将创建新文件。\n");
        return;
    }

    FinanceNode* tail = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        FinanceNode* node = (FinanceNode*)malloc(sizeof(FinanceNode));
        if (node == NULL) continue;

        sscanf(line, "%[^|]|%d|%lf|%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^\n]",
            node->id, &node->type, &node->amount, node->date,
            node->handler, node->department, node->purpose,
            &node->auditStatus, node->remark);

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
    printf("成功加载 %d 条财务记录。\n", list->count);
}

// 保存财务数据到文件
void saveFinanceToFile(FinanceList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法打开文件 %s 进行写入！\n", filename);
        return;
    }

    FinanceNode* current = list->head;
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

// 创建新财务记录结点
FinanceNode* createFinanceNode() {
    FinanceNode* node = (FinanceNode*)malloc(sizeof(FinanceNode));
    if (node == NULL) {
        printf("错误：内存分配失败！\n");
        return NULL;
    }

    printf("\n--- 新增财务记录 ---\n");

    safeInput(node->id, MAX_ID_LEN, "请输入记录编号: ");
    while (isEmpty(node->id)) {
        printf("编号不能为空！\n");
        safeInput(node->id, MAX_ID_LEN, "请输入记录编号: ");
    }

    printf("收支类型:\n");
    printf("0. 收入\n");
    printf("1. 支出\n");
    node->type = inputInt("请选择: ");
    while (node->type != 0 && node->type != 1) {
        printf("无效选择！\n");
        node->type = inputInt("请选择: ");
    }

    node->amount = inputDouble("请输入金额: ");
    while (node->amount < 0) {
        printf("金额不能为负数！\n");
        node->amount = inputDouble("请输入金额: ");
    }

    safeInput(node->date, MAX_DATE_LEN, "请输入日期 (YYYY-MM-DD): ");
    while (!isValidDate(node->date)) {
        printf("日期格式不正确！\n");
        safeInput(node->date, MAX_DATE_LEN, "请输入日期 (YYYY-MM-DD): ");
    }

    safeInput(node->handler, MAX_NAME_LEN, "请输入经办人: ");
    safeInput(node->department, MAX_DEPT_LEN, "请输入所属部门: ");
    safeInput(node->purpose, MAX_REMARK_LEN, "请输入用途/来源: ");

    node->auditStatus = AUDIT_PENDING;
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注 (无则留空): ");

    node->next = NULL;
    return node;
}

// 检查记录编号是否重复
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

// 插入财务记录结点
void insertFinanceNode(FinanceList* list, FinanceNode* node) {
    if (node == NULL) return;

    node->next = list->head;
    list->head = node;
    list->count++;
}

// 按编号查找财务记录
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

// 删除财务记录
int deleteFinanceByID(FinanceList* list, const char* id) {
    FinanceNode* current = list->head;
    FinanceNode* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printf("\n找到以下记录:\n");
            printFinanceOne(current);

            if (!confirm("确认删除该记录?")) {
                printf("已取消删除。\n");
                return 0;
            }

            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "编号:%s, 类型:%s, 金额:%.2f",
                current->id, getFinanceTypeString(current->type), current->amount);

            if (prev == NULL) {
                list->head = current->next;
            }
            else {
                prev->next = current->next;
            }

            free(current);
            list->count--;
            printf("删除成功！\n");

            writeLog(LOG_FINANCE, "删除财务记录", "系统", detail);
            return 1;
        }
        prev = current;
        current = current->next;
    }

    printf("未找到编号为 %s 的记录。\n", id);
    return 0;
}

// 修改财务记录
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

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "编号:%s, 类型:%s, 金额:%.2f, 修改字段:%d",
        node->id, getFinanceTypeString(node->type), node->amount, choice);
    writeLog(LOG_FINANCE, "修改财务记录", "系统", detail);
}

// 显示单个财务记录
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

// 显示所有财务记录
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

// 按日期范围查询
void queryFinanceByDate(FinanceList* list, const char* startDate, const char* endDate) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    int count = 0;
    FinanceNode* current = list->head;

    printf("\n--- %s 至 %s 的财务记录 ---\n", startDate, endDate);
    while (current != NULL) {
        if (compareDate(current->date, startDate) >= 0 &&
            compareDate(current->date, endDate) <= 0) {
            printFinanceOne(current);
            count++;
        }
        current = current->next;
    }

    if (count == 0) {
        printf("该日期范围内无记录。\n");
    }
    else {
        printf("共找到 %d 条记录。\n", count);
    }
}

// 按部门查询
void queryFinanceByDept(FinanceList* list, const char* dept) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    int count = 0;
    FinanceNode* current = list->head;

    printf("\n--- 部门 [%s] 的财务记录 ---\n", dept);
    while (current != NULL) {
        if (strstr(current->department, dept) != NULL) {
            printFinanceOne(current);
            count++;
        }
        current = current->next;
    }

    if (count == 0) {
        printf("该部门无记录。\n");
    }
    else {
        printf("共找到 %d 条记录。\n", count);
    }
}

// 按经办人查询
void queryFinanceByHandler(FinanceList* list, const char* handler) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    int count = 0;
    FinanceNode* current = list->head;

    printf("\n--- 经办人 [%s] 的财务记录 ---\n", handler);
    while (current != NULL) {
        if (strstr(current->handler, handler) != NULL) {
            printFinanceOne(current);
            count++;
        }
        current = current->next;
    }

    if (count == 0) {
        printf("该经办人无记录。\n");
    }
    else {
        printf("共找到 %d 条记录。\n", count);
    }
}

// 按类型查询
void queryFinanceByType(FinanceList* list, int type) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    int count = 0;
    FinanceNode* current = list->head;

    printf("\n--- %s记录 ---\n", getFinanceTypeString(type));
    while (current != NULL) {
        if (current->type == type) {
            printFinanceOne(current);
            count++;
        }
        current = current->next;
    }

    if (count == 0) {
        printf("无%s记录。\n", getFinanceTypeString(type));
    }
    else {
        printf("共找到 %d 条记录。\n", count);
    }
}

// 计算总收入
void calculateTotalIncome(FinanceList* list) {
    double total = 0;
    FinanceNode* current = list->head;

    while (current != NULL) {
        if (current->type == TYPE_INCOME && current->auditStatus == AUDIT_APPROVED) {
            total += current->amount;
        }
        current = current->next;
    }

    printf("总收入 (已审核): %.2f\n", total);
}

// 计算总支出
void calculateTotalExpense(FinanceList* list) {
    double total = 0;
    FinanceNode* current = list->head;

    while (current != NULL) {
        if (current->type == TYPE_EXPENSE && current->auditStatus == AUDIT_APPROVED) {
            total += current->amount;
        }
        current = current->next;
    }

    printf("总支出 (已审核): %.2f\n", total);
}

// 计算结余
void calculateBalance(FinanceList* list) {
    double income = 0, expense = 0;
    FinanceNode* current = list->head;

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

    printf("\n--- 财务汇总 (已审核) ---\n");
    printf("总收入: %.2f\n", income);
    printf("总支出: %.2f\n", expense);
    printf("结余: %.2f\n", income - expense);
}

// 按月统计
void statFinanceByMonth(FinanceList* list, const char* yearMonth) {
    double income = 0, expense = 0;
    int incomeCount = 0, expenseCount = 0;

    FinanceNode* current = list->head;
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

    printf("\n--- %s 财务统计 ---\n", yearMonth);
    printf("收入: %.2f (%d笔)\n", income, incomeCount);
    printf("支出: %.2f (%d笔)\n", expense, expenseCount);
    printf("结余: %.2f\n", income - expense);
}

// 释放财务链表内存
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

// 新增财务记录
void addFinance(FinanceList* list) {
    FinanceNode* node = createFinanceNode();
    if (node == NULL) return;

    if (isFinanceIDExist(list, node->id)) {
        printf("错误：编号 %s 已存在！\n", node->id);
        free(node);
        return;
    }

    insertFinanceNode(list, node);
    printf("财务记录添加成功！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "编号:%s, 类型:%s, 金额:%.2f, 部门:%s",
        node->id, getFinanceTypeString(node->type), node->amount, node->department);
    writeLog(LOG_FINANCE, "新增财务记录", "系统", detail);
}

// 查询财务记录
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
        safeInput(startDate, MAX_DATE_LEN, "请输入开始日期 (YYYY-MM-DD): ");
        safeInput(endDate, MAX_DATE_LEN, "请输入结束日期 (YYYY-MM-DD): ");
        queryFinanceByDate(list, startDate, endDate);
        break;
    case 3:
        safeInput(keyword, MAX_DEPT_LEN, "请输入部门: ");
        queryFinanceByDept(list, keyword);
        break;
    case 4:
        safeInput(keyword, MAX_NAME_LEN, "请输入经办人: ");
        queryFinanceByHandler(list, keyword);
        break;
    case 5:
        printf("类型:\n");
        printf("0. 收入\n");
        printf("1. 支出\n");
        int type = inputInt("请选择: ");
        queryFinanceByType(list, type);
        break;
    case 6:
        printFinanceAll(list);
        break;
    case 0:
        return;
    default:
        printf("无效选择。\n");
    }
}

// 删除财务记录
void deleteFinance(FinanceList* list) {
    if (list->head == NULL) {
        printf("暂无财务记录。\n");
        return;
    }

    char id[MAX_ID_LEN];
    safeInput(id, MAX_ID_LEN, "请输入要删除的记录编号: ");
    deleteFinanceByID(list, id);
}

// 修改财务记录
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

// 审核财务记录
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

// 财务报表
void financeReport(FinanceList* list) {
    printf("\n--- 财务报表 ---\n");
    printf("1. 收支汇总\n");
    printf("2. 按月统计\n");
    printf("0. 返回\n");

    int choice = inputInt("请选择: ");
    char yearMonth[10];

    switch (choice) {
    case 1:
        calculateBalance(list);
        break;
    case 2:
        safeInput(yearMonth, 10, "请输入年月 (YYYY-MM): ");
        statFinanceByMonth(list, yearMonth);
        break;
    case 0:
        return;
    default:
        printf("无效选择。\n");
    }
}

// 财务管理菜单
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

        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}
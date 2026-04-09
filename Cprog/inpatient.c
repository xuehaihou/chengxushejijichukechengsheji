#include "inpatient.h"
#include "log.h"

const char* getInpatientStatusString(int status) {
    switch (status) {
    case INPATIENT_ADMITTED: return "在院";
    case INPATIENT_TRANSFERRED: return "转科";
    case INPATIENT_DISCHARGED: return "已出院";
    default: return "未知";
    }
}

void initInpatientList(InpatientList* list) { list->head = NULL; list->count = 0; }

void loadInpatientsFromFile(InpatientList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：住院数据文件不存在，将创建新文件。\n"); return; }
    InpatientNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        InpatientNode* node = (InpatientNode*)malloc(sizeof(InpatientNode));
        if (node == NULL) continue;
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%lf|%lf|%lf|%d|%[^|]|%[^\n]",
            node->id, node->patientId, node->name, node->gender,
            &node->age, node->admitDate, node->department, node->ward,
            node->roomNo, node->bedNo, node->doctor, node->diagnosis,
            &node->deposit, &node->dailyCost, &node->totalCost,
            &node->status, node->dischargeDate, node->remark);
        node->next = NULL;
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条住院记录。\n", list->count);
}

void saveInpatientsToFile(InpatientList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }
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

InpatientNode* createInpatientNode() {
    InpatientNode* node = (InpatientNode*)malloc(sizeof(InpatientNode));
    if (node == NULL) { printf("错误：内存分配失败！\n"); return NULL; }
    printf("\n--- 办理入院 ---\n");

    safeInput(node->id, MAX_ID_LEN, "请输入住院编号: ");
    while (isEmpty(node->id)) { printf("编号不能为空！\n"); safeInput(node->id, MAX_ID_LEN, "请输入住院编号: "); }

    safeInput(node->patientId, MAX_ID_LEN, "请输入病人编号: ");
    safeInput(node->name, MAX_NAME_LEN, "请输入姓名: ");
    safeInput(node->gender, 10, "请输入性别: ");
    node->age = inputInt("请输入年龄: ");

    getCurrentDate(node->admitDate);
    safeInput(node->department, MAX_DEPT_LEN, "请输入科室: ");
    safeInput(node->ward, 20, "请输入病区: ");
    safeInput(node->roomNo, 20, "请输入病房号: ");
    safeInput(node->bedNo, 20, "请输入床位号: ");
    safeInput(node->doctor, MAX_NAME_LEN, "请输入主治医生: ");
    safeInput(node->diagnosis, MAX_REMARK_LEN, "请输入初始诊断: ");

    node->deposit = inputDouble("请输入押金: ");
    node->dailyCost = inputDouble("请输入每日费用: ");
    node->totalCost = 0;
    node->status = INPATIENT_ADMITTED;
    strcpy(node->dischargeDate, "N/A");
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注: ");
    node->next = NULL;
    return node;
}

int isInpatientIDExist(InpatientList* list, const char* id) {
    InpatientNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return 1; current = current->next; }
    return 0;
}

void insertInpatientNode(InpatientList* list, InpatientNode* node) {
    if (node == NULL) return;
    node->next = list->head; list->head = node; list->count++;
}

InpatientNode* findInpatientByID(InpatientList* list, const char* id) {
    InpatientNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return current; current = current->next; }
    return NULL;
}

int deleteInpatientByID(InpatientList* list, const char* id) {
    InpatientNode* current = list->head, * prev = NULL;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printf("\n找到以下记录:\n"); printInpatientOne(current);
            if (!confirm("确认删除该记录?")) { printf("已取消删除。\n"); return 0; }
            if (prev == NULL) list->head = current->next;
            else prev->next = current->next;
            free(current); list->count--; printf("删除成功！\n"); return 1;
        }
        prev = current; current = current->next;
    }
    printf("未找到编号为 %s 的记录。\n", id); return 0;
}

void modifyInpatientInfo(InpatientNode* node) {
    if (node == NULL) return;
    if (node->status == INPATIENT_DISCHARGED) { printf("该患者已出院，不可修改。\n"); return; }
    printf("\n--- 修改住院信息 ---\n"); printInpatientOne(node);
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

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "住院编号:%s, 病人:%s, 修改字段:%d", node->id, node->name, choice);
    writeLog(LOG_INPATIENT, "修改住院信息", "系统", detail);
}

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
    printf("押金: %.2f 每日费用: %.2f 总费用: %.2f\n", node->deposit, node->dailyCost, node->totalCost);
    printf("状态: %s\n", getInpatientStatusString(node->status));
    if (node->status == INPATIENT_DISCHARGED) printf("出院日期: %s\n", node->dischargeDate);
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

void printInpatientAll(InpatientList* list) {
    if (list->head == NULL) { printf("暂无住院记录。\n"); return; }
    printf("\n--- 所有住院记录 (共 %d 条) ---\n", list->count);
    InpatientNode* current = list->head;
    while (current != NULL) { printInpatientOne(current); current = current->next; }
}

void dischargePatient(InpatientList* list, const char* id) {
    InpatientNode* node = findInpatientByID(list, id);
    if (node == NULL) { printf("未找到该患者。\n"); return; }
    if (node->status == INPATIENT_DISCHARGED) { printf("该患者已出院。\n"); return; }

    int days = daysBetween(node->admitDate, "");
    if (days <= 0) days = 1;
    node->totalCost = node->dailyCost * days;

    getCurrentDate(node->dischargeDate);
    node->status = INPATIENT_DISCHARGED;

    printf("\n--- 出院结算 ---\n");
    printInpatientOne(node);
    printf("\n住院天数: %d 天\n", days);
    printf("应缴费用: %.2f\n", node->totalCost);
    printf("押金余额: %.2f\n", node->deposit - node->totalCost);
    printf("出院办理完成！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "住院编号:%s, 病人:%s, 科室:%s, 住院天数:%d, 总费用:%.2f",
        node->id, node->name, node->department, days, node->totalCost);
    writeLog(LOG_INPATIENT, "办理出院", "系统", detail);
}

void statInpatients(InpatientList* list) {
    int admitted = 0, transferred = 0, discharged = 0;
    double totalDeposit = 0, totalCost = 0;

    InpatientNode* curr = list->head;
    while (curr != NULL) {
        switch (curr->status) {
        case INPATIENT_ADMITTED: admitted++; totalDeposit += curr->deposit; break;
        case INPATIENT_TRANSFERRED: transferred++; break;
        case INPATIENT_DISCHARGED: discharged++; totalCost += curr->totalCost; break;
        }
        curr = curr->next;
    }

    printf("\n--- 住院统计 ---\n");
    printf("当前在院: %d 人\n", admitted);
    printf("转科: %d 人\n", transferred);
    printf("已出院: %d 人\n", discharged);
    printf("总住院人数: %d 人\n", list->count);
    printf("在院押金总额: %.2f\n", totalDeposit);
    printf("已出院费用总额: %.2f\n", totalCost);
}

void freeInpatientList(InpatientList* list) {
    InpatientNode* current = list->head;
    while (current != NULL) { InpatientNode* temp = current; current = current->next; free(temp); }
    list->head = NULL; list->count = 0;
}

void inpatientMenu(InpatientList* list) {
    int choice;
    do {
        printf("\n"); printTitle("病人住院管理系统");
        printf("1. 办理入院\n2. 查询住院记录\n3. 删除记录\n4. 修改信息\n");
        printf("5. 办理出院\n6. 显示全部\n7. 统计分析\n0. 返回主菜单\n");
        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: {
            InpatientNode* node = createInpatientNode();
            if (node && !isInpatientIDExist(list, node->id)) {
                insertInpatientNode(list, node);
                printf("入院办理成功！\n");
                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "住院编号:%s, 病人:%s, 科室:%s, 病房:%s-%s, 医生:%s, 押金:%.2f",
                    node->id, node->name, node->department, node->roomNo, node->bedNo, node->doctor, node->deposit);
                writeLog(LOG_INPATIENT, "办理入院", "系统", detail);
            }
            else if (node) { printf("错误：住院编号已存在！\n"); free(node); }
            break;
        }
        case 2: {
            if (list->head == NULL) { printf("暂无住院记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入住院编号: ");
            InpatientNode* found = findInpatientByID(list, id);
            if (found) printInpatientOne(found); else printf("未找到。\n");
            break;
        }
        case 3: {
            if (list->head == NULL) { printf("暂无住院记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要删除的住院编号: ");
            deleteInpatientByID(list, id);
            break;
        }
        case 4: {
            if (list->head == NULL) { printf("暂无住院记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要修改的住院编号: ");
            InpatientNode* node = findInpatientByID(list, id);
            if (node) modifyInpatientInfo(node); else printf("未找到。\n");
            break;
        }
        case 5: {
            if (list->head == NULL) { printf("暂无住院记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要办理出院的住院编号: ");
            dischargePatient(list, id);
            break;
        }
        case 6: printInpatientAll(list); break;
        case 7: statInpatients(list); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

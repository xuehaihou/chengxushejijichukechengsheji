#include "outpatient.h"
#include "log.h"

void initOutpatientList(OutpatientList* list) { list->head = NULL; list->count = 0; }

void loadOutpatientsFromFile(OutpatientList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：门诊数据文件不存在，将创建新文件。\n"); return; }
    OutpatientNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        OutpatientNode* node = (OutpatientNode*)malloc(sizeof(OutpatientNode));
        if (node == NULL) continue;
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%lf|%[^\n]",
            node->id, node->patientId, node->patientName, node->department,
            node->doctor, node->date, node->diagnosis, node->prescription,
            &node->cost, node->remark);
        node->next = NULL;
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条门诊记录。\n", list->count);
}

void saveOutpatientsToFile(OutpatientList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }
    OutpatientNode* current = list->head;
    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|%s|%.2f|%s\n",
            current->id, current->patientId, current->patientName,
            current->department, current->doctor, current->date,
            current->diagnosis, current->prescription,
            current->cost, current->remark);
        current = current->next;
    }
    fclose(fp);
    printf("成功保存 %d 条门诊记录。\n", list->count);
}

OutpatientNode* createOutpatientNode() {
    OutpatientNode* node = (OutpatientNode*)malloc(sizeof(OutpatientNode));
    if (node == NULL) { printf("错误：内存分配失败！\n"); return NULL; }
    printf("\n--- 新增门诊记录 ---\n");
    safeInput(node->id, MAX_ID_LEN, "请输入门诊编号: ");
    while (isEmpty(node->id)) { printf("编号不能为空！\n"); safeInput(node->id, MAX_ID_LEN, "请输入门诊编号: "); }
    safeInput(node->patientId, MAX_ID_LEN, "请输入病人编号: ");
    safeInput(node->patientName, MAX_NAME_LEN, "请输入姓名: ");
    safeInput(node->department, MAX_DEPT_LEN, "请输入科室: ");
    safeInput(node->doctor, MAX_NAME_LEN, "请输入医生: ");
    getCurrentDate(node->date);
    safeInput(node->diagnosis, MAX_REMARK_LEN, "请输入诊断: ");
    safeInput(node->prescription, 500, "请输入处方: ");
    node->cost = inputDouble("请输入费用: ");
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注: ");
    node->next = NULL;
    return node;
}

int isOutpatientIDExist(OutpatientList* list, const char* id) {
    OutpatientNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return 1; current = current->next; }
    return 0;
}

void insertOutpatientNode(OutpatientList* list, OutpatientNode* node) {
    if (node == NULL) return;
    node->next = list->head; list->head = node; list->count++;
}

OutpatientNode* findOutpatientByID(OutpatientList* list, const char* id) {
    OutpatientNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return current; current = current->next; }
    return NULL;
}

int deleteOutpatientByID(OutpatientList* list, const char* id) {
    OutpatientNode* current = list->head, * prev = NULL;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printOutpatientOne(current);
            if (!confirm("确认删除?")) return 0;

            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "门诊编号:%s, 病人:%s, 科室:%s",
                current->id, current->patientName, current->department);

            if (prev == NULL) list->head = current->next;
            else prev->next = current->next;
            free(current); list->count--;

            writeLog(LOG_OUTPATIENT, "删除门诊记录", "系统", detail);
            return 1;
        }
        prev = current; current = current->next;
    }
    printf("未找到。\n"); return 0;
}

void modifyOutpatientInfo(OutpatientNode* node) {
    if (node == NULL) return;
    printOutpatientOne(node);
    printf("1.科室 2.医生 3.诊断 4.处方 5.费用 6.备注 0.取消\n");
    int choice = inputInt("选择: ");
    switch (choice) {
    case 1: safeInput(node->department, MAX_DEPT_LEN, "新科室: "); break;
    case 2: safeInput(node->doctor, MAX_NAME_LEN, "新医生: "); break;
    case 3: safeInput(node->diagnosis, MAX_REMARK_LEN, "新诊断: "); break;
    case 4: safeInput(node->prescription, 500, "新处方: "); break;
    case 5: node->cost = inputDouble("新费用: "); break;
    case 6: safeInput(node->remark, MAX_REMARK_LEN, "新备注: "); break;
    case 0: return;
    }
    printf("修改成功！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "门诊编号:%s, 病人:%s, 修改字段:%d", node->id, node->patientName, choice);
    writeLog(LOG_OUTPATIENT, "修改门诊记录", "系统", detail);
}

void printOutpatientOne(OutpatientNode* node) {
    if (node == NULL) return;
    printLine('-', 70);
    printf("门诊编号: %s\n", node->id);
    printf("病人编号: %s 姓名: %s\n", node->patientId, node->patientName);
    printf("科室: %s 医生: %s\n", node->department, node->doctor);
    printf("日期: %s\n", node->date);
    printf("诊断: %s\n", node->diagnosis);
    printf("处方: %s\n", node->prescription);
    printf("费用: %.2f\n", node->cost);
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

void printOutpatientAll(OutpatientList* list) {
    if (list->head == NULL) { printf("暂无门诊记录。\n"); return; }
    printf("\n--- 所有门诊记录 (共 %d 条) ---\n", list->count);
    OutpatientNode* current = list->head;
    while (current != NULL) { printOutpatientOne(current); current = current->next; }
}

void statOutpatients(OutpatientList* list) {
    double totalCost = 0;
    OutpatientNode* curr = list->head;
    typedef struct { char dept[50]; int cnt; double cost; } DeptStat;
    DeptStat stats[50]; int scnt = 0;

    while (curr != NULL) {
        totalCost += curr->cost;
        int found = 0;
        for (int i = 0; i < scnt; i++) {
            if (strcmp(stats[i].dept, curr->department) == 0) {
                stats[i].cnt++; stats[i].cost += curr->cost; found = 1; break;
            }
        }
        if (!found && scnt < 50) {
            strcpy(stats[scnt].dept, curr->department);
            stats[scnt].cnt = 1; stats[scnt].cost = curr->cost; scnt++;
        }
        curr = curr->next;
    }

    printf("\n--- 门诊统计 ---\n");
    printf("总门诊量: %d 次\n", list->count);
    printf("总费用: %.2f\n", totalCost);
    for (int i = 0; i < scnt; i++) {
        printf("%s: %d次, 费用%.2f\n", stats[i].dept, stats[i].cnt, stats[i].cost);
    }
}

void freeOutpatientList(OutpatientList* list) {
    OutpatientNode* current = list->head;
    while (current != NULL) { OutpatientNode* temp = current; current = current->next; free(temp); }
    list->head = NULL; list->count = 0;
}

void outpatientMenu(OutpatientList* list) {
    int choice;
    do {
        printf("\n"); printTitle("门诊管理系统");
        printf("1. 新增门诊\n2. 查询门诊\n3. 删除门诊\n4. 修改门诊\n");
        printf("5. 显示全部\n6. 统计分析\n0. 返回主菜单\n");
        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: {
            OutpatientNode* node = createOutpatientNode();
            if (node && !isOutpatientIDExist(list, node->id)) {
                insertOutpatientNode(list, node);
                printf("添加成功！\n");
                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "门诊编号:%s, 病人:%s, 科室:%s, 医生:%s, 费用:%.2f",
                    node->id, node->patientName, node->department, node->doctor, node->cost);
                writeLog(LOG_OUTPATIENT, "新增门诊记录", "系统", detail);
            }
            else if (node) { printf("编号已存在！\n"); free(node); }
            break;
        }
        case 2: {
            if (!list->head) { printf("无记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            OutpatientNode* found = findOutpatientByID(list, id);
            if (found) printOutpatientOne(found); else printf("未找到。\n");
            break;
        }
        case 3: {
            if (!list->head) { printf("无记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            deleteOutpatientByID(list, id);
            break;
        }
        case 4: {
            if (!list->head) { printf("无记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            OutpatientNode* node = findOutpatientByID(list, id);
            if (node) modifyOutpatientInfo(node); else printf("未找到。\n");
            break;
        }
        case 5: printOutpatientAll(list); break;
        case 6: statOutpatients(list); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

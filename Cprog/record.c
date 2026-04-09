#include "record.h"

void initRecordList(RecordList* list) { list->head = NULL; list->count = 0; }

void loadRecordsFromFile(RecordList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：病案数据文件不存在，将创建新文件。\n"); return; }
    RecordNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        RecordNode* node = (RecordNode*)malloc(sizeof(RecordNode));
        if (!node) continue;
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^\n]",
            node->id, node->patientId, node->name, node->gender,
            &node->age, node->department, node->doctor,
            node->admitDiagnosis, node->dischargeDiagnosis,
            node->treatmentSummary, node->dischargeDate,
            &node->archiveStatus, node->remark);
        node->next = NULL;
        if (!list->head) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条病案记录。\n", list->count);
}

void saveRecordsToFile(RecordList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }
    RecordNode* c = list->head;
    while (c) {
        fprintf(fp, "%s|%s|%s|%s|%d|%s|%s|%s|%s|%s|%s|%d|%s\n",
            c->id, c->patientId, c->name, c->gender,
            c->age, c->department, c->doctor,
            c->admitDiagnosis, c->dischargeDiagnosis,
            c->treatmentSummary, c->dischargeDate,
            c->archiveStatus, c->remark);
        c = c->next;
    }
    fclose(fp);
    printf("成功保存 %d 条病案记录。\n", list->count);
}

RecordNode* createRecordNode() {
    RecordNode* n = (RecordNode*)malloc(sizeof(RecordNode));
    if (!n) return NULL;
    printf("\n--- 新增病案 ---\n");
    safeInput(n->id, MAX_ID_LEN, "病案编号: ");
    while (isEmpty(n->id)) safeInput(n->id, MAX_ID_LEN, "编号不能为空！重新输入: ");
    safeInput(n->patientId, MAX_ID_LEN, "病人编号: ");
    safeInput(n->name, MAX_NAME_LEN, "姓名: ");
    safeInput(n->gender, 10, "性别: ");
    n->age = inputInt("年龄: ");
    safeInput(n->department, MAX_DEPT_LEN, "科室: ");
    safeInput(n->doctor, MAX_NAME_LEN, "医生: ");
    safeInput(n->admitDiagnosis, MAX_REMARK_LEN, "入院诊断: ");
    safeInput(n->dischargeDiagnosis, MAX_REMARK_LEN, "出院诊断: ");
    safeInput(n->treatmentSummary, 500, "治疗摘要: ");
    safeInput(n->dischargeDate, MAX_DATE_LEN, "出院日期: ");
    while (!isValidDate(n->dischargeDate)) { printf("日期格式不正确！\n"); safeInput(n->dischargeDate, MAX_DATE_LEN, "重新输入出院日期: "); }
    n->archiveStatus = 0;
    safeInput(n->remark, MAX_REMARK_LEN, "备注: ");
    n->next = NULL;
    return n;
}

int isRecordIDExist(RecordList* list, const char* id) {
    RecordNode* c = list->head;
    while (c) { if (strcmp(c->id, id) == 0) return 1; c = c->next; }
    return 0;
}
void insertRecordNode(RecordList* list, RecordNode* node) { if (node) { node->next = list->head; list->head = node; list->count++; } }

RecordNode* findRecordByID(RecordList* list, const char* id) {
    RecordNode* c = list->head;
    while (c) { if (strcmp(c->id, id) == 0) return c; c = c->next; }
    return NULL;
}

int deleteRecordByID(RecordList* list, const char* id) {
    RecordNode* c = list->head, * p = NULL;
    while (c) {
        if (strcmp(c->id, id) == 0) {
            printRecordOne(c);
            if (!confirm("确认删除?")) return 0;
            if (!p) list->head = c->next; else p->next = c->next;
            free(c); list->count--; return 1;
        }
        p = c; c = c->next;
    }
    printf("未找到。\n"); return 0;
}

void modifyRecordInfo(RecordNode* node) {
    if (!node) return;
    printRecordOne(node);
    printf("1.入院诊断 2.出院诊断 3.治疗摘要 4.归档状态 5.备注 0.取消\n");
    int ch = inputInt("选择: ");
    switch (ch) {
    case 1: safeInput(node->admitDiagnosis, MAX_REMARK_LEN, "新入院诊断: "); break;
    case 2: safeInput(node->dischargeDiagnosis, MAX_REMARK_LEN, "新出院诊断: "); break;
    case 3: safeInput(node->treatmentSummary, 500, "新治疗摘要: "); break;
    case 4:
        printf("归档状态(0.未归档 1.已归档): ");
        node->archiveStatus = inputInt(""); break;
    case 5: safeInput(node->remark, MAX_REMARK_LEN, "新备注: "); break;
    case 0: return;
    }
    printf("修改成功！\n");
}

void printRecordOne(RecordNode* node) {
    if (!node) return;
    printLine('-', 70);
    printf("病案编号: %s\n", node->id);
    printf("病人: %s (%s, %d岁)\n", node->name, node->gender, node->age);
    printf("科室: %s 医生: %s\n", node->department, node->doctor);
    printf("入院诊断: %s\n", node->admitDiagnosis);
    printf("出院诊断: %s\n", node->dischargeDiagnosis);
    printf("治疗摘要: %s\n", node->treatmentSummary);
    printf("出院日期: %s 归档状态: %s\n", node->dischargeDate, node->archiveStatus ? "已归档" : "未归档");
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

void printRecordAll(RecordList* list) {
    if (!list->head) { printf("暂无病案记录。\n"); return; }
    printf("\n--- 所有病案 (%d条) ---\n", list->count);
    RecordNode* c = list->head;
    while (c) { printRecordOne(c); c = c->next; }
}

void freeRecordList(RecordList* list) {
    RecordNode* c = list->head;
    while (c) { RecordNode* t = c; c = c->next; free(t); }
    list->head = NULL; list->count = 0;
}

void recordMenu(RecordList* list) {
    int choice;
    do {
        printf("\n"); printTitle("病案管理系统");
        printf("1.新增 2.查询 3.删除 4.修改 5.全部显示 0.返回\n");
        choice = inputInt("选择: ");
        switch (choice) {
        case 1: {
            RecordNode* n = createRecordNode();
            if (n && !isRecordIDExist(list, n->id)) { insertRecordNode(list, n); printf("添加成功！\n"); }
            else if (n) { printf("编号已存在！\n"); free(n); } break;
        }
        case 2: {
            if (!list->head) break;
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            RecordNode* f = findRecordByID(list, id);
            if (f) printRecordOne(f); else printf("未找到。\n"); break;
        }
        case 3: {
            if (!list->head) break;
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            deleteRecordByID(list, id); break;
        }
        case 4: {
            if (!list->head) break;
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            RecordNode* n = findRecordByID(list, id);
            if (n) modifyRecordInfo(n); else printf("未找到。\n"); break;
        }
        case 5: printRecordAll(list); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

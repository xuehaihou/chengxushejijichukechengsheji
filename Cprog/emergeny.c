#include "emergency.h"
#include "log.h"

const char* getEmergencyLevelString(int level) {
    switch (level) { case 1: return "危重"; case 2: return "重症"; case 3: return "普通"; case 4: return "轻症"; default: return "未知"; }
}
const char* getEmergencyStatusString(int status) {
    switch (status) { case 0: return "接诊中"; case 1: return "已处理"; case 2: return "转住院"; case 3: return "离院"; default: return "未知"; }
}

void initEmergencyList(EmergencyList* list) { list->head = NULL; list->count = 0; }

void loadEmergenciesFromFile(EmergencyList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：急诊数据文件不存在，将创建新文件。\n"); return; }
    EmergencyNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        EmergencyNode* node = (EmergencyNode*)malloc(sizeof(EmergencyNode));
        if (node == NULL) continue;
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%d|%[^|]|%[^|]|%[^|]|%d|%[^\n]",
            node->id, node->patientId, node->name, node->gender,
            &node->age, node->arriveTime, &node->level, node->symptoms,
            node->doctor, node->result, &node->status, node->remark);
        node->next = NULL;
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条急诊记录。\n", list->count);
}

void saveEmergenciesToFile(EmergencyList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }
    EmergencyNode* current = list->head;
    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%d|%s|%d|%s|%s|%s|%d|%s\n",
            current->id, current->patientId, current->name, current->gender,
            current->age, current->arriveTime, current->level, current->symptoms,
            current->doctor, current->result, current->status, current->remark);
        current = current->next;
    }
    fclose(fp);
    printf("成功保存 %d 条急诊记录。\n", list->count);
}

EmergencyNode* createEmergencyNode() {
    EmergencyNode* node = (EmergencyNode*)malloc(sizeof(EmergencyNode));
    if (node == NULL) return NULL;
    printf("\n--- 新增急诊记录 ---\n");
    safeInput(node->id, MAX_ID_LEN, "请输入急诊编号: ");
    while (isEmpty(node->id)) safeInput(node->id, MAX_ID_LEN, "编号不能为空！重新输入: ");
    safeInput(node->patientId, MAX_ID_LEN, "病人编号: ");
    safeInput(node->name, MAX_NAME_LEN, "姓名: ");
    safeInput(node->gender, 10, "性别: ");
    node->age = inputInt("年龄: ");
    getCurrentDate(node->arriveTime);
    printf("分级(1.危重 2.重症 3.普通 4.轻症): ");
    node->level = inputInt("");
    while (node->level < 1 || node->level > 4) { printf("无效！重新输入: "); node->level = inputInt(""); }
    safeInput(node->symptoms, MAX_REMARK_LEN, "症状描述: ");
    safeInput(node->doctor, MAX_NAME_LEN, "接诊医生: ");
    safeInput(node->result, MAX_REMARK_LEN, "处理结果: ");
    node->status = 0;
    safeInput(node->remark, MAX_REMARK_LEN, "备注: ");
    node->next = NULL;
    return node;
}

int isEmergencyIDExist(EmergencyList* list, const char* id) {
    EmergencyNode* c = list->head;
    while (c) { if (strcmp(c->id, id) == 0) return 1; c = c->next; }
    return 0;
}

void insertEmergencyNode(EmergencyList* list, EmergencyNode* node) {
    if (!node) return;
    node->next = list->head; list->head = node; list->count++;
}

EmergencyNode* findEmergencyByID(EmergencyList* list, const char* id) {
    EmergencyNode* c = list->head;
    while (c) { if (strcmp(c->id, id) == 0) return c; c = c->next; }
    return NULL;
}

int deleteEmergencyByID(EmergencyList* list, const char* id) {
    EmergencyNode* c = list->head, * p = NULL;
    while (c) {
        if (strcmp(c->id, id) == 0) {
            printEmergencyOne(c);
            if (!confirm("确认删除?")) return 0;

            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "急诊编号:%s, 病人:%s, 级别:%s",
                c->id, c->name, getEmergencyLevelString(c->level));

            if (!p) list->head = c->next; else p->next = c->next;
            free(c); list->count--;

            writeLog(LOG_EMERGENCY, "删除急诊记录", "系统", detail);
            return 1;
        }
        p = c; c = c->next;
    }
    printf("未找到。\n"); return 0;
}

void modifyEmergencyInfo(EmergencyNode* node) {
    if (!node) return;
    printEmergencyOne(node);
    printf("1.医生 2.结果 3.状态 4.备注 0.取消\n");
    int ch = inputInt("选择: ");
    switch (ch) {
    case 1: safeInput(node->doctor, MAX_NAME_LEN, "新医生: "); break;
    case 2: safeInput(node->result, MAX_REMARK_LEN, "新结果: "); break;
    case 3:
        printf("状态(0.接诊中 1.已处理 2.转住院 3.离院): ");
        node->status = inputInt(""); break;
    case 4: safeInput(node->remark, MAX_REMARK_LEN, "新备注: "); break;
    case 0: return;
    }
    printf("修改成功！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "急诊编号:%s, 病人:%s, 修改字段:%d", node->id, node->name, ch);
    writeLog(LOG_EMERGENCY, "修改急诊记录", "系统", detail);
}

void printEmergencyOne(EmergencyNode* node) {
    if (!node) return;
    printLine('-', 70);
    printf("急诊编号: %s\n", node->id);
    printf("病人: %s (%s, %d岁)\n", node->name, node->gender, node->age);
    printf("到达时间: %s 分级: %s级(%s)\n", node->arriveTime, getEmergencyLevelString(node->level), node->level == 1 ? "危重" : node->level == 2 ? "重症" : "一般");
    printf("症状: %s\n", node->symptoms);
    printf("医生: %s 结果: %s\n", node->doctor, node->result);
    printf("状态: %s 备注: %s\n", getEmergencyStatusString(node->status), node->remark);
    printLine('-', 70);
}

void printEmergencyAll(EmergencyList* list) {
    if (!list->head) { printf("暂无急诊记录。\n"); return; }
    printf("\n--- 所有急诊记录 (%d条) ---\n", list->count);
    EmergencyNode* c = list->head;
    while (c) { printEmergencyOne(c); c = c->next; }
}

void statEmergencies(EmergencyList* list) {
    int lv[5] = { 0 }, st[4] = { 0 };
    EmergencyNode* c = list->head;
    while (c) {
        if (c->level >= 1 && c->level <= 4) lv[c->level]++;
        if (c->status >= 0 && c->status <= 3) st[c->status]++;
        c = c->next;
    }
    printf("\n--- 急诊统计 ---\n");
    printf("按分级: 危重%d 重症%d 普通%d 轻症%d\n", lv[1], lv[2], lv[3], lv[4]);
    printf("按状态: 接诊%d 已处理%d 转住院%d 离院%d\n", st[0], st[1], st[2], st[3]);
    printf("总计: %d\n", list->count);
}

void freeEmergencyList(EmergencyList* list) {
    EmergencyNode* c = list->head;
    while (c) { EmergencyNode* t = c; c = c->next; free(t); }
    list->head = NULL; list->count = 0;
}

void emergencyMenu(EmergencyList* list) {
    int choice;
    do {
        printf("\n"); printTitle("急诊管理系统");
        printf("1.新增 2.查询 3.删除 4.修改 5.全部显示 6.统计 0.返回\n");
        choice = inputInt("选择: ");
        switch (choice) {
        case 1: {
            EmergencyNode* n = createEmergencyNode();
            if (n && !isEmergencyIDExist(list, n->id)) {
                insertEmergencyNode(list, n);
                printf("添加成功！\n");
                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "急诊编号:%s, 病人:%s, 级别:%s, 医生:%s",
                    n->id, n->name, getEmergencyLevelString(n->level), n->doctor);
                writeLog(LOG_EMERGENCY, "新增急诊记录", "系统", detail);
            }
            else if (n) { printf("编号已存在！\n"); free(n); } break;
        }
        case 2: {
            if (!list->head) break;
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            EmergencyNode* f = findEmergencyByID(list, id);
            if (f) printEmergencyOne(f); else printf("未找到。\n"); break;
        }
        case 3: {
            if (!list->head) break;
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            deleteEmergencyByID(list, id); break;
        }
        case 4: {
            if (!list->head) break;
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            EmergencyNode* n = findEmergencyByID(list, id);
            if (n) modifyEmergencyInfo(n); else printf("未找到。\n"); break;
        }
        case 5: printEmergencyAll(list); break;
        case 6: statEmergencies(list); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

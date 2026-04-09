#include "blood.h"
#include "log.h"

void initBloodList(BloodList* list) { list->head = NULL; list->count = 0; }

void loadBloodsFromFile(BloodList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：血库数据文件不存在，将创建新文件。\n"); return; }
    BloodNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        BloodNode* node = (BloodNode*)malloc(sizeof(BloodNode));
        if (!node) continue;
        sscanf_s(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^\n]",
            node->id, node->bloodType, node->rhType,
            &node->quantity, node->unit, node->collectDate,
            node->expiryDate, node->source, &node->status, node->remark);
        node->next = NULL;
        if (!list->head) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条血库记录。\n", list->count);
}

void saveBloodsToFile(BloodList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }
    BloodNode* c = list->head;
    while (c) {
        fprintf(fp, "%s|%s|%s|%d|%s|%s|%s|%s|%d|%s\n",
            c->id, c->bloodType, c->rhType,
            c->quantity, c->unit, c->collectDate,
            c->expiryDate, c->source, c->status, c->remark);
        c = c->next;
    }
    fclose(fp);
    printf("成功保存 %d 条血库记录。\n", list->count);
}

BloodNode* createBloodNode() {
    BloodNode* n = (BloodNode*)malloc(sizeof(BloodNode));
    if (!n) return NULL;
    printf("\n--- 新增血液库存 ---\n");
    safeInput(n->id, MAX_ID_LEN, "血液编号: ");
    while (isEmpty(n->id)) safeInput(n->id, MAX_ID_LEN, "编号不能为空！重新输入: ");
    printf("血型(A/B/O/AB): ");
    safeInput(n->bloodType, 5, "");
    toUpperCase(n->bloodType);
    printf("Rh型(+/-): ");
    safeInput(n->rhType, 3, "");
    n->quantity = inputInt("数量(ml): ");
    strcpy(n->unit, "ml");
    getCurrentDate(n->collectDate);
    safeInput(n->expiryDate, MAX_DATE_LEN, "有效期至: ");
    while (!isValidDate(n->expiryDate)) { printf("日期格式不正确！\n"); safeInput(n->expiryDate, MAX_DATE_LEN, "重新输入: "); }
    safeInput(n->source, MAX_NAME_LEN, "来源(献血者/血站): ");
    n->status = 1;
    safeInput(n->remark, MAX_REMARK_LEN, "备注: ");
    n->next = NULL;
    return n;
}

int isBloodIDExist(BloodList* list, const char* id) {
    BloodNode* c = list->head;
    while (c) { if (strcmp(c->id, id) == 0) return 1; c = c->next; }
    return 0;
}
void insertBloodNode(BloodList* list, BloodNode* node) { if (node) { node->next = list->head; list->head = node; list->count++; } }

BloodNode* findBloodByID(BloodList* list, const char* id) {
    BloodNode* c = list->head;
    while (c) { if (strcmp(c->id, id) == 0) return c; c = c->next; }
    return NULL;
}

int deleteBloodByID(BloodList* list, const char* id) {
    BloodNode* c = list->head, * p = NULL;
    while (c) {
        if (strcmp(c->id, id) == 0) {
            printBloodOne(c);
            if (!confirm("确认删除?")) return 0;

            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "血液编号:%s, 血型:%s%s, 数量:%d%s",
                c->id, c->bloodType, c->rhType, c->quantity, c->unit);

            if (!p) list->head = c->next; else p->next = c->next;
            free(c); list->count--;

            writeLog(LOG_BLOOD, "删除血液记录", "系统", detail);
            return 1;
        }
        p = c; c = c->next;
    }
    printf("未找到。\n"); return 0;
}

void modifyBloodInfo(BloodNode* node) {
    if (!node) return;
    printBloodOne(node);
    printf("1.数量 2.有效期 3.状态 4.备注 0.取消\n");
    int ch = inputInt("选择: ");
    switch (ch) {
    case 1: node->quantity = inputInt("新数量: "); break;
    case 2:
        safeInput(node->expiryDate, MAX_DATE_LEN, "新有效期: ");
        while (!isValidDate(node->expiryDate)) { printf("格式不正确！\n"); safeInput(node->expiryDate, MAX_DATE_LEN, "重新输入: "); }
        break;
    case 3:
        printf("状态(0.不可用 1.可用): ");
        node->status = inputInt(""); break;
    case 4: safeInput(node->remark, MAX_REMARK_LEN, "新备注: "); break;
    case 0: return;
    }
    printf("修改成功！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "血液编号:%s, 血型:%s%s, 修改字段:%d", node->id, node->bloodType, node->rhType, ch);
    writeLog(LOG_BLOOD, "修改血液记录", "系统", detail);
}

void printBloodOne(BloodNode* node) {
    if (!node) return;
    printLine('-', 70);
    printf("血液编号: %s\n", node->id);
    printf("血型: %s%s | 数量: %d %s\n", node->bloodType, node->rhType, node->quantity, node->unit);
    printf("采集日期: %s 有效期至: %s\n", node->collectDate, node->expiryDate);
    printf("来源: %s 状态: %s\n", node->source, node->status ? "可用" : "不可用");
    char today[MAX_DATE_LEN]; getCurrentDate(today);
    if (compareDate(node->expiryDate, today) <= 0) printf("【已过期！】\n");
    else if (daysBetween(today, node->expiryDate) < 7) printf("【即将过期(7天内)】\n");
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

void printBloodAll(BloodList* list) {
    if (!list->head) { printf("暂无血库记录。\n"); return; }
    printf("\n--- 所有血液库存 (%d条) ---\n", list->count);
    BloodNode* c = list->head;
    while (c) { printBloodOne(c); c = c->next; }
}

int bloodInStock(BloodList* list, const char* id, int quantity) {
    BloodNode* n = findBloodByID(list, id);
    if (!n) { printf("未找到该血液记录。\n"); return 0; }
    n->quantity += quantity;
    getCurrentDate(n->collectDate);
    printf("入库成功！当前库存: %d %s\n", n->quantity, n->unit);

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "血液编号:%s, 血型:%s%s, 入库:%d, 当前库存:%d%s",
        n->id, n->bloodType, n->rhType, quantity, n->quantity, n->unit);
    writeLog(LOG_BLOOD, "血液入库", "系统", detail);
    return 1;
}

int bloodOutStock(BloodList* list, const char* id, int quantity) {
    BloodNode* n = findBloodByID(list, id);
    if (!n) { printf("未找到该血液记录。\n"); return 0; }
    if (n->quantity < quantity) { printf("库存不足！当前: %d %s\n", n->quantity, n->unit); return 0; }
    n->quantity -= quantity;
    printf("出库成功！剩余: %d %s\n", n->quantity, n->unit);

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "血液编号:%s, 血型:%s%s, 出库:%d, 剩余库存:%d%s",
        n->id, n->bloodType, n->rhType, quantity, n->quantity, n->unit);
    writeLog(LOG_BLOOD, "血液出库", "系统", detail);
    return 1;
}

void statByBloodType(BloodList* list) {
    typedef struct { char bt[10]; int qty; } BTStat;
    BTStat stats[20]; int sc = 0;
    BloodNode* c = list->head;

    while (c && c->status == 1) {
        char bt[10]; sprintf(bt, "%s%s", c->bloodType, c->rhType);
        int f = 0;
        for (int i = 0; i < sc; i++) {
            if (strcmp(stats[i].bt, bt) == 0) { stats[i].qty += c->quantity; f = 1; break; }
        }
        if (!f && sc < 20) { strcpy(stats[sc].bt, bt); stats[sc].qty = c->quantity; sc++; }
        c = c->next;
    }

    printf("\n--- 血型库存统计 ---\n");
    for (int i = 0; i < sc; i++) {
        printf("%s: %d ml\n", stats[i].bt, stats[i].qty);
    }
}

void freeBloodList(BloodList* list) {
    BloodNode* c = list->head;
    while (c) { BloodNode* t = c; c = c->next; free(t); }
    list->head = NULL; list->count = 0;
}

void bloodMenu(BloodList* list) {
    int choice;
    do {
        printf("\n"); printTitle("血库管理系统");
        printf("1. 新增血液\n2. 查询\n3. 删除\n4. 修改\n");
        printf("5. 入库\n6. 出库\n7. 全部显示\n8. 血型统计\n0. 返回\n");
        choice = inputInt("选择: ");

        switch (choice) {
        case 1: {
            BloodNode* n = createBloodNode();
            if (n && !isBloodIDExist(list, n->id)) {
                insertBloodNode(list, n);
                printf("添加成功！\n");
                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "血液编号:%s, 血型:%s%s, 数量:%d%s",
                    n->id, n->bloodType, n->rhType, n->quantity, n->unit);
                writeLog(LOG_BLOOD, "新增血液记录", "系统", detail);
            }
            else if (n) { printf("编号已存在！\n"); free(n); } break;
        }
        case 2: {
            if (!list->head) break;
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            BloodNode* f = findBloodByID(list, id);
            if (f) printBloodOne(f); else printf("未找到。\n"); break;
        }
        case 3: {
            if (!list->head) break;
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            deleteBloodByID(list, id); break;
        }
        case 4: {
            if (!list->head) break;
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            BloodNode* n = findBloodByID(list, id);
            if (n) modifyBloodInfo(n); else printf("未找到。\n"); break;
        }
        case 5: {
            if (!list->head) break;
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            int qty = inputInt("入库数量: ");
            bloodInStock(list, id, qty); break;
        }
        case 6: {
            if (!list->head) break;
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "输入编号: ");
            int qty = inputInt("出库数量: ");
            bloodOutStock(list, id, qty); break;
        }
        case 7: printBloodAll(list); break;
        case 8: statByBloodType(list); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

#include "medicine.h"
#include "log.h"

void initMedicineList(MedicineList* list) { list->head = NULL; list->count = 0; }

void loadMedicinesFromFile(MedicineList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：药品数据文件不存在，将创建新文件。\n"); return; }
    MedicineNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        MedicineNode* node = (MedicineNode*)malloc(sizeof(MedicineNode));
        if (node == NULL) continue;
        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%lf|%[^|]|%[^|]|%[^\n]",
            node->id, node->name, node->category, &node->quantity,
            node->unit, &node->price, node->manufacturer,
            node->expiryDate, node->remark);
        node->next = NULL;
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条药品记录。\n", list->count);
}

void saveMedicinesToFile(MedicineList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }
    MedicineNode* current = list->head;
    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%d|%s|%.2f|%s|%s|%s\n",
            current->id, current->name, current->category, current->quantity,
            current->unit, current->price, current->manufacturer,
            current->expiryDate, current->remark);
        current = current->next;
    }
    fclose(fp);
    printf("成功保存 %d 条药品记录。\n", list->count);
}

MedicineNode* createMedicineNode() {
    MedicineNode* node = (MedicineNode*)malloc(sizeof(MedicineNode));
    if (node == NULL) { printf("错误：内存分配失败！\n"); return NULL; }
    printf("\n--- 新增药品 ---\n");
    safeInput(node->id, MAX_ID_LEN, "请输入药品编号: ");
    while (isEmpty(node->id)) { printf("编号不能为空！\n"); safeInput(node->id, MAX_ID_LEN, "请输入药品编号: "); }
    safeInput(node->name, MAX_NAME_LEN, "请输入药品名称: ");
    safeInput(node->category, MAX_DEPT_LEN, "请输入类别: ");
    node->quantity = inputInt("请输入库存数量: ");
    safeInput(node->unit, 20, "请输入计量单位: ");
    node->price = inputDouble("请输入单价: ");
    safeInput(node->manufacturer, MAX_NAME_LEN, "请输入生产厂家: ");
    safeInput(node->expiryDate, MAX_DATE_LEN, "请输入有效期 (YYYY-MM-DD): ");
    while (!isValidDate(node->expiryDate)) {
        printf("日期格式不正确！\n");
        safeInput(node->expiryDate, MAX_DATE_LEN, "请输入有效期 (YYYY-MM-DD): ");
    }
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注 (无则留空): ");
    node->next = NULL;
    return node;
}

int isMedicineIDExist(MedicineList* list, const char* id) {
    MedicineNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return 1; current = current->next; }
    return 0;
}

void insertMedicineNode(MedicineList* list, MedicineNode* node) {
    if (node == NULL) return;
    node->next = list->head; list->head = node; list->count++;
}

MedicineNode* findMedicineByID(MedicineList* list, const char* id) {
    MedicineNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return current; current = current->next; }
    return NULL;
}

int deleteMedicineByID(MedicineList* list, const char* id) {
    MedicineNode* current = list->head, * prev = NULL;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printf("\n找到以下记录:\n"); printMedicineOne(current);
            if (!confirm("确认删除该药品?")) { printf("已取消删除。\n"); return 0; }

            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "药品编号:%s, 名称:%s, 类别:%s",
                current->id, current->name, current->category);

            if (prev == NULL) list->head = current->next;
            else prev->next = current->next;
            free(current); list->count--; printf("删除成功！\n");

            writeLog(LOG_MEDICINE, "删除药品", "系统", detail);
            return 1;
        }
        prev = current; current = current->next;
    }
    printf("未找到编号为 %s 的药品。\n", id); return 0;
}

void modifyMedicineInfo(MedicineNode* node) {
    if (node == NULL) return;
    printf("\n--- 修改药品信息 ---\n"); printMedicineOne(node);
    printf("\n请选择要修改的字段:\n");
    printf("1. 名称\n2. 类别\n3. 库存数量\n4. 计量单位\n5. 单价\n6. 生产厂家\n7. 有效期\n8. 备注\n0. 取消\n");
    int choice = inputInt("请选择: ");
    switch (choice) {
    case 1: safeInput(node->name, MAX_NAME_LEN, "请输入新名称: "); break;
    case 2: safeInput(node->category, MAX_DEPT_LEN, "请输入新类别: "); break;
    case 3: node->quantity = inputInt("请输入新库存数量: "); break;
    case 4: safeInput(node->unit, 20, "请输入新计量单位: "); break;
    case 5: node->price = inputDouble("请输入新单价: "); break;
    case 6: safeInput(node->manufacturer, MAX_NAME_LEN, "请输入新生产厂家: "); break;
    case 7:
        safeInput(node->expiryDate, MAX_DATE_LEN, "请输入新有效期: ");
        while (!isValidDate(node->expiryDate)) {
            printf("日期格式不正确！\n");
            safeInput(node->expiryDate, MAX_DATE_LEN, "请输入新有效期: ");
        }
        break;
    case 8: safeInput(node->remark, MAX_REMARK_LEN, "请输入新备注: "); break;
    case 0: printf("取消修改。\n"); return;
    default: printf("无效选择。\n"); return;
    }
    printf("修改成功！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "药品编号:%s, 名称:%s, 修改字段:%d", node->id, node->name, choice);
    writeLog(LOG_MEDICINE, "修改药品", "系统", detail);
}

void medicineInStock(MedicineList* list, const char* id, int quantity) {
    MedicineNode* node = findMedicineByID(list, id);
    if (node == NULL) { printf("未找到该药品。\n"); return; }
    node->quantity += quantity;
    printf("入库成功！当前库存: %d %s\n", node->quantity, node->unit);

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "药品编号:%s, 名称:%s, 入库数量:%d, 当前库存:%d%s",
        node->id, node->name, quantity, node->quantity, node->unit);
    writeLog(LOG_MEDICINE, "药品入库", "系统", detail);
}

int medicineOutStock(MedicineList* list, const char* id, int quantity) {
    MedicineNode* node = findMedicineByID(list, id);
    if (node == NULL) { printf("未找到该药品。\n"); return 0; }
    if (node->quantity < quantity) { printf("库存不足！当前库存: %d %s\n", node->quantity, node->unit); return 0; }
    node->quantity -= quantity;
    printf("出库成功！剩余库存: %d %s\n", node->quantity, node->unit);

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "药品编号:%s, 名称:%s, 出库数量:%d, 剩余库存:%d%s",
        node->id, node->name, quantity, node->quantity, node->unit);
    writeLog(LOG_MEDICINE, "药品出库", "系统", detail);
    return 1;
}

void printMedicineOne(MedicineNode* node) {
    if (node == NULL) return;
    printLine('-', 70);
    printf("药品编号: %s\n", node->id);
    printf("药品名称: %s\n", node->name);
    printf("类别: %s\n", node->category);
    printf("库存: %d %s\n", node->quantity, node->unit);
    printf("单价: %.2f\n", node->price);
    printf("生产厂家: %s\n", node->manufacturer);
    printf("有效期: %s\n", node->expiryDate);
    char today[MAX_DATE_LEN];
    getCurrentDate(today);
    if (compareDate(node->expiryDate, today) <= 0) {
        printf("【已过期！】\n");
    }
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

void printMedicineAll(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    printf("\n--- 所有药品 (共 %d 种) ---\n", list->count);
    MedicineNode* current = list->head;
    while (current != NULL) { printMedicineOne(current); current = current->next; }
}

void printMedicineWarning(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    int count = 0;
    MedicineNode* current = list->head;
    char today[MAX_DATE_LEN];
    getCurrentDate(today);
    printf("\n--- 药品预警 ---\n");
    while (current != NULL) {
        int needWarning = 0;
        if (current->quantity <= 10) needWarning = 1;
        if (compareDate(current->expiryDate, today) <= 0) needWarning = 1;
        if (needWarning) {
            printMedicineOne(current); count++;
        }
        current = current->next;
    }
    if (count == 0) printf("暂无需要关注的药品。\n");
    else printf("共 %d 种药品需要关注。\n", count);
}

void freeMedicineList(MedicineList* list) {
    MedicineNode* current = list->head;
    while (current != NULL) { MedicineNode* temp = current; current = current->next; free(temp); }
    list->head = NULL; list->count = 0;
}

void addMedicine(MedicineList* list) {
    MedicineNode* node = createMedicineNode();
    if (node == NULL) return;
    if (isMedicineIDExist(list, node->id)) {
        printf("错误：药品编号 %s 已存在！\n", node->id); free(node); return;
    }
    insertMedicineNode(list, node);
    printf("药品添加成功！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "药品编号:%s, 名称:%s, 类别:%s, 库存:%d%s, 单价:%.2f",
        node->id, node->name, node->category, node->quantity, node->unit, node->price);
    writeLog(LOG_MEDICINE, "添加药品", "系统", detail);
}

void queryMedicine(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    printf("\n--- 查询药品 ---\n");
    printf("1. 按编号查询\n2. 按名称查询\n3. 按类别查询\n4. 显示全部\n5. 查看药品预警\n0. 返回\n");
    int choice = inputInt("请选择: ");
    char keyword[MAX_NAME_LEN];
    switch (choice) {
    case 1:
        safeInput(keyword, MAX_ID_LEN, "请输入编号: ");
        MedicineNode* found = findMedicineByID(list, keyword);
        if (found) printMedicineOne(found);
        else printf("未找到。\n");
        break;
    case 2:
        safeInput(keyword, MAX_NAME_LEN, "请输入名称: ");
        MedicineNode* curr = list->head; int cnt = 0;
        while (curr != NULL) { if (strstr(curr->name, keyword)) { printMedicineOne(curr); cnt++; } curr = curr->next; }
        if (cnt == 0) printf("未找到。\n");
        break;
    case 3:
        safeInput(keyword, MAX_DEPT_LEN, "请输入类别: ");
        curr = list->head; cnt = 0;
        while (curr != NULL) { if (strstr(curr->category, keyword)) { printMedicineOne(curr); cnt++; } curr = curr->next; }
        if (cnt == 0) printf("未找到。\n");
        break;
    case 4: printMedicineAll(list); break;
    case 5: printMedicineWarning(list); break;
    case 0: return;
    default: printf("无效选择。\n");
    }
}

void deleteMedicine(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要删除的药品编号: ");
    deleteMedicineByID(list, id);
}

void modifyMedicine(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要修改的药品编号: ");
    MedicineNode* node = findMedicineByID(list, id);
    if (node == NULL) { printf("未找到。\n"); return; }
    modifyMedicineInfo(node);
}

void medicineIn(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入药品编号: ");
    int qty = inputInt("请输入入库数量: ");
    medicineInStock(list, id, qty);
}

void medicineOut(MedicineList* list) {
    if (list->head == NULL) { printf("暂无药品记录。\n"); return; }
    char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入药品编号: ");
    int qty = inputInt("请输入出库数量: ");
    medicineOutStock(list, id, qty);
}

void medicineMenu(MedicineList* list) {
    int choice;
    do {
        printf("\n"); printTitle("药库管理系统");
        printf("1. 新增药品\n2. 查询药品\n3. 删除药品\n4. 修改药品\n");
        printf("5. 药品入库\n6. 药品出库\n0. 返回主菜单\n");
        choice = inputInt("请选择功能: ");
        switch (choice) {
        case 1: addMedicine(list); break;
        case 2: queryMedicine(list); break;
        case 3: deleteMedicine(list); break;
        case 4: modifyMedicine(list); break;
        case 5: medicineIn(list); break;
        case 6: medicineOut(list); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

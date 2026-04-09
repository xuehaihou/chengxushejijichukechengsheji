#include "logistics.h"
#include "log.h"

void initMaterialList(MaterialList* list) { list->head = NULL; list->count = 0; }
void initUsageList(UsageList* list) { list->head = NULL; list->count = 0; }

void loadMaterialsFromFile(MaterialList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：物资数据文件不存在，将创建新文件。\n"); return; }
    MaterialNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        MaterialNode* node = (MaterialNode*)malloc(sizeof(MaterialNode));
        if (node == NULL) continue;
        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
            node->id, node->name, node->category, &node->quantity,
            node->unit, &node->minStock, node->location, node->supplier,
            node->lastInDate, node->lastOutDate, node->remark);
        node->next = NULL;
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条物资记录。\n", list->count);
}

void loadUsageFromFile(UsageList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：领用记录文件不存在，将创建新文件。\n"); return; }
    UsageNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        UsageNode* node = (UsageNode*)malloc(sizeof(UsageNode));
        if (node == NULL) continue;
        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%d|%[^\n]",
            node->id, node->materialId, node->dept, &node->quantity,
            node->date, node->handler, &node->auditStatus, node->remark);
        node->next = NULL;
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条领用记录。\n", list->count);
}

void saveMaterialsToFile(MaterialList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }
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

void saveUsageToFile(UsageList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }
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

MaterialNode* createMaterialNode() {
    MaterialNode* node = (MaterialNode*)malloc(sizeof(MaterialNode));
    if (node == NULL) { printf("错误：内存分配失败！\n"); return NULL; }
    printf("\n--- 新增物资 ---\n");
    safeInput(node->id, MAX_ID_LEN, "请输入物资编号: ");
    while (isEmpty(node->id)) { printf("编号不能为空！\n"); safeInput(node->id, MAX_ID_LEN, "请输入物资编号: "); }
    safeInput(node->name, MAX_NAME_LEN, "请输入物资名称: ");
    safeInput(node->category, MAX_DEPT_LEN, "请输入类别: ");
    node->quantity = inputInt("请输入库存数量: ");
    safeInput(node->unit, 20, "请输入计量单位: ");
    node->minStock = inputInt("请输入最低库存值: ");
    safeInput(node->location, MAX_REMARK_LEN, "请输入存放位置: ");
    safeInput(node->supplier, MAX_NAME_LEN, "请输入供应商: ");
    strcpy(node->lastInDate, "N/A"); strcpy(node->lastOutDate, "N/A");
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注 (无则留空): ");
    node->next = NULL;
    return node;
}

int isMaterialIDExist(MaterialList* list, const char* id) {
    MaterialNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return 1; current = current->next; }
    return 0;
}

void insertMaterialNode(MaterialList* list, MaterialNode* node) {
    if (node == NULL) return;
    node->next = list->head; list->head = node; list->count++;
}

MaterialNode* findMaterialByID(MaterialList* list, const char* id) {
    MaterialNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return current; current = current->next; }
    return NULL;
}

MaterialNode* findMaterialByName(MaterialList* list, const char* name) {
    MaterialNode* current = list->head;
    while (current != NULL) { if (strstr(current->name, name) != NULL) return current; current = current->next; }
    return NULL;
}

int deleteMaterialByID(MaterialList* list, const char* id) {
    MaterialNode* current = list->head, * prev = NULL;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printf("\n找到以下记录:\n"); printMaterialOne(current);
            if (!confirm("确认删除该物资?")) { printf("已取消删除。\n"); return 0; }

            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "物资编号:%s, 名称:%s, 类别:%s",
                current->id, current->name, current->category);

            if (prev == NULL) list->head = current->next;
            else prev->next = current->next;
            free(current); list->count--; printf("删除成功！\n");

            writeLog(LOG_LOGISTIC, "删除物资", "系统", detail);
            return 1;
        }
        prev = current; current = current->next;
    }
    printf("未找到编号为 %s 的物资。\n", id); return 0;
}

void modifyMaterialInfo(MaterialNode* node) {
    if (node == NULL) return;
    printf("\n--- 修改物资信息 ---\n"); printMaterialOne(node);
    printf("\n请选择要修改的字段:\n");
    printf("1. 名称\n2. 类别\n3. 库存数量\n4. 计量单位\n5. 最低库存值\n6. 存放位置\n7. 供应商\n8. 备注\n0. 取消\n");
    int choice = inputInt("请选择: ");
    switch (choice) {
    case 1: safeInput(node->name, MAX_NAME_LEN, "请输入新名称: "); break;
    case 2: safeInput(node->category, MAX_DEPT_LEN, "请输入新类别: "); break;
    case 3: node->quantity = inputInt("请输入新库存数量: "); break;
    case 4: safeInput(node->unit, 20, "请输入新计量单位: "); break;
    case 5: node->minStock = inputInt("请输入新最低库存值: "); break;
    case 6: safeInput(node->location, MAX_REMARK_LEN, "请输入新存放位置: "); break;
    case 7: safeInput(node->supplier, MAX_NAME_LEN, "请输入新供应商: "); break;
    case 8: safeInput(node->remark, MAX_REMARK_LEN, "请输入新备注: "); break;
    case 0: printf("取消修改。\n"); return;
    default: printf("无效选择。\n"); return;
    }
    printf("修改成功！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "物资编号:%s, 名称:%s, 修改字段:%d", node->id, node->name, choice);
    writeLog(LOG_LOGISTIC, "修改物资信息", "系统", detail);
}

void materialInStock(MaterialList* list, const char* id, int quantity) {
    MaterialNode* node = findMaterialByID(list, id);
    if (node == NULL) { printf("未找到该物资。\n"); return; }
    node->quantity += quantity;
    getCurrentDate(node->lastInDate);
    printf("入库成功！当前库存: %d %s\n", node->quantity, node->unit);

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "物资编号:%s, 名称:%s, 入库:%d, 当前库存:%d%s",
        node->id, node->name, quantity, node->quantity, node->unit);
    writeLog(LOG_LOGISTIC, "物资入库", "系统", detail);
}

int materialOutStock(MaterialList* list, const char* id, int quantity) {
    MaterialNode* node = findMaterialByID(list, id);
    if (node == NULL) { printf("未找到该物资。\n"); return 0; }
    if (node->quantity < quantity) { printf("库存不足！当前库存: %d %s\n", node->quantity, node->unit); return 0; }
    node->quantity -= quantity;
    getCurrentDate(node->lastOutDate);
    printf("出库成功！剩余库存: %d %s\n", node->quantity, node->unit);

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "物资编号:%s, 名称:%s, 出库:%d, 剩余库存:%d%s",
        node->id, node->name, quantity, node->quantity, node->unit);
    writeLog(LOG_LOGISTIC, "物资出库", "系统", detail);
    return 1;
}

void printMaterialOne(MaterialNode* node) {
    if (node == NULL) return;
    printLine('-', 70);
    printf("物资编号: %s\n", node->id);
    printf("物资名称: %s\n", node->name);
    printf("类别: %s\n", node->category);
    printf("库存: %d %s\n", node->quantity, node->unit);
    printf("最低库存: %d %s\n", node->minStock, node->unit);
    if (node->quantity <= node->minStock) printf("【库存预警】\n");
    printf("存放位置: %s\n", node->location);
    printf("供应商: %s\n", node->supplier);
    printf("最近入库: %s\n", node->lastInDate);
    printf("最近出库: %s\n", node->lastOutDate);
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

void printMaterialAll(MaterialList* list) {
    if (list->head == NULL) { printf("暂无物资记录。\n"); return; }
    printf("\n--- 所有物资 (共 %d 种) ---\n", list->count);
    MaterialNode* current = list->head;
    while (current != NULL) { printMaterialOne(current); current = current->next; }
}

void printStockWarning(MaterialList* list) {
    if (list->head == NULL) { printf("暂无物资记录。\n"); return; }
    int count = 0;
    MaterialNode* current = list->head;
    printf("\n--- 库存预警物资 ---\n");
    while (current != NULL) {
        if (current->quantity <= current->minStock) {
            printMaterialOne(current); count++;
        }
        current = current->next;
    }
    if (count == 0) printf("暂无库存预警物资。\n");
    else printf("共 %d 种物资需要补货。\n", count);
}

void insertUsageNode(UsageList* list, UsageNode* node) {
    if (node == NULL) return;
    node->next = list->head; list->head = node; list->count++;
}

void freeMaterialList(MaterialList* list) {
    MaterialNode* current = list->head;
    while (current != NULL) { MaterialNode* temp = current; current = current->next; free(temp); }
    list->head = NULL; list->count = 0;
}

void freeUsageList(UsageList* list) {
    UsageNode* current = list->head;
    while (current != NULL) { UsageNode* temp = current; current = current->next; free(temp); }
    list->head = NULL; list->count = 0;
}

void addMaterial(MaterialList* list) {
    MaterialNode* node = createMaterialNode();
    if (node == NULL) return;
    if (isMaterialIDExist(list, node->id)) {
        printf("错误：物资编号 %s 已存在！\n", node->id); free(node); return;
    }
    insertMaterialNode(list, node);
    printf("物资添加成功！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "物资编号:%s, 名称:%s, 类别:%s, 库存:%d%s",
        node->id, node->name, node->category, node->quantity, node->unit);
    writeLog(LOG_LOGISTIC, "新增物资", "系统", detail);
}

void queryMaterial(MaterialList* list) {
    if (list->head == NULL) { printf("暂无物资记录。\n"); return; }
    printf("\n--- 查询物资 ---\n");
    printf("1. 按编号查询\n2. 按名称查询\n3. 按类别查询\n4. 显示全部\n5. 查看库存预警\n0. 返回\n");
    int choice = inputInt("请选择: ");
    char keyword[MAX_NAME_LEN];
    switch (choice) {
    case 1:
        safeInput(keyword, MAX_ID_LEN, "请输入编号: ");
        MaterialNode* found = findMaterialByID(list, keyword);
        if (found) printMaterialOne(found);
        else printf("未找到。\n");
        break;
    case 2:
        safeInput(keyword, MAX_NAME_LEN, "请输入名称: ");
        MaterialNode* curr = list->head; int cnt = 0;
        while (curr != NULL) { if (strstr(curr->name, keyword)) { printMaterialOne(curr); cnt++; } curr = curr->next; }
        if (cnt == 0) printf("未找到。\n");
        break;
    case 3:
        safeInput(keyword, MAX_DEPT_LEN, "请输入类别: ");
        curr = list->head; cnt = 0;
        while (curr != NULL) { if (strstr(curr->category, keyword)) { printMaterialOne(curr); cnt++; } curr = curr->next; }
        if (cnt == 0) printf("未找到。\n");
        break;
    case 4: printMaterialAll(list); break;
    case 5: printStockWarning(list); break;
    case 0: return;
    default: printf("无效选择。\n");
    }
}

void deleteMaterial(MaterialList* list) {
    if (list->head == NULL) { printf("暂无物资记录。\n"); return; }
    char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要删除的物资编号: ");
    deleteMaterialByID(list, id);
}

void modifyMaterial(MaterialList* list) {
    if (list->head == NULL) { printf("暂无物资记录。\n"); return; }
    char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要修改的物资编号: ");
    MaterialNode* node = findMaterialByID(list, id);
    if (node == NULL) { printf("未找到。\n"); return; }
    modifyMaterialInfo(node);
}

void materialIn(MaterialList* list) {
    if (list->head == NULL) { printf("暂无物资记录。\n"); return; }
    char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入物资编号: ");
    int qty = inputInt("请输入入库数量: ");
    materialInStock(list, id, qty);
}

void materialOut(MaterialList* list, UsageList* uList) {
    if (list->head == NULL) { printf("暂无物资记录。\n"); return; }
    char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入物资编号: ");
    int qty = inputInt("请输入出库数量: ");
    if (materialOutStock(list, id, qty)) {
        UsageNode* unode = (UsageNode*)malloc(sizeof(UsageNode));
        if (unode) {
            generateID(unode->id, "U");
            strcpy(unode->materialId, id);
            safeInput(unode->dept, MAX_DEPT_LEN, "请输入领用部门: ");
            unode->quantity = qty;
            getCurrentDate(unode->date);
            safeInput(unode->handler, MAX_NAME_LEN, "请输入经手人: ");
            unode->auditStatus = 1;
            unode->remark[0] = '\0';
            unode->next = NULL;
            insertUsageNode(uList, unode);
        }
    }
}

void queryUsage(UsageList* list) {
    if (list->head == NULL) { printf("暂无领用记录。\n"); return; }
    printf("\n--- 领用记录 ---\n");
    UsageNode* current = list->head; int cnt = 0;
    while (current != NULL) {
        printf("%s | %s | %s | %d | %s\n", current->id, current->materialId, current->dept, current->quantity, current->date);
        cnt++; current = current->next;
    }
    printf("共 %d 条记录。\n", cnt);
}

void statMaterials(MaterialList* mList, UsageList* uList) {
    printf("\n--- 物资统计 ---\n");
    printf("物资种类: %d\n", mList->count);
    int totalQty = 0;
    MaterialNode* curr = mList->head;
    while (curr != NULL) { totalQty += curr->quantity; curr = curr->next; }
    printf("物资总数量: %d\n", totalQty);
    printf("领用记录数: %d\n", uList->count);
}

void logisticsMenu(MaterialList* mList, UsageList* uList) {
    int choice;
    do {
        printf("\n"); printTitle("后勤管理系统");
        printf("1. 新增物资\n2. 查询物资\n3. 删除物资\n4. 修改物资\n");
        printf("5. 物资入库\n6. 物资出库\n7. 领用记录\n8. 统计分析\n0. 返回主菜单\n");
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
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

#include "device.h"
#include "log.h"

const char* getDeviceStatusString(int status) {
    switch (status) {
    case DEVICE_NORMAL: return "正常";
    case DEVICE_REPAIRING: return "维修中";
    case DEVICE_SCRAPPED: return "已报废";
    default: return "未知";
    }
}

void initDeviceList(DeviceList* list) { list->head = NULL; list->count = 0; }
void initFixList(FixList* list) { list->head = NULL; list->count = 0; }

void loadDevicesFromFile(DeviceList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：设备数据文件不存在，将创建新文件。\n"); return; }
    DeviceNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        DeviceNode* node = (DeviceNode*)malloc(sizeof(DeviceNode));
        if (node == NULL) continue;
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
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条设备记录。\n", list->count);
}

void loadFixesFromFile(FixList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：维修记录文件不存在，将创建新文件。\n"); return; }
    FixNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        FixNode* node = (FixNode*)malloc(sizeof(FixNode));
        if (node == NULL) continue;
        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%lf|%[^|]|%[^|]|%[^\n]",
            node->id, node->deviceId, node->description,
            node->sendDate, node->finishDate, &node->cost,
            node->result, node->handler, node->remark);
        node->next = NULL;
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条维修记录。\n", list->count);
}

void saveDevicesToFile(DeviceList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }
    DeviceNode* current = list->head;
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

DeviceNode* createDeviceNode() {
    DeviceNode* node = (DeviceNode*)malloc(sizeof(DeviceNode));
    if (node == NULL) { printf("错误：内存分配失败！\n"); return NULL; }
    printf("\n--- 新增设备 ---\n");
    safeInput(node->id, MAX_ID_LEN, "请输入设备编号: ");
    while (isEmpty(node->id)) { printf("编号不能为空！\n"); safeInput(node->id, MAX_ID_LEN, "请输入设备编号: "); }
    safeInput(node->name, MAX_NAME_LEN, "请输入设备名称: ");
    safeInput(node->category, MAX_DEPT_LEN, "请输入设备类别: ");
    safeInput(node->department, MAX_DEPT_LEN, "请输入所属科室: ");
    safeInput(node->purchaseDate, MAX_DATE_LEN, "请输入购入日期: ");
    while (!isValidDate(node->purchaseDate)) { printf("日期格式不正确！\n"); safeInput(node->purchaseDate, MAX_DATE_LEN, "请输入购入日期: "); }
    node->price = inputDouble("请输入购入价格: ");
    node->status = DEVICE_NORMAL;
    node->needPurchase = 0;
    node->purchaseQuantity = 0;
    safeInput(node->handler, MAX_NAME_LEN, "请输入责任人: ");
    safeInput(node->location, MAX_REMARK_LEN, "请输入存放位置: ");
    safeInput(node->warrantyPeriod, 20, "请输入保修期: ");
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注: ");
    node->next = NULL;
    return node;
}

int isDeviceIDExist(DeviceList* list, const char* id) {
    DeviceNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return 1; current = current->next; }
    return 0;
}

void insertDeviceNode(DeviceList* list, DeviceNode* node) {
    if (node == NULL) return;
    node->next = list->head; list->head = node; list->count++;
}

DeviceNode* findDeviceByID(DeviceList* list, const char* id) {
    DeviceNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return current; current = current->next; }
    return NULL;
}

int deleteDeviceByID(DeviceList* list, const char* id) {
    DeviceNode* current = list->head, * prev = NULL;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printf("\n找到以下记录:\n"); printDeviceOne(current);
            if (!confirm("确认删除该设备?")) { printf("已取消删除。\n"); return 0; }

            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "设备编号:%s, 名称:%s, 类别:%s",
                current->id, current->name, current->category);

            if (prev == NULL) list->head = current->next;
            else prev->next = current->next;
            free(current); list->count--; printf("删除成功！\n");

            writeLog(LOG_DEVICE, "删除设备", "系统", detail);
            return 1;
        }
        prev = current; current = current->next;
    }
    printf("未找到编号为 %s 的设备。\n", id); return 0;
}

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
        printf("状态选项:\n0.正常 1.维修中 2.已报废\n");
        node->status = inputInt("请选择: ");
        break;
    case 7: safeInput(node->remark, MAX_REMARK_LEN, "请输入新备注: "); break;
    case 0: printf("取消修改。\n"); return;
    default: printf("无效选择。\n"); return;
    }
    printf("修改成功！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "设备编号:%s, 名称:%s, 修改字段:%d", node->id, node->name, choice);
    writeLog(LOG_DEVICE, "修改设备信息", "系统", detail);
}

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

void printDeviceAll(DeviceList* list) {
    if (list->head == NULL) { printf("暂无设备记录。\n"); return; }
    printf("\n--- 所有设备 (共 %d 台) ---\n", list->count);
    DeviceNode* current = list->head;
    while (current != NULL) { printDeviceOne(current); current = current->next; }
}

void scrapDevice(DeviceList* list, const char* id) {
    DeviceNode* node = findDeviceByID(list, id);
    if (node == NULL) { printf("未找到该设备。\n"); return; }
    if (node->status == DEVICE_SCRAPPED) { printf("该设备已报废。\n"); return; }
    if (!confirm("确认报废该设备?")) { printf("取消操作。\n"); return; }
    node->status = DEVICE_SCRAPPED;
    printf("设备 %s 已标记为报废。\n", id);
}

void statDevices(DeviceList* dList, FixList* fList) {
    int normal = 0, repairing = 0, scrapped = 0;
    double totalValue = 0;
    DeviceNode* curr = dList->head;

    while (curr != NULL) {
        switch (curr->status) {
        case DEVICE_NORMAL: normal++; totalValue += curr->price; break;
        case DEVICE_REPAIRING: repairing++; break;
        case DEVICE_SCRAPPED: scrapped++; break;
        }
        curr = curr->next;
    }

    printf("\n--- 设备统计 ---\n");
    printf("设备总数: %d\n", dList->count);
    printf("正常使用: %d\n", normal);
    printf("维修中: %d\n", repairing);
    printf("已报废: %d\n", scrapped);
    printf("在用设备总价值: %.2f\n", totalValue);
    printf("维修记录数: %d\n", fList->count);
}

void freeDeviceList(DeviceList* list) {
    DeviceNode* current = list->head;
    while (current != NULL) { DeviceNode* temp = current; current = current->next; free(temp); }
    list->head = NULL; list->count = 0;
}
void freeFixList(FixList* list) {
    FixNode* current = list->head;
    while (current != NULL) { FixNode* temp = current; current = current->next; free(temp); }
    list->head = NULL; list->count = 0;
}

void deviceMenu(DeviceList* dList, FixList* fList) {
    int choice;
    do {
        printf("\n"); printTitle("医疗设备管理系统");
        printf("1. 新增设备\n2. 查询设备\n3. 删除设备\n4. 修改设备\n");
        printf("5. 设备报废\n6. 统计分析\n0. 返回主菜单\n");
        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: {
            DeviceNode* node = createDeviceNode();
            if (node && !isDeviceIDExist(dList, node->id)) {
                insertDeviceNode(dList, node);
                printf("设备添加成功！\n");
                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "设备编号:%s, 名称:%s, 类别:%s, 部门:%s, 价格:%.2f",
                    node->id, node->name, node->category, node->department, node->price);
                writeLog(LOG_DEVICE, "新增设备", "系统", detail);
            }
            else if (node) { printf("错误：设备编号已存在！\n"); free(node); }
            break;
        }
        case 2: {
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
            if (dList->head == NULL) { printf("暂无设备记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要删除的设备编号: ");
            deleteDeviceByID(dList, id);
            break;
        }
        case 4: {
            if (dList->head == NULL) { printf("暂无设备记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要修改的设备编号: ");
            DeviceNode* node = findDeviceByID(dList, id);
            if (node) modifyDeviceInfo(node); else printf("未找到。\n");
            break;
        }
        case 5: {
            if (dList->head == NULL) { printf("暂无设备记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要报废的设备编号: ");
            scrapDevice(dList, id);
            break;
        }
        case 6: statDevices(dList, fList); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

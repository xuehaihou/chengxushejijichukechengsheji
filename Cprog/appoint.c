#include "appoint.h"
#include "log.h"

const char* getAppointTypeString(int type) {
    switch (type) {
    case APPOINT_OUTPATIENT: return "门诊预约";
    case APPOINT_SURGERY: return "手术预约";
    case APPOINT_INPATIENT: return "住院预约";
    default: return "未知";
    }
}

const char* getAppointStatusString(int status) {
    switch (status) {
    case APPOINT_PENDING: return "未到诊";
    case APPOINT_ARRIVED: return "已到诊";
    case APPOINT_COMPLETED: return "已完成";
    case APPOINT_CANCELLED: return "已取消";
    default: return "未知";
    }
}

void initAppointList(AppointList* list) { list->head = NULL; list->count = 0; }

void loadAppointsFromFile(AppointList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) { printf("提示：预约数据文件不存在，将创建新文件。\n"); return; }
    AppointNode* tail = NULL; char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        AppointNode* node = (AppointNode*)malloc(sizeof(AppointNode));
        if (node == NULL) continue;
        sscanf_s(line, "%[^|]|%d|%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^\n]",
            node->id, &node->type, node->patientId, node->patientName,
            node->gender, &node->age, node->department, node->doctor,
            node->date, node->timeSlot, &node->status, node->registerPerson, node->remark);
        node->next = NULL;
        if (list->head == NULL) { list->head = node; tail = node; }
        else { tail->next = node; tail = node; }
        list->count++;
    }
    fclose(fp);
    printf("成功加载 %d 条预约记录。\n", list->count);
}

void saveAppointsToFile(AppointList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) { printf("错误：无法打开文件 %s 进行写入！\n", filename); return; }
    AppointNode* current = list->head;
    while (current != NULL) {
        fprintf(fp, "%s|%d|%s|%s|%s|%d|%s|%s|%s|%s|%d|%s|%s\n",
            current->id, current->type, current->patientId, current->patientName,
            current->gender, current->age, current->department, current->doctor,
            current->date, current->timeSlot, current->status,
            current->registerPerson, current->remark);
        current = current->next;
    }
    fclose(fp);
    printf("成功保存 %d 条预约记录。\n", list->count);
}

AppointNode* createAppointNode() {
    AppointNode* node = (AppointNode*)malloc(sizeof(AppointNode));
    if (node == NULL) { printf("错误：内存分配失败！\n"); return NULL; }
    printf("\n--- 新增预约 ---\n");

    safeInput(node->id, MAX_ID_LEN, "请输入预约编号: ");
    while (isEmpty(node->id)) { printf("编号不能为空！\n"); safeInput(node->id, MAX_ID_LEN, "请输入预约编号: "); }

    printf("预约类型:\n0.门诊预约 1.手术预约 2.住院预约\n");
    node->type = inputInt("请选择: ");
    while (node->type < 0 || node->type > 2) { printf("无效选择！\n"); node->type = inputInt("请选择: "); }

    safeInput(node->patientId, MAX_ID_LEN, "请输入病人编号: ");
    safeInput(node->patientName, MAX_NAME_LEN, "请输入病人姓名: ");
    safeInput(node->gender, 10, "请输入性别: ");
    node->age = inputInt("请输入年龄: ");
    safeInput(node->department, MAX_DEPT_LEN, "请输入科室: ");
    safeInput(node->doctor, MAX_NAME_LEN, "请输入医生: ");
    safeInput(node->date, MAX_DATE_LEN, "请输入预约日期: ");
    while (!isValidDate(node->date)) { printf("日期格式不正确！\n"); safeInput(node->date, MAX_DATE_LEN, "请输入预约日期: "); }
    safeInput(node->timeSlot, 20, "请输入时间段(如:上午/下午): ");
    node->status = APPOINT_PENDING;
    safeInput(node->registerPerson, MAX_NAME_LEN, "请输入登记人: ");
    safeInput(node->remark, MAX_REMARK_LEN, "请输入备注: ");
    node->next = NULL;
    return node;
}

int isAppointIDExist(AppointList* list, const char* id) {
    AppointNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return 1; current = current->next; }
    return 0;
}

void insertAppointNode(AppointList* list, AppointNode* node) {
    if (node == NULL) return;
    node->next = list->head; list->head = node; list->count++;
}

AppointNode* findAppointByID(AppointList* list, const char* id) {
    AppointNode* current = list->head;
    while (current != NULL) { if (strcmp(current->id, id) == 0) return current; current = current->next; }
    return NULL;
}

int deleteAppointByID(AppointList* list, const char* id) {
    AppointNode* current = list->head, * prev = NULL;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            printf("\n找到以下记录:\n"); printAppointOne(current);
            if (!confirm("确认取消该预约?")) { printf("已取消操作。\n"); return 0; }

            char detail[MAX_LOG_DETAIL];
            sprintf(detail, "预约编号:%s, 病人:%s, 类型:%s, 日期:%s",
                current->id, current->patientName, getAppointTypeString(current->type), current->date);

            if (prev == NULL) list->head = current->next;
            else prev->next = current->next;
            free(current); list->count--; printf("预约已取消！\n");

            writeLog(LOG_APPOINT, "取消预约", "系统", detail);
            return 1;
        }
        prev = current; current = current->next;
    }
    printf("未找到编号为 %s 的预约。\n", id); return 0;
}

void modifyAppointInfo(AppointNode* node) {
    if (node == NULL) return;
    printf("\n--- 修改预约信息 ---\n"); printAppointOne(node);
    printf("\n请选择要修改的字段:\n");
    printf("1. 科室\n2. 医生\n3. 预约日期\n4. 时间段\n5. 状态\n6. 备注\n0. 取消\n");
    int choice = inputInt("请选择: ");
    switch (choice) {
    case 1: safeInput(node->department, MAX_DEPT_LEN, "请输入新科室: "); break;
    case 2: safeInput(node->doctor, MAX_NAME_LEN, "请输入新医生: "); break;
    case 3:
        safeInput(node->date, MAX_DATE_LEN, "请输入新日期: ");
        while (!isValidDate(node->date)) { printf("日期格式不正确！\n"); safeInput(node->date, MAX_DATE_LEN, "请输入新日期: "); }
        break;
    case 4: safeInput(node->timeSlot, 20, "请输入新时间段: "); break;
    case 5:
        printf("状态选项:\n0.未到诊 1.已到诊 2.已完成 3.已取消\n");
        node->status = inputInt("请选择: ");
        break;
    case 6: safeInput(node->remark, MAX_REMARK_LEN, "请输入新备注: "); break;
    case 0: printf("取消修改。\n"); return;
    default: printf("无效选择。\n"); return;
    }
    printf("修改成功！\n");

    char detail[MAX_LOG_DETAIL];
    sprintf(detail, "预约编号:%s, 病人:%s, 修改字段:%d", node->id, node->patientName, choice);
    writeLog(LOG_APPOINT, "修改预约", "系统", detail);
}

void printAppointOne(AppointNode* node) {
    if (node == NULL) return;
    printLine('-', 70);
    printf("预约编号: %s\n", node->id);
    printf("预约类型: %s\n", getAppointTypeString(node->type));
    printf("病人编号: %s\n", node->patientId);
    printf("病人姓名: %s\n", node->patientName);
    printf("性别: %s 年龄: %d\n", node->gender, node->age);
    printf("科室: %s\n", node->department);
    printf("医生: %s\n", node->doctor);
    printf("预约日期: %s\n", node->date);
    printf("时间段: %s\n", node->timeSlot);
    printf("状态: %s\n", getAppointStatusString(node->status));
    printf("登记人: %s\n", node->registerPerson);
    printf("备注: %s\n", node->remark);
    printLine('-', 70);
}

void printAppointAll(AppointList* list) {
    if (list->head == NULL) { printf("暂无预约记录。\n"); return; }
    printf("\n--- 所有预约 (共 %d 条) ---\n", list->count);
    AppointNode* current = list->head;
    while (current != NULL) { printAppointOne(current); current = current->next; }
}

void statAppoints(AppointList* list) {
    int outpatient = 0, surgery = 0, inpatient = 0;
    int pending = 0, arrived = 0, completed = 0, cancelled = 0;

    AppointNode* curr = list->head;
    while (curr != NULL) {
        switch (curr->type) {
        case APPOINT_OUTPATIENT: outpatient++; break;
        case APPOINT_SURGERY: surgery++; break;
        case APPOINT_INPATIENT: inpatient++; break;
        }
        switch (curr->status) {
        case APPOINT_PENDING: pending++; break;
        case APPOINT_ARRIVED: arrived++; break;
        case APPOINT_COMPLETED: completed++; break;
        case APPOINT_CANCELLED: cancelled++; break;
        }
        curr = curr->next;
    }

    printf("\n--- 预约统计 ---\n");
    printf("按类型统计:\n");
    printf("  门诊预约: %d\n", outpatient);
    printf("  手术预约: %d\n", surgery);
    printf("  住院预约: %d\n", inpatient);
    printf("按状态统计:\n");
    printf("  未到诊: %d\n", pending);
    printf("  已到诊: %d\n", arrived);
    printf("  已完成: %d\n", completed);
    printf("  已取消: %d\n", cancelled);
    printf("总预约数: %d\n", list->count);
}

void freeAppointList(AppointList* list) {
    AppointNode* current = list->head;
    while (current != NULL) { AppointNode* temp = current; current = current->next; free(temp); }
    list->head = NULL; list->count = 0;
}

void appointMenu(AppointList* list) {
    int choice;
    do {
        printf("\n"); printTitle("手术及住院预约系统");
        printf("1. 新增预约\n2. 查询预约\n3. 取消预约\n4. 修改预约\n");
        printf("5. 显示全部预约\n6. 统计分析\n0. 返回主菜单\n");
        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: {
            AppointNode* node = createAppointNode();
            if (node && !isAppointIDExist(list, node->id)) {
                insertAppointNode(list, node);
                printf("预约添加成功！\n");
                char detail[MAX_LOG_DETAIL];
                sprintf(detail, "预约编号:%s, 病人:%s, 类型:%s, 科室:%s, 医生:%s, 日期:%s",
                    node->id, node->patientName, getAppointTypeString(node->type),
                    node->department, node->doctor, node->date);
                writeLog(LOG_APPOINT, "新增预约", "系统", detail);
            }
            else if (node) { printf("错误：预约编号已存在！\n"); free(node); }
            break;
        }
        case 2: {
            if (list->head == NULL) { printf("暂无预约记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入预约编号: ");
            AppointNode* found = findAppointByID(list, id);
            if (found) printAppointOne(found); else printf("未找到。\n");
            break;
        }
        case 3: {
            if (list->head == NULL) { printf("暂无预约记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要取消的预约编号: ");
            deleteAppointByID(list, id);
            break;
        }
        case 4: {
            if (list->head == NULL) { printf("暂无预约记录。\n"); break; }
            char id[MAX_ID_LEN]; safeInput(id, MAX_ID_LEN, "请输入要修改的预约编号: ");
            AppointNode* node = findAppointByID(list, id);
            if (node) modifyAppointInfo(node); else printf("未找到。\n");
            break;
        }
        case 5: printAppointAll(list); break;
        case 6: statAppoints(list); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

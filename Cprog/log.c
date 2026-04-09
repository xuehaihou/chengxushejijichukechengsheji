#include "log.h"
#include <time.h>

const char* getLogFileName(int logType) {
    switch (logType) {
    case LOG_MEDICINE:   return "data/log_medicine.txt";
    case LOG_OUTPATIENT: return "data/log_outpatient.txt";
    case LOG_APPOINT:    return "data/log_appoint.txt";
    case LOG_INPATIENT:  return "data/log_inpatient.txt";
    case LOG_EMERGENCY:  return "data/log_emergency.txt";
    case LOG_BLOOD:      return "data/log_blood.txt";
    case LOG_DEVICE:     return "data/log_device.txt";
    case LOG_FINANCE:    return "data/log_finance.txt";
    case LOG_STAFF:      return "data/log_staff.txt";
    case LOG_LOGISTIC:   return "data/log_logistic.txt";
    default:             return "data/log_other.txt";
    }
}

const char* getLogTypeName(int logType) {
    switch (logType) {
    case LOG_MEDICINE:   return "药品管理";
    case LOG_OUTPATIENT: return "门诊管理";
    case LOG_APPOINT:    return "预约管理";
    case LOG_INPATIENT:  return "住院管理";
    case LOG_EMERGENCY:  return "急诊管理";
    case LOG_BLOOD:      return "血库管理";
    case LOG_DEVICE:     return "设备管理";
    case LOG_FINANCE:    return "财务管理";
    case LOG_STAFF:      return "人事管理";
    case LOG_LOGISTIC:   return "后勤管理";
    default:             return "其他";
    }
}

void initLogList(LogList* list) {
    list->head = NULL;
    list->count = 0;
}

void freeLogList(LogList* list) {
    LogNode* current = list->head;
    while (current != NULL) {
        LogNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->count = 0;
}

void getCurrentDateTime(char* datetime) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);
}

void writeLog(int logType, const char* operation, const char* operator, const char* detail) {
    const char* filename = getLogFileName(logType);
    FILE* fp = fopen(filename, "a");
    if (fp == NULL) {
        system("if not exist data mkdir data 2>nul");
        fp = fopen(filename, "a");
        if (fp == NULL) {
            printf("错误：无法写入日志文件！\n");
            return;
        }
    }

    char datetime[20];
    getCurrentDateTime(datetime);

    fprintf(fp, "%s|%s|%s|%s\n", datetime, operation, operator, detail);
    fclose(fp);
}

void loadLogFromFile(LogList* list, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return;
    }

    LogNode* tail = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        LogNode* node = (LogNode*)malloc(sizeof(LogNode));
        if (node == NULL) continue;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^\n]",
            node->datetime, node->operation, node->operator, node->detail);

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
}

void saveLogToFile(LogList* list, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法保存日志文件！\n");
        return;
    }

    LogNode* current = list->head;
    while (current != NULL) {
        fprintf(fp, "%s|%s|%s|%s\n",
            current->datetime, current->operation,
            current->operator, current->detail);
        current = current->next;
    }

    fclose(fp);
}

void printLogOne(LogNode* node) {
    if (node == NULL) return;
    printLine('-', 70);
    printf("时间: %s\n", node->datetime);
    printf("操作: %s\n", node->operation);
    printf("操作人: %s\n", node->operator);
    printf("详情: %s\n", node->detail);
    printLine('-', 70);
}

void printLogAll(LogList* list) {
    if (list->head == NULL) {
        printf("暂无日志记录。\n");
        return;
    }

    printf("\n--- 日志记录 (共 %d 条) ---\n", list->count);
    LogNode* current = list->head;
    while (current != NULL) {
        printLogOne(current);
        current = current->next;
    }
}

int isAdminAuthenticated() {
    char password[50];
    printf("\n=== 管理员验证 ===\n");
    printf("请输入管理员密码: ");

    int i = 0;
    char ch;
    while ((ch = _getch()) != '\r' && i < 49) {
        if (ch == '\b') {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        }
        else {
            password[i++] = ch;
            printf("*");
        }
    }
    password[i] = '\0';
    printf("\n");

    if (strcmp(password, ADMIN_PASSWORD) == 0) {
        printf("验证成功！\n");
        return 1;
    }
    else {
        printf("密码错误！拒绝访问。\n");
        return 0;
    }
}

void queryLogByType(int logType) {
    LogList list;
    initLogList(&list);

    const char* filename = getLogFileName(logType);
    loadLogFromFile(&list, filename);

    printf("\n=== %s 日志 ===\n", getLogTypeName(logType));
    printLogAll(&list);

    freeLogList(&list);
}

void queryAllLogs() {
    printf("\n=== 所有操作日志 ===\n");

    int logTypes[] = { LOG_MEDICINE, LOG_OUTPATIENT, LOG_APPOINT,
                       LOG_INPATIENT, LOG_EMERGENCY, LOG_BLOOD,
                       LOG_DEVICE, LOG_FINANCE, LOG_STAFF, LOG_LOGISTIC };
    int count = 10;

    for (int i = 0; i < count; i++) {
        LogList list;
        initLogList(&list);

        const char* filename = getLogFileName(logTypes[i]);
        loadLogFromFile(&list, filename);

        if (list.count > 0) {
            printf("\n--- %s (%d条) ---\n", getLogTypeName(logTypes[i]), list.count);
            printLogAll(&list);
        }

        freeLogList(&list);
    }
}

void queryLogByDate() {
    char startDate[20], endDate[20];
    printf("\n=== 按日期查询日志 ===\n");
    safeInput(startDate, 20, "请输入开始日期 (YYYY-MM-DD): ");
    safeInput(endDate, 20, "请输入结束日期 (YYYY-MM-DD): ");

    int logTypes[] = { LOG_MEDICINE, LOG_OUTPATIENT, LOG_APPOINT,
                       LOG_INPATIENT, LOG_EMERGENCY, LOG_BLOOD,
                       LOG_DEVICE, LOG_FINANCE, LOG_STAFF, LOG_LOGISTIC };
    int count = 10;
    int totalFound = 0;

    for (int i = 0; i < count; i++) {
        LogList list;
        initLogList(&list);

        const char* filename = getLogFileName(logTypes[i]);
        loadLogFromFile(&list, filename);

        LogNode* current = list.head;
        int found = 0;
        while (current != NULL) {
            char logDate[11];
            strncpy(logDate, current->datetime, 10);
            logDate[10] = '\0';

            if (compareDate(logDate, startDate) >= 0 &&
                compareDate(logDate, endDate) <= 0) {
                if (found == 0) {
                    printf("\n--- %s ---\n", getLogTypeName(logTypes[i]));
                }
                printLogOne(current);
                found++;
                totalFound++;
            }
            current = current->next;
        }

        freeLogList(&list);
    }

    if (totalFound == 0) {
        printf("未找到符合条件的日志记录。\n");
    }
    else {
        printf("\n共找到 %d 条记录。\n", totalFound);
    }
}

void logMenu() {
    if (!isAdminAuthenticated()) {
        return;
    }

    int choice;
    do {
        printf("\n");
        printTitle("日志查询系统 (管理员)");
        printf("1. 查看药品管理日志\n");
        printf("2. 查看门诊管理日志\n");
        printf("3. 查看预约管理日志\n");
        printf("4. 查看住院管理日志\n");
        printf("5. 查看急诊管理日志\n");
        printf("6. 查看血库管理日志\n");
        printf("7. 查看设备管理日志\n");
        printf("8. 查看财务管理日志\n");
        printf("9. 查看人事管理日志\n");
        printf("10. 查看后勤管理日志\n");
        printf("11. 查看所有日志\n");
        printf("12. 按日期查询日志\n");
        printf("0. 返回主菜单\n");

        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: queryLogByType(LOG_MEDICINE); break;
        case 2: queryLogByType(LOG_OUTPATIENT); break;
        case 3: queryLogByType(LOG_APPOINT); break;
        case 4: queryLogByType(LOG_INPATIENT); break;
        case 5: queryLogByType(LOG_EMERGENCY); break;
        case 6: queryLogByType(LOG_BLOOD); break;
        case 7: queryLogByType(LOG_DEVICE); break;
        case 8: queryLogByType(LOG_FINANCE); break;
        case 9: queryLogByType(LOG_STAFF); break;
        case 10: queryLogByType(LOG_LOGISTIC); break;
        case 11: queryAllLogs(); break;
        case 12: queryLogByDate(); break;
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }

        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

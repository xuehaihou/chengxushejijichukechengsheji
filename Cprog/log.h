#ifndef LOG_H
#define LOG_H

#include "utils.h"
#include <conio.h>

#define MAX_LOG_DETAIL 500
#define ADMIN_PASSWORD "admin123"

#define LOG_MEDICINE    1
#define LOG_OUTPATIENT  2
#define LOG_APPOINT     3
#define LOG_INPATIENT   4
#define LOG_EMERGENCY   5
#define LOG_BLOOD       6
#define LOG_DEVICE      7
#define LOG_FINANCE     8
#define LOG_STAFF       9
#define LOG_LOGISTIC    10

typedef struct LogNode {
    char datetime[20];
    char operation[50];
    char operator[50];
    char detail[MAX_LOG_DETAIL];
    struct LogNode* next;
} LogNode;

typedef struct {
    LogNode* head;
    int count;
} LogList;

void initLogList(LogList* list);
void freeLogList(LogList* list);

void writeLog(int logType, const char* operation, const char* operator, const char* detail);
void loadLogFromFile(LogList* list, const char* filename);
void saveLogToFile(LogList* list, const char* filename);

void printLogOne(LogNode* node);
void printLogAll(LogList* list);

int isAdminAuthenticated();
void logMenu();

const char* getLogFileName(int logType);
const char* getLogTypeName(int logType);

#endif

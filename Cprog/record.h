#ifndef RECORD_H
#define RECORD_H

#include "utils.h"

typedef struct RecordNode {
    char id[MAX_ID_LEN];
    char patientId[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char gender[10];
    int age;
    char department[MAX_DEPT_LEN];
    char doctor[MAX_NAME_LEN];
    char admitDiagnosis[MAX_REMARK_LEN];
    char dischargeDiagnosis[MAX_REMARK_LEN];
    char treatmentSummary[500];
    char dischargeDate[MAX_DATE_LEN];
    int archiveStatus;
    char remark[MAX_REMARK_LEN];
    struct RecordNode* next;
} RecordNode;

typedef struct {
    RecordNode* head;
    int count;
} RecordList;

void initRecordList(RecordList* list);
void loadRecordsFromFile(RecordList* list, const char* filename);
void saveRecordsToFile(RecordList* list, const char* filename);
RecordNode* createRecordNode();
int isRecordIDExist(RecordList* list, const char* id);
void insertRecordNode(RecordList* list, RecordNode* node);
RecordNode* findRecordByID(RecordList* list, const char* id);
int deleteRecordByID(RecordList* list, const char* id);
void modifyRecordInfo(RecordNode* node);
void printRecordOne(RecordNode* node);
void printRecordAll(RecordList* list);
void freeRecordList(RecordList* list);
void recordMenu(RecordList* list);

#endif

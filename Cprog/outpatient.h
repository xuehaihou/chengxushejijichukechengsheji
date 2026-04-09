#ifndef OUTPATIENT_H
#define OUTPATIENT_H

#include "utils.h"

typedef struct OutpatientNode {
    char id[MAX_ID_LEN];
    char patientId[MAX_ID_LEN];
    char patientName[MAX_NAME_LEN];
    char department[MAX_DEPT_LEN];
    char doctor[MAX_NAME_LEN];
    char date[MAX_DATE_LEN];
    char diagnosis[MAX_REMARK_LEN];
    char prescription[500];
    double cost;
    char remark[MAX_REMARK_LEN];
    struct OutpatientNode* next;
} OutpatientNode;

typedef struct {
    OutpatientNode* head;
    int count;
} OutpatientList;

void initOutpatientList(OutpatientList* list);
void loadOutpatientsFromFile(OutpatientList* list, const char* filename);
void saveOutpatientsToFile(OutpatientList* list, const char* filename);
OutpatientNode* createOutpatientNode();
int isOutpatientIDExist(OutpatientList* list, const char* id);
void insertOutpatientNode(OutpatientList* list, OutpatientNode* node);
OutpatientNode* findOutpatientByID(OutpatientList* list, const char* id);
int deleteOutpatientByID(OutpatientList* list, const char* id);
void modifyOutpatientInfo(OutpatientNode* node);
void printOutpatientOne(OutpatientNode* node);
void printOutpatientAll(OutpatientList* list);
void statOutpatients(OutpatientList* list);
void freeOutpatientList(OutpatientList* list);
void outpatientMenu(OutpatientList* list);

#endif

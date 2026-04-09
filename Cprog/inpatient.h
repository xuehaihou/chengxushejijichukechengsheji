#ifndef INPATIENT_H
#define INPATIENT_H

#include "utils.h"

// Inpatient status enumeration
typedef enum {
    INPATIENT_ADMITTED = 0,
    INPATIENT_TRANSFERRED,
    INPATIENT_DISCHARGED
} InpatientStatus;

// Inpatient node structure
typedef struct InpatientNode {
    char id[MAX_ID_LEN];
    char patientId[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char gender[10];
    int age;
    char admitDate[MAX_DATE_LEN];
    char department[MAX_DEPT_LEN];
    char ward[20];
    char roomNo[20];
    char bedNo[20];
    char doctor[MAX_NAME_LEN];
    char diagnosis[MAX_REMARK_LEN];
    double deposit;
    double dailyCost;
    double totalCost;
    int status;
    char dischargeDate[MAX_DATE_LEN];
    char remark[MAX_REMARK_LEN];
    struct InpatientNode* next;
} InpatientNode;

typedef struct {
    InpatientNode* head;
    int count;
} InpatientList;

const char* getInpatientStatusString(int status);
void initInpatientList(InpatientList* list);
void loadInpatientsFromFile(InpatientList* list, const char* filename);
void saveInpatientsToFile(InpatientList* list, const char* filename);
InpatientNode* createInpatientNode();
int isInpatientIDExist(InpatientList* list, const char* id);
void insertInpatientNode(InpatientList* list, InpatientNode* node);
InpatientNode* findInpatientByID(InpatientList* list, const char* id);
int deleteInpatientByID(InpatientList* list, const char* id);
void modifyInpatientInfo(InpatientNode* node);
void printInpatientOne(InpatientNode* node);
void printInpatientAll(InpatientList* list);
void dischargePatient(InpatientList* list, const char* id);
void statInpatients(InpatientList* list);
void freeInpatientList(InpatientList* list);
void inpatientMenu(InpatientList* list);

#endif

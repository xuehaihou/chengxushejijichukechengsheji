#ifndef EMERGENCY_H
#define EMERGENCY_H

#include "utils.h"

typedef struct EmergencyNode {
    char id[MAX_ID_LEN];
    char patientId[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char gender[10];
    int age;
    char arriveTime[MAX_DATE_LEN];
    int level;
    char symptoms[MAX_REMARK_LEN];
    char doctor[MAX_NAME_LEN];
    char result[MAX_REMARK_LEN];
    int status;
    char remark[MAX_REMARK_LEN];
    struct EmergencyNode* next;
} EmergencyNode;

typedef struct {
    EmergencyNode* head;
    int count;
} EmergencyList;

void initEmergencyList(EmergencyList* list);
void loadEmergenciesFromFile(EmergencyList* list, const char* filename);
void saveEmergenciesToFile(EmergencyList* list, const char* filename);
EmergencyNode* createEmergencyNode();
int isEmergencyIDExist(EmergencyList* list, const char* id);
void insertEmergencyNode(EmergencyList* list, EmergencyNode* node);
EmergencyNode* findEmergencyByID(EmergencyList* list, const char* id);
int deleteEmergencyByID(EmergencyList* list, const char* id);
void modifyEmergencyInfo(EmergencyNode* node);
void printEmergencyOne(EmergencyNode* node);
void printEmergencyAll(EmergencyList* list);
void statEmergencies(EmergencyList* list);
void freeEmergencyList(EmergencyList* list);
void emergencyMenu(EmergencyList* list);

#endif

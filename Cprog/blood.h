#ifndef BLOOD_H
#define BLOOD_H

#include "utils.h"

typedef struct BloodNode {
    char id[MAX_ID_LEN];
    char bloodType[5];
    char rhType[3];
    int quantity;
    char unit[20];
    char collectDate[MAX_DATE_LEN];
    char expiryDate[MAX_DATE_LEN];
    char source[MAX_NAME_LEN];
    int status;
    char remark[MAX_REMARK_LEN];
    struct BloodNode* next;
} BloodNode;

typedef struct {
    BloodNode* head;
    int count;
} BloodList;

void initBloodList(BloodList* list);
void loadBloodsFromFile(BloodList* list, const char* filename);
void saveBloodsToFile(BloodList* list, const char* filename);
BloodNode* createBloodNode();
int isBloodIDExist(BloodList* list, const char* id);
void insertBloodNode(BloodList* list, BloodNode* node);
BloodNode* findBloodByID(BloodList* list, const char* id);
int deleteBloodByID(BloodList* list, const char* id);
void modifyBloodInfo(BloodNode* node);
void printBloodOne(BloodNode* node);
void printBloodAll(BloodList* list);
int bloodInStock(BloodList* list, const char* id, int quantity);
int bloodOutStock(BloodList* list, const char* id, int quantity);
void statByBloodType(BloodList* list);
void freeBloodList(BloodList* list);
void bloodMenu(BloodList* list);

#endif

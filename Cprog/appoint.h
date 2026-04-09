#ifndef APPOINT_H
#define APPOINT_H

#include "utils.h"

// Appointment type enumeration
typedef enum {
    APPOINT_OUTPATIENT = 0,  // outpatient
    APPOINT_SURGERY,         // surgery
    APPOINT_INPATIENT        // inpatient
} AppointType;

// Appointment status enumeration
typedef enum {
    APPOINT_PENDING = 0,     // pending
    APPOINT_ARRIVED,         // arrived
    APPOINT_COMPLETED,       // completed
    APPOINT_CANCELLED        // cancelled
} AppointStatus;

// Appointment node structure
typedef struct AppointNode {
    char id[MAX_ID_LEN];            // appointment id
    int type;                       // appointment type
    char patientId[MAX_ID_LEN];     // patient id
    char patientName[MAX_NAME_LEN]; // patient name
    char gender[10];                // patient gender
    int age;                        // patient age
    char department[MAX_DEPT_LEN];  // department
    char doctor[MAX_NAME_LEN];      // doctor
    char date[MAX_DATE_LEN];        // appointment date
    char timeSlot[20];              // time slot
    int status;                     // status
    char registerPerson[MAX_NAME_LEN]; // registrar
    char remark[MAX_REMARK_LEN];    // remark
    struct AppointNode* next;       // next node
} AppointNode;

// Appointment list structure
typedef struct {
    AppointNode* head;
    int count;
} AppointList;

// Initialize appointment list
void initAppointList(AppointList* list);

// Load appointments from file
void loadAppointsFromFile(AppointList* list, const char* filename);

// Save appointments to file
void saveAppointsToFile(AppointList* list, const char* filename);

// Create a new appointment node
AppointNode* createAppointNode();

// Check if appointment ID exists
int isAppointIDExist(AppointList* list, const char* id);

// Insert appointment node
void insertAppointNode(AppointList* list, AppointNode* node);

// Find appointment by ID
AppointNode* findAppointByID(AppointList* list, const char* id);

// Delete appointment by ID
int deleteAppointByID(AppointList* list, const char* id);

// Modify appointment information
void modifyAppointInfo(AppointNode* node);

// Print single appointment
void printAppointOne(AppointNode* node);

// Print all appointments
void printAppointAll(AppointList* list);

// Statistics for appointments
void statAppoints(AppointList* list);

// Free appointment list memory
void freeAppointList(AppointList* list);

// Appointment management menu
void appointMenu(AppointList* list);

#endif

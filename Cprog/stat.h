#ifndef STAT_H
#define STAT_H

#include "utils.h"

// Statistics menu - declare prototype. The actual type definitions are provided by
// the modules that include their respective headers before including this file.
void statMenu(/* OutpatientList* */ void* opList, /* InpatientList* */ void* ipList, /* EmergencyList* */ void* emList, /* MedicineList* */ void* medList);

#endif

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "staff.h"
#include "finance.h"
#include "logistics.h"
#include "medicine.h"
#include "device.h"
#include "appoint.h"
#include "inpatient.h"
#include "outpatient.h"
#include "emergency.h"
#include "record.h"
#include "stat.h"
#include "blood.h"
#include "log.h"

#define STAFF_FILE "data/staff.txt"
#define FINANCE_FILE "data/finance.txt"
#define MATERIAL_FILE "data/material.txt"
#define USAGE_FILE "data/usage.txt"
#define MEDICINE_FILE "data/medicine.txt"
#define DEVICE_FILE "data/device.txt"
#define FIX_FILE "data/fix.txt"
#define APPOINT_FILE "data/appoint.txt"
#define INPATIENT_FILE "data/inpatient.txt"
#define OUTPATIENT_FILE "data/outpatient.txt"
#define EMERGENCY_FILE "data/emergency.txt"
#define RECORD_FILE "data/record.txt"
#define BLOOD_FILE "data/blood.txt"

// 全局数据结构
StaffList staffList;
FinanceList financeList;
MaterialList materialList;
UsageList usageList;
MedicineList medicineList;
DeviceList deviceList;
FixList fixList;
AppointList appointList;
InpatientList inpatientList;
OutpatientList outpatientList;
EmergencyList emergencyList;
RecordList recordList;
BloodList bloodList;

void saveAllData() {
    printf("\n正在保存数据...\n");
    system("if not exist data mkdir data 2>nul");

    saveStaffToFile(&staffList, STAFF_FILE);
    saveFinanceToFile(&financeList, FINANCE_FILE);
    saveMaterialsToFile(&materialList, MATERIAL_FILE);
    saveUsageToFile(&usageList, USAGE_FILE);
    saveMedicinesToFile(&medicineList, MEDICINE_FILE);
    saveDevicesToFile(&deviceList, DEVICE_FILE);
    saveFixesToFile(&fixList, FIX_FILE);
    saveAppointsToFile(&appointList, APPOINT_FILE);
    saveInpatientsToFile(&inpatientList, INPATIENT_FILE);
    saveOutpatientsToFile(&outpatientList, OUTPATIENT_FILE);
    saveEmergenciesToFile(&emergencyList, EMERGENCY_FILE);
    saveRecordsToFile(&recordList, RECORD_FILE);
    saveBloodsToFile(&bloodList, BLOOD_FILE);

    printf("数据保存完成！\n");
}

void loadAllData() {
    printf("正在加载数据...\n");

    initStaffList(&staffList);
    initFinanceList(&financeList);
    initMaterialList(&materialList);
    initUsageList(&usageList);
    initMedicineList(&medicineList);
    initDeviceList(&deviceList);
    initFixList(&fixList);
    initAppointList(&appointList);
    initInpatientList(&inpatientList);
    initOutpatientList(&outpatientList);
    initEmergencyList(&emergencyList);
    initRecordList(&recordList);
    initBloodList(&bloodList);

    loadStaffFromFile(&staffList, STAFF_FILE);
    loadFinanceFromFile(&financeList, FINANCE_FILE);
    loadMaterialsFromFile(&materialList, MATERIAL_FILE);
    loadUsageFromFile(&usageList, USAGE_FILE);
    loadMedicinesFromFile(&medicineList, MEDICINE_FILE);
    loadDevicesFromFile(&deviceList, DEVICE_FILE);
    loadFixesFromFile(&fixList, FIX_FILE);
    loadAppointsFromFile(&appointList, APPOINT_FILE);
    loadInpatientsFromFile(&inpatientList, INPATIENT_FILE);
    loadOutpatientsFromFile(&outpatientList, OUTPATIENT_FILE);
    loadEmergenciesFromFile(&emergencyList, EMERGENCY_FILE);
    loadRecordsFromFile(&recordList, RECORD_FILE);
    loadBloodsFromFile(&bloodList, BLOOD_FILE);
}

void freeAllMemory() {
    freeStaffList(&staffList);
    freeFinanceList(&financeList);
    freeMaterialList(&materialList);
    freeUsageList(&usageList);
    freeMedicineList(&medicineList);
    freeDeviceList(&deviceList);
    freeFixList(&fixList);
    freeAppointList(&appointList);
    freeInpatientList(&inpatientList);
    freeOutpatientList(&outpatientList);
    freeEmergencyList(&emergencyList);
    freeRecordList(&recordList);
    freeBloodList(&bloodList);
}

void showMainMenu() {
    printf("\n\n");
    printTitle("医院管理系统 v2.0 - 完整版");
    printf("=== 行政管理系统 ===\n");
    printf("1. 人事管理系统\n");
    printf("2. 财务管理系统\n");
    printf("3. 后勤管理系统\n");
    printf("4. 药库管理系统\n");
    printf("5. 医疗设备管理\n");
    printf("=== 医疗管理系统 ===\n");
    printf("6. 手术及住院预约\n");
    printf("7. 病人住院管理\n");
    printf("8. 门诊管理\n");
    printf("9. 急诊管理\n");
    printf("10. 病案管理\n");
    printf("11. 血库管理\n");
    printf("12. 统计查询系统\n");
    printf("=== 其他功能 ===\n");
    printf("13. 保存数据\n");
    printf("14. 关于系统\n");
    printf("15. 日志查询(管理员)\n");
    printf("0. 退出系统\n");
    printLine('-', 60);
}

void showAbout() {
    printTitle("关于系统 - 医院管理系统 v2.0");
    printf("完整版医院综合管理系统\n\n");
    printf("【行政模块】\n");
    printf("- 人事管理: 职工档案、岗位管理、工资统计\n");
    printf("- 财务管理: 收支记录、审核管理、报表统计\n");
    printf("- 后勤管理: 物资库存、领用记录、预警提示\n");
    printf("- 药库管理: 药品信息、入库出库、库存预警\n");
    printf("- 设备管理: 设备档案、维修记录、报废管理\n\n");
    printf("【医疗模块】\n");
    printf("- 预约管理: 门诊/手术/住院预约\n");
    printf("- 住院管理: 入院办理、费用结算、出院管理\n");
    printf("- 门诊管理: 门诊记录、处方管理\n");
    printf("- 急诊管理: 分级诊疗、急诊处理\n");
    printf("- 病案管理: 诊断归档、治疗摘要\n");
    printf("- 血库管理: 血液库存、出入库、有效期预警\n");
    printf("- 统计查询: 多维度统计分析\n\n");
    printf("技术实现:\n");
    printf("- 开发语言: C语言 (C99)\n");
    printf("- 数据结构: 单向链表\n");
    printf("- 数据存储: 文本文件(管道符分隔)\n");
}

int main() {
    int choice;
    int exitFlag = 0;

    printf("========================================\n");
    printf("   欢迎使用医院管理系统 v2.0 完整版\n");
    printf("========================================\n");

    loadAllData();

    do {
        showMainMenu();
        choice = inputInt("请选择功能: ");

        switch (choice) {
        case 1: staffMenu(&staffList); break;
        case 2: financeMenu(&financeList); break;
        case 3: logisticsMenu(&materialList, &usageList); break;
        case 4: medicineMenu(&medicineList); break;
        case 5: deviceMenu(&deviceList, &fixList); break;
        case 6: appointMenu(&appointList); break;
        case 7: inpatientMenu(&inpatientList); break;
        case 8: outpatientMenu(&outpatientList); break;
        case 9: emergencyMenu(&emergencyList); break;
        case 10: recordMenu(&recordList); break;
        case 11: bloodMenu(&bloodList); break;
        case 12: statMenu(&outpatientList, &inpatientList, &emergencyList, &medicineList); break;
        case 13: saveAllData(); break;
        case 14: showAbout(); break;
        case 15: logMenu(); break;
        case 0:
            if (confirm("确定要退出系统吗?")) {
                if (confirm("是否保存数据?")) {
                    saveAllData();
                }
                exitFlag = 1;
            }
            break;
        default:
            printf("无效选择，请重试！\n");
        }

        if (choice != 0 && !exitFlag) {
            pauseScreen();
        }

    } while (!exitFlag);

    freeAllMemory();

    printf("\n========================================\n");
    printf("   感谢使用医院管理系统，再见！\n");
    printf("========================================\n");
    return 0;
}

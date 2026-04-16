/**
 * @file main.c
 * brief 医院管理系统主程序 - 系统入口和主菜单控制
 *
 * 本文件是医院管理系统的主控程序，负责：
 * - 系统初始化和数据加载
 * - 主菜单显示与用户交互
 * - 各功能模块的调度
 * - 数据的统一保存和内存释放
 *
 * 系统功能模块:
 *   行政管理系统: 人事、财务、后勤、药库、设备
 *   医疗管理系统: 预约、住院、门诊、急诊、病案、血库
 *   辅助功能: 统计查询、日志管理、数据保存
 */

#include <stdio.h>
#include <stdlib.h>

/* ==================== 系统头文件包含 ==================== */

/** 工具函数库 - 提供输入输出辅助函数 */
#include "utils.h"

/** 各功能模块头文件 */
#include "staff.h"       /**< 人事管理模块 */
#include "finance.h"     /**< 财务管理模块 */
#include "logistics.h"   /**< 后勤/物资管理模块 */
#include "medicine.h"    /**< 药品/药库管理模块 */
#include "device.h"      /**< 设备管理模块 */
#include "appoint.h"     /**< 预约管理模块 */
#include "inpatient.h"   /**< 住院管理模块 */
#include "outpatient.h"  /**< 门诊管理模块 */
#include "emergency.h"   /**< 急诊管理模块 */
#include "record.h"      /**< 病案管理模块 */
#include "stat.h"        /**< 统计分析模块 */
#include "blood.h"       /**< 血库管理模块 */
#include "log.h"         /**< 日志系统模块 */

/* ==================== 数据文件路径定义 ==================== */

/**
 * 数据存储目录结构:
 * data/
 * ├── staff.txt      员工信息
 * ├── finance.txt    财务记录
 * ├── material.txt   物资库存
 * ├── usage.txt      物资领用记录
 * ├── medicine.txt   药品库存
 * ├── device.txt     设备档案
 * ├── fix.txt        设备维修记录
 * ├── appoint.txt    预约记录
 * ├── inpatient.txt  住院记录
 * ├── outpatient.txt 门诊记录
 * ├── emergency.txt  急诊记录
 * ├── record.txt     病案记录
 * └── blood.txt      血液库存
 */

#define STAFF_FILE "data/staff.txt"           /**< 员工数据文件路径 */
#define FINANCE_FILE "data/finance.txt"       /**< 财务数据文件路径 */
#define MATERIAL_FILE "data/material.txt"     /**< 物资库存文件路径 */
#define USAGE_FILE "data/usage.txt"           /**< 物资领用记录文件路径 */
#define MEDICINE_FILE "data/medicine.txt"     /**< 药品数据文件路径 */
#define DEVICE_FILE "data/device.txt"         /**< 设备数据文件路径 */
#define FIX_FILE "data/fix.txt"               /**< 维修记录文件路径 */
#define APPOINT_FILE "data/appoint.txt"       /**< 预约数据文件路径 */
#define INPATIENT_FILE "data/inpatient.txt"   /**< 住院数据文件路径 */
#define OUTPATIENT_FILE "data/outpatient.txt" /**< 门诊数据文件路径 */
#define EMERGENCY_FILE "data/emergency.txt"   /**< 急诊数据文件路径 */
#define RECORD_FILE "data/record.txt"         /**< 病案数据文件路径 */
#define BLOOD_FILE "data/blood.txt"           /**< 血库数据文件路径 */

/* ==================== 全局数据结构声明 ==================== */

/**
 * 全局链表变量说明:
 * 所有数据使用单向链表结构存储在内存中，
 * 通过这些全局变量在各函数间共享访问。
 * 每个List结构包含head指针(指向首节点)和count计数器。
 */

StaffList staffList;          /**< 员工列表 - 存储所有职工信息 */
FinanceList financeList;      /**< 财务列表 - 存储收支记录 */
MaterialList materialList;    /**< 物资列表 - 存储物资库存信息 */
UsageList usageList;          /**< 领用列表 - 存储物资领用记录 */
MedicineList medicineList;    /**< 药品列表 - 存储药品库存信息 */
DeviceList deviceList;        /**< 设备列表 - 存储医疗设备档案 */
FixList fixList;              /**< 维修列表 - 存储设备维修记录 */
AppointList appointList;      /**< 预约列表 - 存储各类预约信息 */
InpatientList inpatientList;  /**< 住院列表 - 存储住院病人信息 */
OutpatientList outpatientList;/**< 门诊列表 - 存储门诊挂号记录 */
EmergencyList emergencyList;  /**< 急诊列表 - 存储急诊处理记录 */
RecordList recordList;        /**< 病案列表 - 存储病历档案 */
BloodList bloodList;          /**< 血液列表 - 存储血液库存信息 */

/* ==================== 数据持久化函数 ==================== */

/**
 * brief 保存所有数据到文件
 *
 * 功能说明:
 *   将内存中所有链表的数据一次性写入对应的文本文件。
 *   在保存前会自动创建data目录（如果不存在）。
 *
 * 执行流程:
 *   1. 显示保存提示信息
 *   2. 创建data目录（使用system命令，忽略已存在错误）
 *   3. 依次调用各模块的saveXxxToFile()函数
 *   4. 显示保存完成提示
 *
 * 文件格式:
 *   使用管道符(|)分隔各字段，每行一条记录
 *   示例: 字段1|字段2|字段3|...\n
 *
 * 调用时机:
 *   - 用户选择手动保存时
 *   - 退出系统且选择保存数据时
 *
 * 注意事项:
 *   - 采用覆盖写入模式，原文件内容会被替换
 *   - 保存过程中如某个文件失败不影响其他文件
 */
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

/* ==================== 数据加载函数 ==================== */

/**
 * @brief 从文件加载所有数据到内存
 *
 * 功能说明:
 *   系统启动时调用，从data目录下的各个文件读取数据到内存链表。
 *   加载顺序：先初始化所有链表，再依次加载各文件数据。
 *
 * 执行流程:
 *   1. 显示加载提示信息
 *   2. 对每个全局链表调用initXxxList()进行初始化
 *   3. 对每个数据文件调用loadXxxFromFile()加载数据
 *   4. 各模块内部会显示加载结果（成功条数或错误提示）
 *
 * 容错处理:
 *   - 文件不存在时会自动创建空链表（不报错）
 *   - 格式错误的行会被跳过
 *   - 内存分配失败时跳过该记录
 *
 * 性能考虑:
 *   需要打开14个文件，对于大量数据可能需要几秒钟
 *
 * 调用时机:
 *   仅在main()函数开始时调用一次
 */
void loadAllData() {
    printf("正在加载数据...\n");

    /* 初始化所有链表结构 */
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

    /* 从文件加载数据到各链表 */
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

/* ==================== 内存释放函数 ==================== */

/**
 * @brief 释放所有全局链表占用的内存
 *
 * 功能说明:
 *   程序退出前调用，遍历并释放所有链表节点的堆内存。
 *   防止内存泄漏，确保资源正确回收。
 *
 * 执行过程:
 *   对14个全局链表分别调用freeXxxList()函数：
 *   1. 从头节点开始遍历
 *   2. 逐个free()释放节点内存
 *   3. 将head置NULL，count清零
 *
 * 重要提示:
 *   - 必须在退出main()前调用
 *   - 释放后不可再访问这些链表
 *   - 不释放list结构体本身（栈上分配）
 *
 * 内存泄漏检测:
 *   可使用工具(如Visual Leak Detector)验证是否完全释放
 */
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

/* ==================== 界面显示函数 ==================== */

/**
 * @brief 显示系统主菜单界面
 *
 * 功能说明:
 *   打印完整的系统功能菜单，供用户选择要使用的功能模块。
 *   菜单按功能类别分组显示，清晰直观。
 *
 * 菜单结构:
 *   === 行政管理系统 === (选项1-5)
 *   1. 人事管理系统    - 员工信息管理
 *   2. 财务管理系统    - 收支账目管理
 *   3. 后勤管理系统    - 物资库存管理
 *   4. 药库管理系统    - 药品进销存
 *   5. 医疗设备管理    - 设备档案维护
 *
 *   === 医疗管理系统 === (选项6-11)
 *   6. 手术及住院预约  - 多类型预约管理
 *   7. 病人住院管理    - 住院全流程
 *   8. 门诊管理        - 挂号和处方
 *   9. 急诊管理        - 急救处理
 *   10. 病案管理       - 病历归档
 *   11. 血库管理       - 血液库存管理
 *
 *   === 其他功能 === (选项12-15, 0)
 *   12. 统计查询系统   - 数据分析报表
 *   13. 保存数据       - 手动保存到文件
 *   14. 关于系统       - 版本和功能说明
 *   15. 日志查询(管理员) - 操作日志查看
 *   0. 退出系统        - 结束程序
 *
 * 输入格式: 整数(0-15)，通过键盘输入
 */
void showMainMenu() {
    printf("\n\n");
    printTitle("医院管理系统 v2.0 - 完整版");

    /* 行政管理系统分组 */
    printf("=== 行政管理系统 ===\n");
    printf("1. 人事管理系统\n");
    printf("2. 财务管理系统\n");
    printf("3. 后勤管理系统\n");
    printf("4. 药库管理系统\n");
    printf("5. 医疗设备管理\n");

    /* 医疗管理系统分组 */
    printf("=== 医疗管理系统 ===\n");
    printf("6. 手术及住院预约\n");
    printf("7. 病人住院管理\n");
    printf("8. 门诊管理\n");
    printf("9. 急诊管理\n");
    printf("10. 病案管理\n");
    printf("11. 血库管理\n");
    printf("12. 统计查询系统\n");

    /* 其他功能分组 */
    printf("=== 其他功能 ===\n");
    printf("13. 保存数据\n");
    printf("14. 关于系统\n");
    printf("15. 日志查询(管理员)\n");
    printf("0. 退出系统\n");

    printLine('-', 60);
}

/**
 * @brief 显示关于系统的详细信息
 *
 * 功能说明:
 *   展示系统的版本信息、功能介绍和技术实现细节。
 *   帮助用户了解系统能力和技术架构。
 *
 * 显示内容包括:
 *   - 系统名称和版本号
 *   - 行政模块功能概述（5个模块）
 *   - 医疗模块功能概述（7个模块）
 *   - 技术实现说明（语言、数据结构、存储方式）
 *
 * 适用场景:
 *   用户选择菜单选项14时调用
 */
void showAbout() {
    printTitle("关于系统 - 医院管理系统 v2.0");

    printf("完整版医院综合管理系统\n\n");

    /* 行政模块功能说明 */
    printf("【行政模块】\n");
    printf("- 人事管理: 职工档案、岗位管理、工资统计\n");
    printf("- 财务管理: 收支记录、审核管理、报表统计\n");
    printf("- 后勤管理: 物资库存、领用记录、预警提示\n");
    printf("- 药库管理: 药品信息、入库出库、库存预警\n");
    printf("- 设备管理: 设备档案、维修记录、报废管理\n\n");

    /* 医疗模块功能说明 */
    printf("【医疗模块】\n");
    printf("- 预约管理: 门诊/手术/住院预约\n");
    printf("- 住院管理: 入院办理、费用结算、出院管理\n");
    printf("- 门诊管理: 门诊记录、处方管理\n");
    printf("- 急诊管理: 分级诊疗、急诊处理\n");
    printf("- 病案管理: 诊断归档、治疗摘要\n");
    printf("- 血库管理: 血液库存、出入库、有效期预警\n");
    printf("- 统计查询: 多维度统计分析\n\n");

    /* 技术实现说明 */
    printf("技术实现:\n");
    printf("- 开发语言: C语言 (C99)\n");
    printf("- 数据结构: 单向链表\n");
    printf("- 数据存储: 文本文件(管道符分隔)\n");
}

/* ==================== 主函数 ==================== */

/**
 * @brief 程序入口主函数
 * @return int 返回0表示正常退出
 *
 * 功能说明:
 *   系统的主控制循环，负责：
 *   1. 显示欢迎信息
 *   2. 加载历史数据
 *   3. 循环显示主菜单并处理用户选择
 *   4. 根据选择调用对应的功能模块
 *   5. 退出时释放内存并显示告别信息
 *
 * 程序执行流程:
 *   ┌─────────────────┐
 *   │  显示欢迎信息    │
 *   └────────┬────────┘
 *            ▼
 *   ┌─────────────────┐
 *   │  加载所有数据    │ ← loadAllData()
 *   └────────┬────────┘
 *            ▼
 *   ┌─────────────────┐
 *   │  显示主菜单      │ ← showMainMenu()
 *   └────────┬────────┘
 *            ▼
 *   ┌─────────────────┐     ┌──────────────┐
 *   │  获取用户选择    │────▶│ 处理用户选择  │
 *   └────────┬────────┘     └──┬───────────┘
 *            ▼                │
 *   ┌─────────────────┐       ▼
 *   │ 选择0? 是→退出   │  ┌─────────────────┐
 *   │ 否 →继续循环     │  │ 调用对应模块函数 │
 *   └─────────────────┘  └────────┬────────┘
 *                                 ▼
 *                          ┌──────────────┐
 *                          │ 暂停等待确认  │
 *                          └──────┬───────┘
 *                                 │
 *                                 ▼
 *                           (返回显示菜单)
 *
 * 菜单选项处理:
 *   1-11: 进入对应的管理子菜单
 *   12:   进入统计分析系统
 *   13:   保存所有数据到文件
 *   14:   显示系统信息
 *   15:   进入日志查询（需管理员密码）
 *   0:    询问确认后退出系统
 *
 * 退出流程:
 *   1. 弹出确认对话框
 *   2. 询问是否保存数据
 *   3. 如选择保存则调用saveAllData()
 *   4. 设置退出标志
 *   5. 释放所有内存
 *   6. 显示告别信息
 *   7. 返回0结束程序
 *
 * 错误处理:
 *   - 无效选择给出提示并重新显示菜单
 *   - 各子菜单有独立的输入验证
 */
int main() {
    int choice;          /**< 用户选择的菜单项 */
    int exitFlag = 0;    /**< 退出标志，1表示准备退出 */

    /* 显示欢迎横幅 */
    printf("========================================\n");
    printf("   欢迎使用医院管理系统 v2.0 完整版\n");
    printf("========================================\n");

    /* 加载数据文件到内存 */
    loadAllData();

    /* 主循环：持续显示菜单直到用户选择退出 */
    do {
        showMainMenu();
        choice = inputInt("请选择功能: ");

        switch (choice) {
        /* 行政管理模块 */
        case 1: staffMenu(&staffList); break;
        case 2: financeMenu(&financeList); break;
        case 3: logisticsMenu(&materialList, &usageList); break;
        case 4: medicineMenu(&medicineList); break;
        case 5: deviceMenu(&deviceList, &fixList); break;

        /* 医疗管理模块 */
        case 6: appointMenu(&appointList); break;
        case 7: inpatientMenu(&inpatientList); break;
        case 8: outpatientMenu(&outpatientList); break;
        case 9: emergencyMenu(&emergencyList); break;
        case 10: recordMenu(&recordList); break;
        case 11: bloodMenu(&bloodList); break;
        case 12: statMenu(&outpatientList, &inpatientList, &emergencyList, &medicineList); break;

        /* 其他功能 */
        case 13: saveAllData(); break;
        case 14: showAbout(); break;
        case 15: logMenu(); break;

        /* 退出系统 */
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

        /* 非退出操作后暂停屏幕，让用户看清结果 */
        if (choice != 0 && !exitFlag) {
            pauseScreen();
        }

    } while (!exitFlag);

    /* 释放所有动态分配的内存 */
    freeAllMemory();

    /* 显示告别信息 */
    printf("\n========================================\n");
    printf("   感谢使用医院管理系统，再见！\n");
    printf("========================================\n");

    return 0;
}

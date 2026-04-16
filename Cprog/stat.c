/**
 * @file stat.c
 * @brief 医疗统计查询模块实现 - 提供医院运营数据的统计分析功能
 *
 * 本模块实现医院管理系统的综合统计功能，包括：
 * - 患者数量统计（门诊、住院、急诊）
 * - 费用收支统计（门诊费用、住院费用）
 * - 药品库存统计（种类、数量、预警）
 * - 科室业务量分析（各科室就诊量分布）
 *
 * 统计数据来源:
 *   从门诊列表(OutpatientList)、住院列表(InpatientList)，
 *   急诊列表(EmergencyList)和药品列表(MedicineList)
 *   中提取数据进行汇总计算。
 *
 * 应用场景:
 *   - 院长/管理层查看运营概况
 *   - 月度/季度经营报告生成
 *   - 科室绩效评估参考
 *   - 资源配置决策支持
 *
 * 数据特点:
 *   - 实时从内存链表读取最新数据
 *   - 支持多维度交叉分析
 *   - 输出格式化便于阅读理解
 */

#include "stat.h"
#include "outpatient.h"
#include "inpatient.h"
#include "emergency.h"
#include "medicine.h"

/**
 * @brief 医疗统计查询主菜单
 * @param opList 门诊患者链表指针（用于统计门诊数据）
 * @param ipList 住院患者链表指针（用于统计住院数据）
 * @param emList 急诊患者链表指针（用于统计急诊数据）
 * @param medList 药品库存链表指针（用于统计药品数据）
 *
 * 功能说明:
 *   提供医疗数据的综合统计分析界面，整合多个业务模块的数据，
 *   为管理者提供全面的运营概览。
 *
 * 菜单功能列表:
 *   1. 患者统计    - 统计各类患者总数及住院状态分布
 *   2. 费用统计    - 汇总门诊和住院的总费用
 *   3. 药品库存统计 - 显示药品种类、总量和预警信息
 *   4. 科室业务量统计 - 按科室分类统计门诊和住院人数
 *   0. 返回主菜单  - 退出统计功能
 *
 * 统计指标说明:
 *
 * 【患者统计】包含:
 *   - 门诊患者总人次：累计挂号就诊人数
 *   - 住院患者总人数：当前在册的住院人数（含已出院）
 *   - 急诊患者总人次：累计急诊处理人数
 *   - 患者总计：三类患者总和
 *   - 当前在院人数：状态为"住院中"的患者数
 *   - 已出院人数：状态为"已出院"的患者数
 *
 * 【费用统计】包含:
 *   - 门诊总费用：所有门诊记录的费用累加
 *   - 住院总费用：所有住院记录的总费用累加
 *   - 费用合计：门诊+住院费用总和
 *
 * 【药品库存统计】包含:
 *   - 药品种类：不同药品的数量
 *   - 药品总数量：所有药品库存量的总和
 *   - 库存预警：显示低于最低库存的药品
 *
 * 【科室业务量统计】包含:
 *   - 门诊-XX科：该科室的门诊人次
 *   - 住院-XX科：该科的住院人数
 *
 * 操作流程:
 *   1. 循环显示统计菜单
 *   2. 用户选择统计类型
 *   3. 遍历对应链表进行数据汇总
 *   4. 格式化输出统计结果
 *   5. 操作后暂停等待用户查看
 *
 * 算法特点:
 *   - 采用线性遍历算法，时间复杂度O(n)
 *   - 使用临时结构体存储分组统计结果
 *   - 支持动态部门数量的统计
 *
 * 业务价值:
 *   - 帮助管理者快速了解医院运营状况
 *   - 发现业务增长点和瓶颈环节
 *   - 为资源调配提供数据支撑
 *   - 支持绩效考核和决策制定
 */
void statMenu(OutpatientList* opList, InpatientList* ipList, EmergencyList* emList, MedicineList* medList) {
    int choice;

    do {
        printf("\n");
        printTitle("医疗统计查询系统");
        printf("1. 患者统计(门诊+住院+急诊)\n");
        printf("2. 费用统计\n");
        printf("3. 药品库存统计\n");
        printf("4. 科室业务量统计\n");
        printf("0. 返回主菜单\n");

        choice = inputInt("请选择: ");

        switch (choice) {

        /**
         * 功能1: 患者综合统计
         *
         * 统计内容:
         *   - 三类患者的分别计数和总计
         *   - 住院患者的状态细分（在院/出院）
         *
         * 实现逻辑:
         *   1. 直接读取各链表的count字段获取总数
         *   2. 遍历住院链表，按status字段分类统计
         *   3. 格式化输出统计报表
         *
         * 数据来源:
         *   opList->count: 门诊总人次
         *   ipList->count: 住院总人数
         *   emList->count: 急诊总人次
         *   ip->status: 住院状态（INPATIENT_ADMITTED/INPATIENT_DISCHARGED）
         */
        case 1:
            printf("\n--- 患者统计 ---\n");

            /* 显示三类患者的基本统计数据 */
            printf("门诊患者总数: %d 人次\n", opList->count);
            printf("住院患者总数: %d 人\n", ipList->count);
            printf("急诊患者总数: %d 人次\n", emList->count);
            printf("患者总计: %d 人次/人\n", opList->count + ipList->count + emList->count);

            /* 住院状态细分统计 */
            int admitted = 0, discharged = 0;
            InpatientNode* ip = ipList->head;

            /* 遍历住院链表，按状态分类计数 */
            while (ip) {
                if (ip->status == INPATIENT_ADMITTED) {
                    admitted++;      /* 在院治疗中 */
                }
                else if (ip->status == INPATIENT_DISCHARGED) {
                    discharged++;    /* 已办理出院 */
                }
                ip = ip->next;
            }

            printf("当前在院: %d 人 | 已出院: %d 人\n", admitted, discharged);
            break;

        /**
         * 功能2: 费用统计
         *
         * 统计内容:
         *   - 门诊费用的总额
         *   - 住院费用的总额
         *   - 两项费用的合计
         *
         * 实现逻辑:
         *   1. 遍历门诊链表，累加每条记录的cost字段
         *   2. 遍历住院链表，累加每条记录的totalCost字段
         *   3. 计算并显示各项费用及总计
         *
         * 字段说明:
         *   op->cost: 单次门诊费用
         *   in->totalCost: 住院期间总费用（含检查、药品、床位等）
         */
        case 2: {
            double totalOp = 0, totalIp = 0;

            /* 累计门诊总费用 */
            OutpatientNode* op = opList->head;
            while (op) {
                totalOp += op->cost;
                op = op->next;
            }

            /* 累计住院总费用 */
            InpatientNode* in = ipList->head;
            while (in) {
                totalIp += in->totalCost;
                in = in->next;
            }

            printf("\n--- 费用统计 ---\n");
            printf("门诊总费用: %.2f 元\n", totalOp);
            printf("住院总费用: %.2f 元\n", totalIp);
            printf("费用合计: %.2f 元\n", totalOp + totalIp);
            break;
        }

        /**
         * 功能3: 药品库存统计
         *
         * 统计内容:
         *   - 药品种类总数
         *   - 所有药品的库存量总和
         *   - 库存预警信息
         *
         * 实现逻辑:
         *   1. 遍历药品链表，同时统计种类和总量
         *   2. 调用printMedicineWarning()显示预警药品
         *
         * 注意事项:
         *   - 不同药品的计量单位可能不同
         *   - 总数量仅供参考，实际需关注单品库存
         */
        case 3: {
            int totalQty = 0;
            MedicineNode* m = medList->head;
            int count = 0;

            /* 同时统计种类数和总数量 */
            while (m) {
                totalQty += m->quantity;  /* 累加库存量 */
                m = m->next;
                count++;                  /* 计数种类 */
            }

            printf("\n--- 药品库存统计 ---\n");
            printf("药品种类: %d 种\n", count);
            printf("药品总数量: %d (单位)\n", totalQty);

            /* 显示库存预警信息 */
            printMedicineWarning(medList);
            break;
        }

        /**
         * 功能4: 科室业务量统计
         *
         * 统计内容:
         *   - 各科室的门诊就诊人次
         *   - 各科室的住院人数
         *
         * 实现逻辑:
         *   1. 定义内部结构体DeptCnt存储科室名和计数
         *   2. 遍历门诊链表，按department字段分组统计
         *   3. 遍历住院链表，按department字段分组统计
         *   4. 分别输出两个维度的统计结果
         *
         * 分组算法:
         *   - 使用数组存储已发现的科室
         *   - 新科室首次出现时创建条目
         *   - 已存在的科室增加计数
         *
         * 数据结构限制:
         *   - 最多支持50个不同科室
         *   - 超过限制时新科室不计入统计
         *
         * 输出格式示例:
         *   门诊-内科: 150 人次
         *   门诊-外科: 120 人次
         *   住院-内科: 45 人
         *   住院-外科: 38 人
         */
        case 4: {
            printf("\n--- 科室业务量 ---\n");

            /* ====== 门诊按科室统计 ====== */

            /* 定义科室计数结构体 */
            typedef struct {
                char dept[50];  /* 科室名称 */
                int cnt;        /* 就诊次数 */
            } DeptCnt;

            DeptCnt dcs[50];   /* 门诊科室统计数组 */
            int dc = 0;        /* 已发现科室数 */

            OutpatientNode* op = opList->head;

            /* 遍历门诊链表进行分组统计 */
            while (op) {
                int f = 0;

                /* 查找是否已有该科室的统计项 */
                for (int i = 0; i < dc; i++) {
                    if (strcmp(dcs[i].dept, op->department) == 0) {
                        dcs[i].cnt++;   /* 已存在则累加 */
                        f = 1;
                        break;
                    }
                }

                /* 不存在则新建统计项 */
                if (!f && dc < 50) {
                    strcpy(dcs[dc].dept, op->department);
                    dcs[dc].cnt = 1;
                    dc++;
                }
                op = op->next;
            }

            /* 输出门诊科室统计结果 */
            for (int i = 0; i < dc; i++) {
                printf("门诊-%s: %d 人次\n", dcs[i].dept, dcs[i].cnt);
            }

            /* ====== 住院按科室统计 ====== */

            DeptCnt dcs2[50];  /* 住院科室统计数组 */
            int dc2 = 0;       /* 已发现科室数 */

            InpatientNode* ip2 = ipList->head;

            /* 遍历住院链表进行分组统计 */
            while (ip2) {
                int f = 0;

                for (int i = 0; i < dc2; i++) {
                    if (strcmp(dcs2[i].dept, ip2->department) == 0) {
                        dcs2[i].cnt++;
                        f = 1;
                        break;
                    }
                }

                if (!f && dc2 < 50) {
                    strcpy(dcs2[dc2].dept, ip2->department);
                    dcs2[dc2].cnt = 1;
                    dc2++;
                }
                ip2 = ip2->next;
            }

            /* 输出住院科室统计结果 */
            for (int i = 0; i < dc2; i++) {
                printf("住院-%s: %d 人\n", dcs2[i].dept, dcs2[i].cnt);
            }
            break;
        }

        case 0:
            printf("返回主菜单...\n");
            break;

        default:
            printf("无效选择。\n");
        }

        /* 除退出外每次操作后暂停，让用户查看结果 */
        if (choice != 0) {
            pauseScreen();
        }
    } while (choice != 0);
}

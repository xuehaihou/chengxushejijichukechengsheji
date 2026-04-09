#include "stat.h"
#include "outpatient.h"
#include "inpatient.h"
#include "emergency.h"
#include "medicine.h"

void statMenu(OutpatientList* opList, InpatientList* ipList, EmergencyList* emList, MedicineList* medList) {
    int choice;
    do {
        printf("\n"); printTitle("医疗统计查询系统");
        printf("1. 患者统计(门诊+住院+急诊)\n");
        printf("2. 费用统计\n");
        printf("3. 药品库存统计\n");
        printf("4. 科室业务量统计\n");
        printf("0. 返回主菜单\n");

        choice = inputInt("请选择: ");

        switch (choice) {
        case 1:
            printf("\n--- 患者统计 ---\n");
            printf("门诊患者总数: %d 人次\n", opList->count);
            printf("住院患者总数: %d 人\n", ipList->count);
            printf("急诊患者总数: %d 人次\n", emList->count);
            printf("患者总计: %d 人次/人\n", opList->count + ipList->count + emList->count);

            // 住院状态细分
            int admitted = 0, discharged = 0;
            InpatientNode* ip = ipList->head;
            while (ip) {
                if (ip->status == INPATIENT_ADMITTED) admitted++;
                else if (ip->status == INPATIENT_DISCHARGED) discharged++;
                ip = ip->next;
            }
            printf("当前在院: %d 人 | 已出院: %d 人\n", admitted, discharged);
            break;

        case 2: {
            double totalOp = 0, totalIp = 0;
            OutpatientNode* op = opList->head;
            while (op) { totalOp += op->cost; op = op->next; }
            InpatientNode* in = ipList->head;
            while (in) { totalIp += in->totalCost; in = in->next; }
            printf("\n--- 费用统计 ---\n");
            printf("门诊总费用: %.2f 元\n", totalOp);
            printf("住院总费用: %.2f 元\n", totalIp);
            printf("费用合计: %.2f 元\n", totalOp + totalIp);
            break;
        }

        case 3: {
            int totalQty = 0;
            MedicineNode* m = medList->head;
            int count = 0;
            while (m) { totalQty += m->quantity; m = m->next; count++; }
            printf("\n--- 药品库存统计 ---\n");
            printf("药品种类: %d 种\n", count);
            printf("药品总数量: %d (单位)\n", totalQty);
            printMedicineWarning(medList);
            break;
        }

        case 4: {
            printf("\n--- 科室业务量 ---\n");
            // 门诊按科室
            typedef struct { char dept[50]; int cnt; } DeptCnt;
            DeptCnt dcs[50]; int dc = 0;
            OutpatientNode* op = opList->head;
            while (op) {
                int f = 0;
                for (int i = 0; i < dc; i++) {
                    if (strcmp(dcs[i].dept, op->department) == 0) { dcs[i].cnt++; f = 1; break; }
                }
                if (!f && dc < 50) { strcpy(dcs[dc].dept, op->department); dcs[dc].cnt = 1; dc++; }
                op = op->next;
            }
            for (int i = 0; i < dc; i++) {
                printf("门诊-%s: %d 人次\n", dcs[i].dept, dcs[i].cnt);
            }
            // 住院按科室
            DeptCnt dcs2[50]; int dc2 = 0;
            InpatientNode* ip2 = ipList->head;
            while (ip2) {
                int f = 0;
                for (int i = 0; i < dc2; i++) {
                    if (strcmp(dcs2[i].dept, ip2->department) == 0) { dcs2[i].cnt++; f = 1; break; }
                }
                if (!f && dc2 < 50) { strcpy(dcs2[dc2].dept, ip2->department); dcs2[dc2].cnt = 1; dc2++; }
                ip2 = ip2->next;
            }
            for (int i = 0; i < dc2; i++) {
                printf("住院-%s: %d 人\n", dcs2[i].dept, dcs2[i].cnt);
            }
            break;
        }
        case 0: printf("返回主菜单...\n"); break;
        default: printf("无效选择。\n");
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

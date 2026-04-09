#include "utils.h"

// 清除输入缓冲区
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 安全输入字符串
void safeInput(char* dest, int maxLen, const char* prompt) {
    printf("%s", prompt);
    if (fgets(dest, maxLen, stdin) != NULL) {
        size_t len = strlen(dest);
        if (len > 0 && dest[len - 1] == '\n') {
            dest[len - 1] = '\0';
        }
        else {
            clearInputBuffer();
        }
        trim(dest);
    }
}

// 输入整数
int inputInt(const char* prompt) {
    int value;
    char buffer[100];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &value) == 1) {
                return value;
            }
        }
        printf("输入无效，请重新输入整数！\n");
    }
}

// 输入浮点数
double inputDouble(const char* prompt) {
    double value;
    char buffer[100];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%lf", &value) == 1) {
                return value;
            }
        }
        printf("输入无效，请重新输入数字！\n");
    }
}

// 检查日期格式是否合法 (YYYY-MM-DD)
int isValidDate(const char* date) {
    if (strlen(date) != 10) return 0;
    if (date[4] != '-' || date[7] != '-') return 0;

    int year, month, day;
    if (sscanf(date, "%d-%d-%d", &year, &month, &day) != 3) return 0;

    if (year < 1900 || year > 2100) return 0;
    if (month < 1 || month > 12) return 0;
    if (day < 1 || day > 31) return 0;

    // 检查各月份天数
    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    // 闰年判断
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        daysInMonth[1] = 29;
    }
    if (day > daysInMonth[month - 1]) return 0;

    return 1;
}

// 比较两个日期，返回: -1(date1<date2), 0(相等), 1(date1>date2)
int compareDate(const char* date1, const char* date2) {
    int y1, m1, d1, y2, m2, d2;
    sscanf(date1, "%d-%d-%d", &y1, &m1, &d1);
    sscanf(date2, "%d-%d-%d", &y2, &m2, &d2);

    if (y1 != y2) return (y1 > y2) ? 1 : -1;
    if (m1 != m2) return (m1 > m2) ? 1 : -1;
    if (d1 != d2) return (d1 > d2) ? 1 : -1;
    return 0;
}

// 计算两个日期之间的天数差
int daysBetween(const char* date1, const char* date2) {
    struct tm tm1 = { 0 }, tm2 = { 0 };
    int y1, m1, d1, y2, m2, d2;

    sscanf(date1, "%d-%d-%d", &y1, &m1, &d1);
    sscanf(date2, "%d-%d-%d", &y2, &m2, &d2);

    tm1.tm_year = y1 - 1900;
    tm1.tm_mon = m1 - 1;
    tm1.tm_mday = d1;

    tm2.tm_year = y2 - 1900;
    tm2.tm_mon = m2 - 1;
    tm2.tm_mday = d2;

    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);

    double diff = difftime(t1, t2) / (60 * 60 * 24);
    return (int)diff;
}

// 获取当前日期
void getCurrentDate(char* date) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    sprintf(date, "%04d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
}

// 检查电话号码格式
int isValidPhone(const char* phone) {
    int len = strlen(phone);
    if (len < 7 || len > 15) return 0;

    for (int i = 0; i < len; i++) {
        if (!isdigit(phone[i]) && phone[i] != '-' && phone[i] != ' ') {
            return 0;
        }
    }
    return 1;
}

// 检查字符串是否为空
int isEmpty(const char* str) {
    if (str == NULL || strlen(str) == 0) return 1;

    // 检查是否全是空格
    for (int i = 0; str[i]; i++) {
        if (!isspace((unsigned char)str[i])) return 0;
    }
    return 1;
}

// 去除字符串首尾空格
void trim(char* str) {
    if (str == NULL) return;

    // 去除前导空格
    char* start = str;
    while (isspace((unsigned char)*start)) start++;

    if (*start == '\0') {
        str[0] = '\0';
        return;
    }

    // 去除尾部空格
    char* end = str + strlen(str) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;

    // 移动字符串
    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

// 字符串转大写
void toUpperCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

// 生成唯一ID (基于时间戳)
void generateID(char* id, const char* prefix) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    static int counter = 0;
    counter++;

    sprintf(id, "%s%04d%02d%02d%04d",
        prefix,
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        counter);
}

// 确认操作
int confirm(const char* message) {
    char choice[10];
    printf("%s (Y/N): ", message);
    safeInput(choice, sizeof(choice), "");

    if (strlen(choice) == 0) {
        printf("请输入 Y 或 N\n");
        return 0;
    }

    return (choice[0] == 'Y' || choice[0] == 'y');
}

// 暂停等待用户按键
void pauseScreen() {
    printf("\n按回车键继续...");
    clearInputBuffer();
}

// 打印分隔线
void printLine(char c, int len) {
    for (int i = 0; i < len; i++) {
        putchar(c);
    }
    putchar('\n');
}

// 打印标题
void printTitle(const char* title) {
    int len = strlen(title);
    int padding = (60 - len) / 2;

    printLine('=', 60);
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s\n", title);
    printLine('=', 60);
}

// 检查金额是否合法
int isValidAmount(double amount) {
    return amount >= 0;
}

// 检查编号是否重复（通用）
int isDuplicateID(void* head, const char* id, int idOffset) {
    // 这是一个通用函数，通过偏移量访问id字段
    // 使用时需要传入链表头指针和id字段在结构体中的偏移量
    // 由于C语言限制，这里提供一个简化版本
    return 0; // 具体实现需要在各模块中单独处理
}
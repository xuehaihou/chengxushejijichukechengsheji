#ifndef UTILS_H
#define UTILS_H

/* Disable MSVC warnings about unsafe CRT functions like strcpy/sscanf/sprintf */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_ID_LEN 20
#define MAX_NAME_LEN 50
#define MAX_PHONE_LEN 20
#define MAX_DATE_LEN 20
#define MAX_REMARK_LEN 200
#define MAX_DEPT_LEN 50
#define MAX_POSITION_LEN 50
#define MAX_ADDRESS_LEN 100

// 清除输入缓冲区
void clearInputBuffer();

// 安全输入字符串
void safeInput(char* dest, int maxLen, const char* prompt);

// 输入整数
int inputInt(const char* prompt);

// 输入浮点数
double inputDouble(const char* prompt);

// 检查日期格式是否合法 (YYYY-MM-DD)
int isValidDate(const char* date);

// 比较两个日期，返回: -1(date1<date2), 0(相等), 1(date1>date2)
int compareDate(const char* date1, const char* date2);

// 计算两个日期之间的天数差
int daysBetween(const char* date1, const char* date2);

// 获取当前日期
void getCurrentDate(char* date);

// 检查电话号码格式
int isValidPhone(const char* phone);

// 检查字符串是否为空
int isEmpty(const char* str);

// 去除字符串首尾空格
void trim(char* str);

// 字符串转大写
void toUpperCase(char* str);

// 生成唯一ID (基于时间戳)
void generateID(char* id, const char* prefix);

// 确认操作
int confirm(const char* message);

// 暂停等待用户按键
void pauseScreen();

// 打印分隔线
void printLine(char c, int len);

// 打印标题
void printTitle(const char* title);

// 检查金额是否合法
int isValidAmount(double amount);

// 检查编号是否重复（通用）
int isDuplicateID(void* head, const char* id, int idOffset);

#endif
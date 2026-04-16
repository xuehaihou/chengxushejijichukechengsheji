/**
 * @file utils.h
 * @brief 通用工具函数库头文件 - 定义基础工具函数的接口和常量
 *
 * 本模块是医院管理系统的底层基础支撑库，
 *   提供所有业务模块共用的工具函数和常量定义。
 *
 * 核心功能分类:
 *
 * 【输入处理】
 *   - 安全的字符串输入（防溢出）
 *   - 整数/浮点数输入（带验证）
 *   - 输入缓冲区管理
 *
 * 【数据验证】
 *   - 日期格式验证（含闰年判断）
 *   - 电话号码格式检查
 *   - 金额合法性验证
 *   - 空字符串检测
 *
 * 【日期时间】
 *   - 当前日期获取
 *   - 日期比较
 *   - 日期差计算
 *
 * 【字符串操作】
 *   - 首尾空格去除
 *   - 大小写转换
 *
 * 【用户交互】
 *   - 操作确认对话框
 *   - 屏幕暂停等待
 *   - 格式化输出（分隔线、标题）
 *
 * 【ID生成】
 *   - 基于时间戳的唯一ID生成
 *   - ID重复性检查框架
 *
 * 设计原则:
 *   1. 安全性: 所有输入函数都有缓冲区保护
 *   2. 健壮性: 完善的错误提示和循环验证
 *   3. 一致性: 统一的命名规范和返回值约定
 *   4. 可移植性: 使用标准C库，跨平台兼容
 *
 * 使用方式:
 *   本头文件被所有业务模块包含，
 *   提供统一的基础功能支持。
 *
 *   #include "utils.h"
 *   // 即可使用所有声明的函数和常量
 *
 * 依赖关系:
 *   - 标准C库: stdio, stdlib, string, time, ctype
 *   - 无其他自定义模块依赖（最底层模块）
 *
 * 重要常量定义:
 *   定义了系统中使用的各种字段最大长度，
 *   所有业务模块都应使用这些常量以确保一致性。
 */

#ifndef UTILS_H
#define UTILS_H

/**
 * @brief 禁用MSVC不安全CRT函数警告
 *
 * 说明:
 *   Visual Studio编译器会对strcpy、sprintf等标准C函数
 *   发出安全警告(C4996)。通过定义此宏禁用这些警告，
 *   以便使用标准C函数而不产生警告。
 *
 * 影响:
 *   仅影响MSVC编译器，对GCC/Clang无影响
 */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

/* ==================== 系统头文件引用 ==================== */

#include <stdio.h>      /**< 标准输入输出 - printf/fprintf/fgets/sscanf等 */
#include <stdlib.h>     /**< 标准库 - malloc/free/exit等 */
#include <string.h>     /**< 字符串处理 - strcpy/strlen/strcmp/memmove等 */
#include <time.h>       /**< 时间处理 - time/localtime/mktime/difftime等 */
#include <ctype.h>      /**< 字符处理 - isdigit/isspace/toupper等 */

/* ==================== 全局常量定义 ==================== */

/**
 * @defgroup FieldLengthConstants 字段最大长度常量
 * @brief 定义系统中各数据字段的最大长度限制
 *
 * 说明:
 *   这些常量用于统一各模块的字段长度定义，
 *   确保字符数组分配的一致性和安全性。
 *   所有业务模块都应使用这些常量而非硬编码数值。
 * @{
 */

#define MAX_ID_LEN 20           /**< ID编号最大长度(20字符)
                                  * 用于：病案编号、患者编号、员工工号、药品编号等唯一标识符 */

#define MAX_NAME_LEN 50         /**< 姓名最大长度(50字符)
                                  * 用于：患者姓名、医生姓名、药品名称、员工姓名等人名 */

#define MAX_PHONE_LEN 20        /**< 电话号码最大长度(20字符)
                                  * 用于：手机号、座机号、紧急联系电话 */

#define MAX_DATE_LEN 20         /**< 日期字符串最大长度(20字符)
                                  * 格式为YYYY-MM-DD(10字符)，预留空间给其他格式 */

#define MAX_REMARK_LEN 200      /**< 备注信息最大长度(200字符)
                                  * 用于：备注说明、诊断结果、摘要信息等文本描述 */

#define MAX_DEPT_LEN 50         /**< 部门名称最大长度(50字符)
                                  * 用于：科室名称、部门名称 */

#define MAX_POSITION_LEN 50     /**< 岗位名称最大长度(50字符)
                                  * 用于：职位名称、岗位描述 */

#define MAX_ADDRESS_LEN 100     /**< 地址信息最大长度(100字符)
                                  * 用于：联系地址、住址等信息 */

/** @} */ /* 结束FieldLengthConstants组 */

/* ==================== 输入处理函数声明 ==================== */

/**
 * @brief 清除输入缓冲区的残留数据
 *
 * 清除stdin中的剩余字符，防止影响后续输入。
 * 通常在scanf或fgets后调用。
 */
void clearInputBuffer(void);

/**
 * @brief 安全的字符串输入函数
 * @param dest 目标字符数组指针
 * @param maxLen 目标数组最大容量
 * @param prompt 输入提示信息
 *
 * 使用fgets安全读取，自动处理换行符和首尾空格。
 * 具有缓冲区溢出保护功能。
 */
void safeInput(char* dest, int maxLen, const char* prompt);

/**
 * @brief 安全的整数输入函数
 * @param prompt 输入提示信息
 * @return 用户输入的整数值
 *
 * 循环读取直到获得有效整数，具有错误恢复能力。
 */
int inputInt(const char* prompt);

/**
 * @brief 安全的浮点数输入函数
 * @param prompt 输入提示信息
 * @return 用户输入的浮点数值(double类型)
 *
 * 循环读取直到获得有效数字，支持整数和小数形式。
 */
double inputDouble(const char* prompt);

/* ==================== 数据验证函数声明 ==================== */

/**
 * @brief 验证日期格式是否合法
 * @param date 待验证的日期字符串
 * @return 合法返回1，非法返回0
 *
 * 检查格式是否为YYYY-MM-DD，并验证日期的实际有效性。
 * 包含闰年2月份天数检查。
 */
int isValidDate(const char* date);

/**
 * @brief 比较两个日期的先后顺序
 * @param date1 第一个日期(YYYY-MM-DD)
 * @param date2 第二个日期(YYYY-MM-DD)
 * @return date1<date2返回-1，相等返回0，date1>date2返回1
 *
 * 采用字典序比较法，先比年再比月最后比日。
 */
int compareDate(const char* date1, const char* date2);

/**
 * @brief 计算两个日期之间的天数差
 * @param date1 起始日期(YYYY-MM-DD)
 * @param date2 结束日期(YYYY-MM-DD)
 * @return 天数差值(date1-date2)，可为正负数或零
 *
 * 利用C标准库time函数计算精确的天数差。
 */
int daysBetween(const char* date1, const char* date2);

/**
 * @brief 获取当前系统日期
 * @param date 用于存储结果的字符数组(至少11字节)
 *
 * 获取本地当前日期，格式化为"YYYY-MM-DD"字符串。
 */
void getCurrentDate(char* date);

/**
 * @brief 验证电话号码格式是否合法
 * @param phone 待验证的电话号码字符串
 * @return 合法返回1，非法返回0
 *
 * 检查长度(7-15)和字符(只允许数字、连字符、空格)。
 */
int isValidPhone(const char* phone);

/**
 * @brief 检查字符串是否为空或纯空白
 * @param str 待检查的字符串
 * @return 为空(NULL/空串/纯空白)返回1，否则返回0
 *
 * 判断是否包含任何非空白字符。
 */
int isEmpty(const char* str);

/* ==================== 字符串处理函数声明 ==================== */

/**
 * @brief 去除字符串首尾的空白字符
 * @param str 要处理的字符串(原地修改)
 *
 * 移除开头和结尾的所有空白字符(空格、制表符、换行等)。
 * 使用memmove进行内存移动，可处理重叠区域。
 */
void trim(char* str);

/**
 * @brief 将字符串转换为大写字母
 * @param str 要转换的字符串(原地修改)
 *
 * 将所有小写字母(a-z)转换为大写(A-Z)，其他字符不变。
 */
void toUpperCase(char* str);

/* ==================== 工具函数声明 ==================== */

/**
 * @brief 生成唯一标识符(ID)
 * @param id 用于存储生成的ID的字符数组(至少25字节)
 * @param prefix ID前缀字符串(如"MED"、"EMP")
 *
 * 基于当前日期和时间戳生成唯一ID。
 * 格式：[prefix][YYYY][MM][DD][NNNN]
 */
void generateID(char* id, const char* prefix);

/* ==================== 用户交互函数声明 ==================== */

/**
 * @brief 要求用户确认操作
 * @param message 确认提示信息
 * @return 用户输入Y/y返回1，否则返回0
 *
 * 显示"(Y/N)"提示并等待用户选择。
 * 用于危险操作的二次确认。
 */
int confirm(const char* message);

/**
 * @brief 暂停屏幕等待用户按键继续
 *
 * 显示"按回车键继续..."并阻塞等待用户按回车。
 * 用于操作完成后让用户查看结果。
 */
void pauseScreen(void);

/* ==================== 格式化输出函数声明 ==================== */

/**
 * @brief 打印指定长度的分隔线
 * @param c 分隔线使用的字符
 * @param len 分隔线的长度(字符个数)
 *
 * 输出由指定字符组成的水平线，用于界面美化。
 */
void printLine(char c, int len);

/**
 * @brief 打印居中显示的标题栏
 * @param title 标题文字
 *
 * 输出带有上下边框的居中标题，总宽度60字符。
 * 使界面更加专业美观。
 */
void printTitle(const char* title);

/* ==================== 业务验证函数声明 ==================== */

/**
 * @brief 检查金额数值是否合法
 * @param amount 待检查的金额数值
 * @return 合法(≥0)返回1，非法(<0)返回0
 *
 * 目前仅检查非负性，未来可扩展更多规则。
 */
int isValidAmount(double amount);

/**
 * @brief 通用ID重复性检查（框架函数）
 * @param head 链表头指针(void*)
 * @param id 要检查的ID字符串
 * @param idOffset ID字段的字节偏移量
 * @return 存在返回1，不存在返回0
 *
 * 占位实现，实际应在各模块中单独实现具体版本。
 */
int isDuplicateID(void* head, const char* id, int idOffset);

#endif

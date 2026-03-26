/**
 * @brief 系统相关api
 * @date 2022/11/28
 * @author laoyao
 */

#pragma once

#include <string>

struct MEMINFO {
    uint64_t sz_total;
    uint64_t sz_free;
    uint64_t sz_available;
};

typedef struct CPUPACKED {
    char name[20];       // 定义一个char类型的数组名name有20个元素
    unsigned int user;   // 定义一个无符号的int类型的user
    unsigned int nice;   // 定义一个无符号的int类型的nice
    unsigned int system; // 定义一个无符号的int类型的system
    unsigned int idle;   // 定义一个无符号的int类型的idle
    unsigned int lowait;
    unsigned int irq;
    unsigned int softirq;
} CPU_OCCUPY;

extern CPU_OCCUPY last_cpu_occupy;
extern struct MEMINFO meminfo;

extern float getCpuUseInfo(CPU_OCCUPY &last_cpu);
extern float getGpuUseInfo();
extern float getTemperatureInfo();
extern float getMemInfo(struct MEMINFO &mi);
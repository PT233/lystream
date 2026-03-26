/**
 * @brief 系统相关api
 * @date 2022/11/28
 * @author laoyao
 */

#include "system.h"

#include <cstdio>

#define MAX_PATH 256

CPU_OCCUPY last_cpu_occupy;
struct MEMINFO meminfo;

float getMemInfo(struct MEMINFO &mi)
{
    char buf[MAX_PATH];

    FILE *fp = fopen("/proc/meminfo", "rt");
    if (!fp) {
        printf("open file %s error'n", "/proc/meminfo");
        return 0.0;
    }

    fscanf(fp, "%s %lu kB\n", buf, &mi.sz_total);
    fscanf(fp, "%s %lu kB\n", buf, &mi.sz_free);
    fscanf(fp, "%s %lu kB\n", buf, &mi.sz_available);

    fclose(fp);

    float available = (float)mi.sz_available * 1.0f / (float)mi.sz_total;
    return (1.0f - available) * 100.0f;
}

float getTemperatureInfo()
{
    unsigned int temp;
    FILE *fp = fopen("/sys/devices/virtual/thermal/thermal_zone0/temp", "rt");
    fscanf(fp, "%d", &temp);
    fclose(fp);
    return (float)temp / 1000.0f;
}

static double getCpuUse(CPU_OCCUPY *o, CPU_OCCUPY *n)
{
    unsigned long od, nd;
    od          = (o->user + o->nice + o->system + o->idle + o->lowait + o->irq
          + o->softirq); // 第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd          = (n->user + n->nice + n->system + n->idle + n->lowait + n->irq
          + n->softirq); // 第二次(用户+优先级+系统+空闲)的时间再赋给od
    auto sum    = static_cast<double>(nd - od);
    double idle = n->idle - o->idle;
    return (sum - idle) / sum;
}

float getCpuUseInfo(CPU_OCCUPY &last_cpu)
{
    CPU_OCCUPY cpu_occupy;

    FILE *fp = fopen("/proc/stat", "rt");
    if (!fp) { return false; }

    char buff[256];
    fgets(buff, sizeof(buff), fp);

    sscanf(buff, "%s %u %u %u %u %u %u %u", cpu_occupy.name, &cpu_occupy.user, &cpu_occupy.nice, &cpu_occupy.system,
           &cpu_occupy.idle, &cpu_occupy.lowait, &cpu_occupy.irq, &cpu_occupy.softirq);
    fclose(fp);

    auto cpu_use = (float)getCpuUse(&last_cpu, &cpu_occupy);

    last_cpu = cpu_occupy;

    return cpu_use * 100.0f;
}

float getGpuUseInfo()
{
    unsigned int gpu_use;
    FILE *fp = fopen("/sys/devices/gpu.0/load", "rt");
    fscanf(fp, "%d", &gpu_use);
    fclose(fp);
    return (float)gpu_use / 10.0f;
}
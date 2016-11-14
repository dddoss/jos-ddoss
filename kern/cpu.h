
#ifndef JOS_INC_CPU_H
#define JOS_INC_CPU_H

#include <inc/types.h>
#include <inc/memlayout.h>
#include <inc/mmu.h>
#include <inc/env.h>
#include <inc/assert.h>

// Maximum number of CPUs
#define NCPU  8

// Values of status in struct Cpu
enum {
	CPU_UNUSED = 0,
	CPU_STARTED,
	CPU_HALTED,
};

// Per-CPU state
struct CpuInfo {
	uint8_t cpu_id;                 // Local APIC ID; index into cpus[] below
	volatile unsigned cpu_status;   // The status of the CPU
	struct Env *cpu_env;            // The currently-running environment.
	struct Taskstate cpu_ts;        // Used by x86 to find stack for interrupt
};

// Initialized in mpconfig.c
extern struct CpuInfo cpus[NCPU];
extern int ncpu;                    // Total number of CPUs in the system
extern struct CpuInfo *bootcpu;     // The boot-strap processor (BSP)
extern physaddr_t lapicaddr;        // Physical MMIO address of the local APIC

// Per-CPU kernel stacks
extern unsigned char percpu_kstacks[NCPU][KSTKSIZE];

int cpunum(void);
#define thiscpu (&cpus[cpunum()])

void mp_init(void);
void lapic_init(void);
void lapic_startap(uint8_t apicid, uint32_t addr);
void lapic_eoi(void);
void lapic_ipi(int vector);

/* This macro takes a cpu id number (an index between [0, NCPU)) and returns
 *   the virtual address corresponding to that CPU's kernel stack top. It panics
 *   if you pass it an invalid cpu_id.
 *   TODO: Should this go in memlayout.h? How to deal with circular #include
 *   to reference NCPU?
 */    
#define KSTACKTOPI(cpu_id) _kstacktopi(__FILE__, __LINE__, cpu_id)

static inline uintptr_t
_kstacktopi(const char *file, int line, int cpu_id)
{
        if (cpu_id < 0 || cpu_id > NCPU)
                    _panic(file, line, "KSTACKI called with invalid cpuid %d", cpu_id);

            return (uintptr_t)(KSTACKTOP - cpu_id * (KSTKSIZE + KSTKGAP));
}


#endif

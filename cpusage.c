#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/sysinfo.h>

typedef enum
{
    TCK_USER = 0,
    TCK_NICE,
    TCK_SYSTEM,
    TCK_IDLE,
    TCK_IOWAIT,
    TCK_IRQ,
    TCK_SOFTIRQ,
    TCK_STEAL,
    TCK_GUEST,
    TCK_GUEST_NICE,
    NUM_TCK_TYPES
} cpu_tck_type;

typedef struct
{
    char name[16];
    uint64_t tcks[NUM_TCK_TYPES];
} cpu_tck_t;

uint64_t idle_ticks(cpu_tck_t *stat)
{
    return stat->tcks[TCK_IDLE] + stat->tcks[TCK_IOWAIT];
}

uint64_t total_ticks(cpu_tck_t *stat)
{
    uint64_t total = 0;
    for(int i = 0; i < NUM_TCK_TYPES; i++)
        total += stat->tcks[i];
    return total;
}

void cpusage(cpu_tck_t *prev, cpu_tck_t *curr)
{
    int nprocs = get_nprocs();
    for(int i = 1; i <= nprocs; i++){
        uint64_t total = total_ticks(curr+i) - total_ticks(prev+i);
        uint64_t idle = idle_ticks(curr+i) - idle_ticks(prev+i);
        uint64_t active = total - idle;
        printf("%s - load %.1f%% \n", curr[i].name, active*100.f/total );
    }
    printf("\n");
}

void read_cpustat(cpu_tck_t *cpu_stat)
{
    FILE *stat_fp = fopen("/proc/stat", "r");

    int nprocs = get_nprocs();
    for(int i = 0; i <= nprocs; i++){
        fscanf(
            stat_fp,
            "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
            cpu_stat[i].name,
            &(cpu_stat[i].tcks[TCK_USER]),
            &(cpu_stat[i].tcks[TCK_NICE]),
            &(cpu_stat[i].tcks[TCK_SYSTEM]),
            &(cpu_stat[i].tcks[TCK_IDLE]),
            &(cpu_stat[i].tcks[TCK_IOWAIT]),
            &(cpu_stat[i].tcks[TCK_IRQ]),
            &(cpu_stat[i].tcks[TCK_SOFTIRQ]),
            &(cpu_stat[i].tcks[TCK_STEAL]),
            &(cpu_stat[i].tcks[TCK_GUEST]),
            &(cpu_stat[i].tcks[TCK_GUEST_NICE])
        );
    }

    fclose(stat_fp);
}

int main(int ac, char **av)
{
    int opt;
    int sample_times = 5;
    int sample_ms = 1000;
    while((opt = getopt(ac, av, "s:t:")) != -1){
        switch(opt){
            case 's':
                sample_times = atoi(optarg);
                break;
            case 't':
                sample_ms = atoi(optarg);
                break;
        }
    }

    uint16_t nprocs = get_nprocs_conf();
    cpu_tck_t *cpu_stat[2];
    // allocate 1 more for the overall 'cpu' item
    cpu_stat[0] = malloc(sizeof(cpu_tck_t)*(nprocs + 1));
    cpu_stat[1] = malloc(sizeof(cpu_tck_t)*(nprocs + 1));

    int curr = 0;
    read_cpustat(cpu_stat[curr]);
    curr ^= 1;
    while(sample_times--){
        usleep(sample_ms*1000);
        read_cpustat(cpu_stat[curr]);
        cpusage(cpu_stat[curr^1], cpu_stat[curr]);
        curr ^= 1;
    }

    free(cpu_stat[0]);
    free(cpu_stat[1]);
}

#include "asm/io.h"
#include "linux/clock.h"

#define PIT_CHAN0_REG 0X40
#define PIT_CHAN2_REG 0X42
#define PIT_CTRL_REG 0X43

#define HZ 100
#define OSCILLATOR 1193182 /* 晶振频率 */
#define CLOCK_COUNTER (OSCILLATOR / HZ) /* 定时器一秒计数OSCILLATOR次，那么10ms则计数 OSCILLATOR/100 次，并产生一次定时器中断 */

// 10ms触发一次中断
#define JIFFY (1000 / HZ)

int jiffy = JIFFY;
int cpu_tickes = 0;

void clock_init() {
    outByte(PIT_CTRL_REG, 0b00110100);
    outByte(PIT_CHAN0_REG, CLOCK_COUNTER & 0xff);
    outByte(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff);
}

void clock_hander()
{
    cpu_tickes++;
    // printk("cpu_tickes %d.\n", cpu_tickes);
}
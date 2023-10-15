SECTIONS
{
    . = 0x1200;      /* 设置起始地址 */

    .text : {         
        build/init/head.o(.text)   /* 首先放置start.o中的.text段 */
        *(.text)         /* 然后放置其他所有的.text段 */
    }

    .data : {         
        *(.data)
    }

    .bss : {          
        *(.bss)
    }
}

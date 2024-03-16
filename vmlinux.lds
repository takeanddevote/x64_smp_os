SECTIONS
{
    . = 0x1200;      /* 设置起始地址 */
    .head.text : {
        *(.head.text)
    }

    .text : {         
        *(.text)         /* 然后放置其他所有的.text段 */
    }

    .data : {         
        *(.data)
    }

    .bss : {          
        *(.bss)
    }
}

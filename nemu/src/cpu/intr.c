#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

 // TODO();
 if(NO>cpu.idtr.limit){
   // printf("%d %d\n",NO,cpu.idtr.limit);
    assert(0);
   }
  int low,high;
  rtl_push(&cpu.eflags.value); 
  rtl_push(&cpu.cs);
  rtl_push(&ret_addr); //保存现场
  low=vaddr_read(cpu.idtr.base+8*NO,4) & 0x0000ffff;
  high=vaddr_read(cpu.idtr.base+8*NO+4,4) & 0xffff0000;
  decoding.jmp_eip= low | high; //连接跳转地址
  decoding.is_jmp=1;
}

void dev_raise_intr() {
}

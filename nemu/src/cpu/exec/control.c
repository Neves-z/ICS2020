#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
 // TODO();
  rtl_li(&t2,decoding.seq_eip); //把call指令的下一条指令的地址，即返回地址保存在临时寄存器中；
  rtl_push(&t2); //准备入口参数，返回地址进栈；
  decoding.is_jmp=1;//设置跳转标志

  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
 // TODO();
 rtl_pop(&decoding.jmp_eip);
  decoding.is_jmp=1;
  print_asm("ret");
}

make_EHelper(call_rm) {
 // TODO();
rtl_li(&t2,decoding.seq_eip); //把call指令的下一条指令的地址，即返回地址保存在临时寄存器中；
  rtl_push(&t2); //准备入口参数，返回地址进栈；
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;
  print_asm("call *%s", id_dest->str);
}

#include "cpu/exec.h"

make_EHelper(test) {
 // TODO();
  rtl_and(&t0, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&t0, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  print_asm_template2(test);
}

make_EHelper(and) {
 // TODO();
// printf("%d %d %d\n",id_dest->val, id_src->val,id_dest->width);
  rtl_and(&t1, &id_dest->val, &id_src->val); //目的操作数与源操作数相与
 // printf("%d \n",t1);
  operand_write(id_dest, &t1); //写入目的操作数
  rtl_update_ZFSF(&t1, id_dest->width); //更新ZFSF位
  t0 = 0;
  rtl_set_OF(&t0); //设置OF位为0
  rtl_set_CF(&t0); //设置CF位为0
  print_asm_template2(and);
}

make_EHelper(xor) {
 // TODO();
  rtl_xor(&t0,&id_dest->val,&id_src->val);
  operand_write(id_dest, &t0); 
  t1=0;
  rtl_set_OF(&t1);
  rtl_set_CF(&t1);
  rtl_update_ZFSF(&t0,id_dest->width);
  print_asm_template2(xor);
}

make_EHelper(or) {
 // TODO();
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_OF(&tzero);
  rtl_set_CF(&tzero);
  print_asm_template2(or);
}

make_EHelper(sar) {
 // TODO();
  // unnecessary to update CF and OF in NEMU
  // printf("%d %d\n",id_dest->val,id_src->val);   
 t1=id_dest->val;
  switch(id_dest->width){
    case 1: t2=24;
            rtl_shl(&t1,&t1,&t2);
            rtl_sar(&t1,&t1,&t2);
            break;
    case 2: t2=16;
            rtl_shl(&t1,&t1,&t2);
            rtl_sar(&t1,&t1,&t2);
            break;
  }
  rtl_sar(&t0,&t1,&id_src->val);
 // printf("%d %d\n",t0,t1);
  // rtl_sar(&t0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t0);
  rtl_update_ZFSF(&t0,id_dest->width);
  print_asm_template2(sar);
 // printf("%d\n",id_dest->width);
 ;
}

make_EHelper(shl) {
 // TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);
  rtl_update_ZFSF(&id_dest->val,id_dest->width);
  print_asm_template2(shl);
}

make_EHelper(shr) {
 // TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shr(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);
  rtl_update_ZFSF(&id_dest->val,id_dest->width);
  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
 // TODO();
  rtl_mv(&t0, &id_dest->val);
  rtl_not(&t0);
  operand_write(id_dest, &t0);
  print_asm_template1(not);
}

make_EHelper(rol) {
 // TODO();
  rtl_li(&t0,id_dest->val);
  rtl_shri(&t1,&t0,id_dest->width*8-id_src->val);
  rtl_shli(&t0,&t0,id_src->val);
  rtl_or(&t0,&t1,&t0);
  operand_write(id_dest,&t0);
  rtl_update_ZFSF(&t0,id_dest->width);;
  print_asm_template2(rol);
}

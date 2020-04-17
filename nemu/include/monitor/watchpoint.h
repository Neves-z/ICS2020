#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char exprs[42];           // 存储需要监视表达式
  int new_val;              // 储存表达式新值
  int old_val;              // 储存表达式旧值
  /* TODO: Add more members if necessary */


} WP;



WP* new_wp();
int free_wp(int NO);

#endif

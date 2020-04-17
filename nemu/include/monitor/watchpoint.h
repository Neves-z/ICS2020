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
int set_watchpoint(char *e);    //给予一个表达式e，构造以该表达式为监视目标的监视点，并返回编号
bool delete_watchpoint(int NO); //给予一个监视点编号，从已使用的监视点中归还该监视点到池中
void list_watchpoint(void);     //显示当前在使用状态中的监视点列表
WP* scan_watchpoint(void);      //扫描所有使用中的监视点，返回触发的监视点指针，若无触发返回NULL

#endif

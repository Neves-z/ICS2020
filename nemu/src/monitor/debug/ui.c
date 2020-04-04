#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args){
  if(args == NULL) {
  	cpu_exec(1);
  	return 0;
  }
  // TODO: 利用 strtok 读取出 N
  char *steps = strtok(NULL, " ");
  // TODO: 然后根据 N 来执行对应的 cpu_exec(N) 操作
  if(steps == NULL) {
        //N 缺省，即si
        cpu_exec(1);
   } 
  else {
    int n = 1;
    if(sscanf(steps, "%d", &n) == 1 &&n >= -1) {
       cpu_exec(n); 
    }
    else { 
      printf("Bad number: \e[0;31m%s\e[0m\n", steps); //输入不合法；
    }
	}
	return 0;
}

static int cmd_info(char *args){
    // 分割字符
    char *subcommand = strtok(NULL, " ");
    // 判断子命令是否是r
    if (strcmp(subcommand,"r")==0) {
        // 依次打印所有寄存器
        for(int j=0;j<8;j++) 
          printf("%s:\t%#010x\t%d\t\n", regsl[j], cpu.gpr[j]._32,cpu.gpr[j]._32);
        for(int j=0;j<8;j++){
          printf("%s:\t%#06x\t%d\t\n", regsw[j], cpu.gpr[j]._16,cpu.gpr[j]._16);
        }
        for(int j=0;j<8;j++){
          printf("%s:\t%#04x\t%d\t\n", regsb[j], cpu.gpr[j]._8[1],cpu.gpr[j]._8[1]);
        }
    }
    else if (strcmp(subcommand,"w")==0) {
        // 这里我们会在 PA1.3 中实现
    }
    else{
      printf("Bad Subcommand\n");
    }
    return 0;
}

static int cmd_x(char *args){
  //分割字符串，得到起始位置和要读取的次数
  char *ch=strtok(NULL," ");
  int num =atoi(ch);
  if (num<=0){
     printf("please input an positive integer\n");
     return 0;
  }
  char *expr=strtok(NULL," ");
  if (expr==NULL){
     printf("please input an hexadecimal address like 0x~\n");
     return 0;
  }
  vaddr_t addr;
  sscanf(expr,"%x",&addr);
  //循环使用 vaddr_read 函数来读取内存
  for (int i=0;i<num;i++){
    printf("%#010x:   ",addr);
    int memory=vaddr_read(addr,4);
    printf("%#010x   ",memory);
    int byte[4]={0,0,0,0};
    int i=3;
    while(memory){
      byte[i--]=memory%256;
      memory/=256;
    }
    for(int j=0;j<4;j++){
      /*if(byte[j]<16)
          printf("0%x  ",byte[j]);
      else {
        printf("%x  ",byte[j]);
      }*/
      printf("%02x  ",byte[j]);
    }
    addr+=4;	  
    printf("\n");
  }
  return 0;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Run serveral steps of the program", cmd_si },
  { "info", "Print register status or monitor information", cmd_info },
  { "x","Scan memory",cmd_x},
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}



void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

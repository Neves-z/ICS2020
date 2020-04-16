#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,TK_NEQ,TK_TEN, TK_HEX,
  TK_REG, TK_SYMB, TK_LS, TK_RS, TK_NG, TK_NL,TK_AND, TK_OR,

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // 减
  {"\\*", '*'},         // 乘
  {"\\/", '/'},         // 除
  {"\\%", '%'},         // 取余
  {"\\^",'^'},          // 次方
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},     
  {"&&",TK_AND},        // 与
  {"\\|\\|",TK_OR},     // 或
  {"!",'!'},            // 非
  {"0[x,X][0-9a-fA-F]+", TK_HEX},     // 十六进制数
  {"[0-9]+",TK_TEN},                  // 十进制数
  {"\\$e[a,d,c,b]x",TK_REG},          // 三十二位寄存器
  {"\\$e[s,b,i]p", TK_REG},       
  {"\\$e[d,s]i",TK_REG},
  {"\\$[a,b,c,d]x",TK_REG},           // 十六位寄存器
  {"\\$[s,b]p", TK_REG},  
  {"\\$[d,s]i",TK_REG},
  {"\\$[a,b,c,d][l,h]",TK_REG},       // 八位寄存器
  {"[a-zA-Z][a-zA-Z0-9_]*", TK_SYMB}, // 变量名或函数名
  {"\\(", '('},
  {"\\)", ')'},
  {"<<", TK_LS},		// 左移
  {">>", TK_RS},          	// 右移
  {"<=",TK_NG},			// 不大于
  {">=",TK_NL},			// 不小于
  {"<", '<'},
  {">", '>'},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

      /*  switch (rules[i].token_type) {
          default: TODO();
        }*/

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
*success = true;
  /* TODO: Insert codes to evaluate the expression. */
 // TODO();

  return 0;
}

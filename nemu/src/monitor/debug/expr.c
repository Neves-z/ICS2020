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

        switch (rules[i].token_type) {
	      	case TK_NOTYPE: break;     // 空格直接被丢弃
		case TK_REG:               // 寄存器
		case TK_TEN:               // 十进制数
		case TK_HEX:               // 十六进制数
		case TK_SYMB:{	     // 函数名或变量名
	          int j;
		  for(j=0;j<substr_len;j++)
		  {
                     tokens[nr_token].str[j]=*(substr_start+j);
               	  }
		  tokens[nr_token].str[j]='\0';
		}
		default:                    // 其他类型，例如'+','-'
              	  tokens[nr_token].type = rules[i].token_type;
		  nr_token ++;              // 更新已经识别的toke
        }
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


int check_parentheses(int p, int q) {
  if(!((tokens[p].type=='(')&&tokens[q].type==')'))
  {
    return false;  //the whole expression is not surrounded by a matched pair of parentheses
  }
  int i,j=0;
  for(i=p+1;i<q;i++)
 {
     if(tokens[i].type=='(')
	j++;
     else if(tokens[i].type==')')
        j--;
     if(j<0)
	return 0;
 }
 return j==0;
}

/*

uint32_t eval(int p, int q) {
  //printf("p: %d q:%d\n", p,q);
  if (p > q) {
    return false;
  }
  else if (p == q) {             // 单个表达式，只能为数字或者寄存器的情况
    int number = 0;
    if(tokens[p].type == TK_HEX) // 十六进制转化成十六进制数
      sscanf(tokens[p].str, "%x",&number);
    else if (tokens[p].type == TK_TEN)   // 十进制数转化成十进制数
      sscanf(tokens[p].str, "%d",&number);
    else if (tokens[p].type == TK_REG) {// 是寄存器
      for (int i = 0;i <strlen(tokens[p].str);i ++) {
        tokens[p].str[i] = tokens[p].str[i + 1]; // 把$号去掉
      }
      if (strcmp(tokens[p].str,"eip") == 0) {//如果是eip寄存器
        number = cpu.eip;
      }
      else {
          for (int i = 0;i < 8;i ++) {
            if (strcmp(tokens[p].str,regsl[i]) == 0) {//规定只有32位寄存器
              number = cpu.gpr[i]._32;
              break;
            }
          }
        }
    }
    return number;
  }
  else if (check_parentheses(p, q) == true) {  // 脱括号
    return eval(p + 1, q - 1);
  }
  else {
    int op,val1,val2;
    val1=val2=0;
    op = find_dominated_op(p,q);  //获取较低优先级运算符的位置
    //printf("op: %d\n", op);
    if(tokens[op].type!='!'&&tokens[op].type!='~')
	val1 = eval(p, op - 1);
    val2 = eval(op + 1, q);
    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case '%': return val1 % val2;
      case '^': return val1 ^ val2;
      case '<': return val1 < val2;
      case '>': return val1 > val2;
      case '!': return !val2;
      case '~': return ~val2;
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_NG: return val1 <= val2;
      case TK_NL: return val1 >= val2;
      case TK_AND: return val1 && val2;
      case TK_OR: return val1 || val2;
      case TK_LS: return val1 << val2;
      case TK_RS: return val1 >> val2;
      default: assert(0);
    }
  }
  return 0;
}
*/
uint32_t expr(char *e, bool *success) {
  *success = false;	
  if (!make_token(e)) {
    return 0;
  }

  if(!check_parentheses(0,nr_token-1))
  {
    printf("括号不匹配!\n");
    return 0;
  }
  *success=true;
	//return eval(0,nr_token-1);

  return 0;
}

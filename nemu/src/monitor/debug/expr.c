#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,TK_NEQ,TK_TEN, TK_HEX,
  TK_REG, TK_SYMB, TK_LS, TK_RS, TK_NG, TK_NL,TK_AND, TK_OR,TK_DEREF,TK_NEG,

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


int check_parentheses1(int p, int q) {
	if(!((tokens[p].type=='(')&&tokens[q].type==')'))
  {
    return false;  //the whole expression is not surrounded by a matched pair of parentheses
  }
  else
  {
    return true;
  }
  
}
int check_parentheses2(int p, int q) {
  int i,j=0;
  for(i=p;i<=q;i++)  {
     if(tokens[i].type=='(')
	j++;
     else if(tokens[i].type==')')
	j--;
     if(j<0){
        printf("表达式非法！\n");
        return 0;
    }
  }
  if(j==0)
    return true;
  else
  {
    printf("表达式非法！\n");
      return 0;
  }
}

static struct Node {
	int operand;
	int priority;
} table[] = {
	{TK_OR,10},
	{TK_AND,9},
	{'^',8},
	{TK_EQ,7},
	{TK_NEQ,7},
	{'>',6},
	{'<',6},
	{TK_NG,6},
	{TK_NL,6},
	{TK_LS,5},
	{TK_RS,5},
	{'+',4},
	{'-',4},
	{'*',3},
	{'/',3},
	{'%',3},
	{'!',2},
	{'~',2},
	{TK_DEREF,2},
	{TK_NEG,2},
};
int NR_TABLE = sizeof(table) / sizeof(table[0]);
int isoperand(int i){
  //检查是否为操作符
	return tokens[i].type!=TK_NOTYPE && tokens[i].type!=TK_TEN && tokens[i].type!=TK_REG && tokens[i].type!=TK_SYMB && tokens[i].type!=TK_HEX && tokens[i].type!=')' &&tokens[i].type!='(';
}

int op_comparative(int i){ //获取操作符的优先级
  int j;
  for(j=0;j<NR_TABLE;j++){
    if(tokens[i].type==table[j].operand){
      return table[j].priority;
    }
  }
  if(NR_TABLE==j){
    printf("未识别该操作符！\n");
  }
  return 0;
}
int find_dominated_op(int p,int q){
  int stack[50]; // 用数组模拟栈,存储操作符在数组tokens中的位置;
  int i=0;
  stack[0]=q;    // 注意：最后一个位置绝对不会是'(';
  bool mark=false;
  for(int j=p;j<=q;j++){
   if(isoperand(j)&&tokens[stack[i]].type!='(') {  // 是操作符或括号且不在括号内部
      if(i==0&&!mark){
        stack[i]=j;
	mark=true; //第一个操作符已入栈
	//printf("ok%d\n",j);
      }
      else{
        if(op_comparative(j)>=op_comparative(stack[i])){ //比栈顶操作符优先级低
          stack[++i]=j;
	  //printf("%d\n",stack[i]);
        }
      }
    } 
   if(tokens[j].type==')'&&tokens[stack[i]].type=='(') { //括号出栈
	if(i==0){
           stack[0]=q;
	   mark=false;   //栈内无操作符了
	}
	else i--;
    }
    if(tokens[j].type=='(') { //左括号进栈
        stack[++i]=j;
    }
  }
  return stack[i]; //栈顶优先级最低且相对最靠右
}




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
  else if (check_parentheses1(p, q) == true) {  // 脱括号
    return eval(p + 1, q - 1);
  }
  else {
    int op,val1,val2;
    val1=val2=0;
    op = find_dominated_op(p,q);  //获取较低优先级运算符的位置
   // printf("op: %d\n", op);
    if(tokens[op].type!='!'&&tokens[op].type!='~'&&tokens[op].type!=TK_NEG&&tokens[op].type!=TK_DEREF)
	val1 = eval(p, op - 1);
    val2 = eval(op + 1, q);
    if(tokens[op].type==TK_NEG&&tokens[op-1].type==TK_NEG)
      return val2;
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
      case TK_DEREF: return vaddr_read(val2,4);
      case TK_NEG: return -val2;
      default: assert(0);
    }
  }
  return 0;
}

uint32_t expr(char *e, bool *success) {
  *success = false;	
  if (!make_token(e)) {
    return 0;
  }

  if(!check_parentheses1(0,nr_token-1)&&check_parentheses2(0,nr_token-1))
  {
    printf("括号不匹配，但表达式合法！\n");
  }
  if(!check_parentheses2(0,nr_token-1))
  {
    return 0;
  }
  *success=true;
    int i;
    for(i = 0; i < nr_token; i++)
    {
         if(tokens[i].type == '*' && (i == 0 || isoperand(i-1)))
		tokens[i].type =TK_DEREF;
         else if(tokens[i].type == '-' && (i == 0 || isoperand(i-1)))
		tokens[i].type = TK_NEG;
    }
  return eval(0,nr_token-1);
}

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Tokenizer
//

// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_NUM, // 整数トークン
    TK_EOF, // 入力の終わりを表すトークン
} TokenKind;

// トークン型
typedef struct Token Token;
struct Token {
    TokenKind kind; // どの種類のトークンか
    Token *next; // 次のトークンを指すポインタ（連結リスト用）
    int val; // kindがTK_NUMの場合, その数値
    char *str; // kindがTK_RESERVEDの場合, その文字列
    int len; // kindがTK_RESERVEDの場合, その文字列の長さ
};

// 入力プログラム
extern char *user_input;

// 現在着目しているトークン
extern Token *token;

// トークナイザー関数（連結リストを作成）
Token *tokenize(void);

// トークン操作関数
bool consume(char *op);
void expect(char *op);
int expect_number(void);
bool at_eof(void);

//
// Parser (AST generator)
//

// ASTのノードの種類
typedef enum {
    ND_EQ, // ==
    ND_NE, // !=
    ND_LT, // <
    ND_LE, // <=
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // 整数
} NodeKind;

// ASTのノードの型
typedef struct Node Node;
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs; // 左の子（第1オペランド）を指すポインタ
    Node *rhs; // 右の子（第2オペランド）を指すポインタ
    int val; // 型がND_NUMの場合, その数値
};

Node *expr(void);

//
// asm code generator (x86-64)
//

void asm_prologue(void);
void gen(Node *node);

//
// Error handling
//

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

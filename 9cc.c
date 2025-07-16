#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_NUM, // 整数トークン
    TK_EOF, // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind; // どの種類のトークンか
    Token *next; // 次のトークンを指すポインタ（連結リスト用）
    int val; // kindがTK_NUMの場合, その数値
    char *str; // kindがTK_RESERVEDの場合, その文字列
    int len; // kindがTK_RESERVEDの場合, その文字列の長さ
};

// 入力プログラム
char *user_input;

// 現在着目しているトークン
Token *token;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号の場合には, トークンを1つ読み進めて
// 真を返す. それ以外の場合には偽を返す. 
bool consume(char *op) {
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len || 
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

// 次のトークンが期待している記号の場合には, トークンを1つ読み進めて
// 真を返す. それ以外の場合にはエラーを報告する.
void expect(char *op) {
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "'%c'ではありません", op);
    token = token->next;
}

// 次のトークンが数値の場合には, トークンを1つ読み進めてその数値を返す.
// それ以外の場合にはエラーを報告する.
int expect_number() {
    if (token->kind != TK_NUM)
        error("数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    if (kind == TK_RESERVED) {
        tok->str = (char *)malloc(strlen(str) + 1);
        if (tok->str != NULL) {
            strncpy(tok->str, str, strlen(str));
            tok->str[strlen(str)] = '\0';
        }
        tok->len = strlen(str);
    }
    cur->next = tok;
    return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {

        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        // 2文字の記号
        if (strncmp(p, "==", 2) == 0 || 
        strncmp(p, "!=", 2) == 0 || 
        strncmp(p, "<=", 2) == 0 || 
        strncmp(p, ">=", 2) == 0) {
            char str[3]; strncpy(str, p, 2); str[2] = '\0';
            cur = new_token(TK_RESERVED, cur, str);
            p += 2;
            continue;
        }

        // 1文字の記号  
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>') {
            char str[2]; strncpy(str, p, 1); str[1] = '\0';
            cur = new_token(TK_RESERVED, cur, str);
            p++;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p);    
    return head.next;
}

// トークナイズのデバッグ用
void debug_token(Token *token) {
    while (token != NULL) {
        if (token->kind == TK_RESERVED) {
            printf("Token: %s\n", token->str);
        } else if (token->kind == TK_NUM) {
            printf("Token: %d\n", token->val);
        }
        token = token->next;
    }
}

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

typedef struct Node Node;

// ASTのノードの型
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs; // 左の子（第1オペランド）を指すポインタ
    Node *rhs; // 右の子（第2オペランド）を指すポインタ
    int val; // 型がND_NUMの場合, その数値
};

// 非終端ノード（2項演算子）の作成
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 終端ノード（数値）の作成
Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// プロトタイプ宣言
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// expr = equality
Node *expr() {
    return equality();
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if (consume(">")) {
            node = new_node(ND_LT, add(), node);
        } else if (consume(">=")) {
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

// mul = unary ( "*" unary | "/" unary )*
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

// unary = ("+" | "-")? primary
Node *unary() {
    if (consume("+")) {
        return unary(); // +x = x
    }
    if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), unary()); // -x = 0 - x
    }
    return primary();
}

// primary = "(" expr ")" | num
Node *primary() {
    // 次のトークンが"("なら, "(" expr ")"のはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    // そうでなければ数値のはず
    return new_node_num(expect_number());
}

// デバッグ用
void debug_node(Node *node) {
    if (node->kind == ND_NUM) {
        printf("Node: %d\n", node->val);
        return;
    }
    debug_node(node->lhs);
    debug_node(node->rhs);
    printf("Node: %d\n", node->kind);
}

// ASTからasmコードを生成
void gen(Node *node) {
    // 終端ノード (数値)ならスタックにpush
    if (node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    // 非終端ノード (2項演算子)なら, まず左辺と右辺をそれぞれ処理
    gen(node->lhs);
    gen(node->rhs);

    // スタックから2つの値をpopして、それらの計算結果をpush
    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            // x86-64でのidivの仕様のため. 
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n"); // raxの下位8ビット
            printf("  movzb rax, al\n"); // raxの上位56ビットをゼロクリア
            break;
        case ND_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
    }

    printf("  push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    // トークナイズする
    user_input = argv[1];
    token = tokenize();
    // debug_token(token);

    // ASTを生成
    Node *node = expr();
    // debug_node(node);

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // ASTを下りながらコード生成
    gen(node);

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}

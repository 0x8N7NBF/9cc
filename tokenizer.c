#include "9cc.h"

//
// Tokenizer
//

// 入力プログラム
char *user_input;

// 現在着目しているトークン
Token *token;

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

// 次のトークンが識別子の場合には, トークンを1つ読み進めてその識別子を返す.
// それ以外の場合にはNULLを返す.
Token *consume_ident() {
    if (token->kind != TK_IDENT)
        return NULL;
    Token *tok = token;
    token = token->next;
    return tok;
}

// 次のトークンが期待している記号の場合には, トークンを1つ読み進める. 
// それ以外の場合にはエラーを報告する. (閉じ括弧などで使うことが多い)
void expect(char *op) {
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}

// 次のトークンが数値の場合には, トークンを1つ読み進めてその数値を返す.
// それ以外の場合にはエラーを報告する.
int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
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
        if (startswith(p, "==") || startswith(p, "!=") || 
            startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        // 1文字の記号  
        if (strchr("+-*/()<>=;", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        // 識別子(小文字アルファベット1文字の変数)
        if ('a' <= *p && *p <= 'z' ||
            'A' <= *p && *p <= 'Z' ||
            *p == '_') {
            cur = new_token(TK_IDENT, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);    
    return head.next;
}

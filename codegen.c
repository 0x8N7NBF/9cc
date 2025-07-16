#include "9cc.h"

//
// asm code generator (x86-64)
//

// asmの前半部分を出力
void asm_prologue() {
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
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
            printf("  sete al\n"); // raxの下位8ビットに結果を格納
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
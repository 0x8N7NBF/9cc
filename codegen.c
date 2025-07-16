#include "9cc.h"

//
// asm code generator (x86-64)
//

// ローカル変数のアドレスをスタックにpushする
void gen_lval(Node *node) {
    if (node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");
    
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", node->offset);
        printf("  push rax\n");
}

// asmの前半部分を出力
void asm_prologue() {
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
}

// ASTからasmコードを生成
void gen(Node *node) {
    
    switch (node->kind) {
        // 終端ノード (数値)ならスタックにpush
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        // 終端ノード (変数)ならスタックにpush
        case ND_LVAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        // 代入ノードなら, 左辺(変数)のアドレスを計算して, 右辺を処理して, 
        // それからそのアドレスが指すメモリに代入する. 代入した値そのものも返す
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
    }

    // 非終端ノード (2項演算子)なら, まず左辺と右辺をそれぞれ処理
    gen(node->lhs);
    gen(node->rhs);

    // スタックから2つの値をpopして, それらの計算結果をpush
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
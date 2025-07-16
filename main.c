#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    // トークナイズする
    user_input = argv[1];
    token = tokenize();

    // パース（ASTを生成）
    program();

    // アセンブリの前半部分を出力
    asm_prologue();

    // プロローグ
    // ローカル変数26個分の領域を確保
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n"); // 26個×8バイト

    // 先頭の式から順にコード生成
    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        
        // 式の評価結果としてスタックに1つ値が残っているはずなので, 
        // スタックが溢れないようにpopしておく
        printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているはずなので, それが返り値になる. 
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");

    return 0;
}

#include <stdio.h>

const char flag[] = {'b', 'c', 't', 'f', '{', 112, 107, 'g', 114, 16, '}'};

int check(char *buf) {

    if (buf[0] != flag[0] || buf[1] != flag[1] || buf[2] != flag[2] ||
        buf[3] != flag[3] || buf[4] != flag[4]) {
        return 0;
    }

    if (flag[5] - 60 != buf[5]) {
        return 0;
    }

    if ((buf[6] ^ 0x33) + 12 != flag[6]) {
        return 0;
    }

    if (flag[7] != buf[7]) {
        return 0;
    }

    if (buf[8] - 0x30 != 3) {
        return 0;
    }

    if ((buf[9]^0x10) != flag[8]) {
        return 0;
    }

    if ((buf[10] ^ buf[9]) != flag[9]) {
        return 0;
    }

    if ((buf[11] - 0x30) - 1 != buf[8] - 0x30) {
        return 0;
    }

    if (buf[12] != '!') {
        return 0;
    }

    if (buf[13] != flag[10]) {
        return 0;
    }
}

int main(void) {
    char buf[0x30];
    printf("Product key> ");
    fgets(buf, sizeof(buf) - 1, stdin);

    if (check(buf)) {
        printf("Key correct, activating.\n");
        return 0;
    } else {
        printf("Key incorrect, not activating.\n");
        return 1;
    }
}

#include <stdio.h>

int check(char * buf) {
    if (buf[0 ] != 'b') {
        return 0;
    }

    if (buf[1 ] != 'c') {
        return 0;
    }

    if (buf[2 ] != 't') {
        return 0;
    }

    if (buf[3 ] != 'f') {
        return 0;
    }

    if (buf[4 ] != '{') {
        return 0;
    }

    if (buf[5 ] != '1') {
        return 0;
    }

    if (buf[6 ] != '3') {
        return 0;
    }

    if (buf[7 ] != '3') {
        return 0;
    }

    if (buf[8 ] != '&') {
        return 0;
    }

    if (buf[9] != '_') {
        return 0;
    }

    if (buf[10] != 'l') {
        return 0;
    }

    if (buf[11] != 'e') {
        return 0;
    }

    if (buf[12] != 't') {
        return 0;
    }

    if (buf[13] != 'm') {
        return 0;
    }

    if (buf[14] != 'e') {
        return 0;
    }

    if (buf[15] != 'i') {
        return 0;
    }

    if (buf[16] != 'n') {
        return 0;
    }

    if (buf[17] != '_') {
        return 0;
    }

    if (buf[18] != '1') {
        return 0;
    }

    if (buf[19] != '2') {
        return 0;
    }

    if (buf[20] != '3') {
        return 0;
    }

    if (buf[21] != '}') {
        return 0;
    }

    return 1;
}

int main(void) {
    char buf[0x30];

    printf("Product Key> ");

    fgets(buf, sizeof(buf) - 1, stdin);

    if (check(buf)) {
        printf("Key correct, activating.\n");
        return 0;
    } else {
        printf("Key incorrect, not activating.\n");
        return 1;
    }
}

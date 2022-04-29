#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int bet_loss = 10;
int bet_win = 10;
int balance = 100;

char *flag;

void read_file(const char *filename, char **buf) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    if (fsize == -1) {
        perror("ftell");
        exit(1);
    }
    rewind(fp);
    *buf = (char *)calloc(fsize + 1, sizeof(char));
    if (!buf) {
        perror("calloc");
        exit(1);
    }
    if (fread(*buf, sizeof(char), fsize, fp) != fsize) {
        perror("fread");
        exit(1);
    }
}

void setup(void) {
    read_file("flag.txt", &flag);
    setvbuf(stdin, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);
    setvbuf(stderr, 0, 2, 0);
    FILE *stream = fopen("/dev/urandom", "r");
    if (!stream) {
        perror("fopen");
        exit(1);
    }
    int rand_data;
    if (fread(&rand_data, 1, 4, stream) != 4) {
    	perror("fread");
    	exit(1);
    }
    srand(rand_data);
}

void give_flag() {
	printf("%s\n", flag);
}

void casino()
{
	char guess_word[8];
	char correct_word[8];

	printf("Welcome to the casino! A great prize awaits you when you hit 1000 coins ;)\n");

	memset(correct_word, 0, 8);
	memset(guess_word, 0, 8);

	while (balance > 0) {

		printf("Your current balance: %d\n", balance);

		for (int i = 0; i < 4; i++) 
			correct_word[i] = rand() % 26 + 'a';

		printf("Guess me a string of length 4 with lowercase letters: ");
		gets(guess_word);

		printf("Your guess: %s\n", guess_word);
		printf("Correct word: %s\n", correct_word);
		if (!strcmp(correct_word, guess_word)) {
			printf("You won (wow)! +%d coins.\n", bet_win);
			balance += bet_win;
		}
		else {
			printf("Bummer, you lost. -%d coins.\n", bet_loss);
			balance -= bet_loss;
		}


		if (balance < 0) {
			printf("You lost your entire balance! Better luck next time...\n");
			break;
		}
		if (balance >= 1000) {
			give_flag();
			break;
		}
	}
	printf("See you next time!\n");
}

int main()
{
	setup();
	casino();
}
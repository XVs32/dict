#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char output[50];

char *random_word()
{
    int len = rand() % 15 + 1;

    output[0] = rand() % 26 + 65;
    int i;
    for (i = 1; i < len; i++) {
        output[i] = rand() % 26 + 97;
    }
    output[i] = '\0';

    return output;
}

char *random_error(char *input)
{
    int i = 1;
    while (input[i] != '\0' && i < 15) {
        if (rand() % 10 == 0) {
            input[i] = ' ';
        }
        if (rand() % 10 == 0) {
            input[i] = '\0';
            return input;
        }
        i++;
    }
    input[4] = '\0';

    return input;
}

int main(int argc, char **argv)
{
    char option[] = "afsdq";
    // char option[] = "ad";

    srand(time(NULL));

    FILE *output;
    output = fopen("random_command.txt", "w");

    int i = 8000;
    while (i--) {
        // fprintf(output,"%c\n%s\n",option[rand()% 4],random_word());
        fprintf(output, "%c\n%s\n", option[2], random_word());
    }

    fprintf(output, "q\n");
    fclose(output);



    output = fopen("simulate_command.txt", "w");

    FILE *input;
    input = fopen("city_one_col.txt", "r");

    i = 8000;
    char buf[500];
    while (i--) {
        fgets(buf, 128, input);
        // fprintf(output,"%c\n%s",option[rand()% 4],random_error(buf));
        fprintf(output, "%c\n%s\n", option[2], random_error(buf));
    }

    fprintf(output, "q\n");

    fclose(input);
    fclose(output);

    return 0;
}
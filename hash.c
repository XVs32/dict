#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TableSize 90000

int count = 0;

void printCHAR(char *str)
{
    while (*str) {
        printf("%c", *str);
        str++;
    }
    printf("\n");
}

unsigned int SDBMHash(char *str)
{
    unsigned int hash = 0;

    while (*str) {
        // printf("%c",*str);
        hash = (*str++) + (hash << 6) + (hash << 16) - hash;
    }

    // printf("\n");
    return (hash);
}

// RS Hash Function
unsigned int RSHash(char *str)
{
    unsigned int b = 378551;
    unsigned int a = 63689;
    unsigned int hash = 0;

    while (*str) {
        hash = hash * a + (*str++);
        a *= b;
    }
    return (hash);
}

// JS Hash Function
unsigned int JSHash(char *str)
{
    unsigned int hash = 1315423911;

    while (*str) {
        hash ^= ((hash << 5) + (*str++) + (hash >> 2));
    }

    return (hash);
}

// P. J. Weinberger Hash Function
unsigned int PJWHash(char *str)
{
    unsigned int BitsInUnignedInt = (unsigned int) (sizeof(unsigned int) * 8);
    unsigned int ThreeQuarters = (unsigned int) ((BitsInUnignedInt * 3) / 4);
    unsigned int OneEighth = (unsigned int) (BitsInUnignedInt / 8);
    unsigned int HighBits = (unsigned int) (0xFFFFFFFF)
                            << (BitsInUnignedInt - OneEighth);
    unsigned int hash = 0;
    unsigned int test = 0;

    while (*str) {
        hash = (hash << OneEighth) + (*str++);
        if ((test = hash & HighBits) != 0) {
            hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
        }
    }

    return (hash);
}

// ELF Hash Function
unsigned int ELFHash(char *str)
{
    unsigned int hash = 0;
    unsigned int x = 0;

    while (*str) {
        hash = (hash << 4) + (*str++);
        if ((x = hash & 0xF0000000L) != 0) {
            hash ^= (x >> 24);
            hash &= ~x;
        }
    }

    return (hash);
}

// BKDR Hash Function
unsigned int BKDRHash(char *str)
{
    unsigned int seed = 131;  // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*str) {
        hash = hash * seed + (*str++);
    }

    return (hash);
}

// DJB Hash Function
unsigned int DJBHash(char *str)
{
    unsigned int hash = 5381;

    while (*str) {
        hash += (hash << 5) + (*str++);
    }

    return (hash);
}

// AP Hash Function
unsigned int APHash(char *str)
{
    unsigned int hash = 0;
    int i;

    for (i = 0; *str; i++) {
        if ((i & 1) == 0) {
            hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
        } else {
            hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
        }
    }
    return (hash);
}

static unsigned int djb2(char *str)
{
    // const char *str = _str;
    unsigned int hash = 5381;
    char c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return (hash);
}

unsigned int jenkins(char *str)
{
    // const char *key = _str;
    unsigned int hash = 0;
    while (*str) {
        hash += *str;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        str++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return (hash);
}


void testHash(unsigned int (*hash)(char *))
{
    count = 0;
    FILE *fp = fopen("data.txt", "r");
    if (!fp) {
        printf("error");
        return;
    }
    char word[256];
    int rtn = 0;
    unsigned int *table = calloc(TableSize, sizeof(unsigned int));

    while ((rtn = fscanf(fp, "%s", word)) != EOF) {
        unsigned int res = hash(word);
        // printCHAR(word);
        unsigned int idx = (res % TableSize);
        while (table[idx]) {  // the content of table[idx] != hash
            if (table[idx] == (res)) {
                // printf("------------------\n");
                count++;
                break;
            }
            if (idx == TableSize - 1) {
                idx = 0;
            }
            idx++;
        }
        table[idx] = res;
        memset(word, 0, 255);
    }
    printf("the collision is: %d \n", count);
    free(table);
    fclose(fp);
}

int main()
{
    testHash(SDBMHash);
    testHash(RSHash);
    testHash(JSHash);
    testHash(PJWHash);
    testHash(ELFHash);
    testHash(BKDRHash);
    testHash(DJBHash);
    testHash(APHash);
    testHash(DJBHash);
    testHash(jenkins);
    testHash(djb2);
}

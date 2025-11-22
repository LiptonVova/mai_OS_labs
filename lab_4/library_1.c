#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void reverse(char *str) {
    // реверс строки
    int n = strlen(str);
    for (int i = 0; i < n/2; ++i) {
        char temp = str[n - i - 1];
        str[n - i - 1] = str[i];
        str[i] = temp;
    }
}

int prime_count(int a, int b) {
    // подсчет колва простых чисел на отрезке [a, b]
    // наивная реализация
    const char msg[] = "-- Log[library_1]: function prime_count naive impl\n";
    write(STDOUT_FILENO, msg, sizeof(msg));

    int count = 0; 
    for (int num = a; num <= b; ++num) {
        int is_prime = 1;
        for (int del = 2; del < num; ++del) {
            if (num % del == 0) {
                is_prime = 0;
                break;
            }
        }
        if (is_prime) ++count;
    }
    return count;
}

char *convert(int x) {
    // перевод из десятичной системы счисления
    // реализация 1: перевод в двоичную
    const char msg[] = "-- Log[library_1]: function convert to 2 c/c\n";
    write(STDOUT_FILENO, msg, sizeof(msg));


    const int mask = 1;
    char *result = (char*)malloc(sizeof(char) * 32);
    if (!result) return NULL;

    int i = 0;
    if (x == 0) {
        result[i] = '0';
    }

    int is_negative = 0;
    if (x < 0) {
        is_negative = 1;
        x *= -1;
    }

    while (x != 0) {
        result[i] = (mask & x) + '0'; 
        ++i;
        x >>= mask;
    }

    if (is_negative) {
        result[i] = '-';
        ++i;
    }

    result[i] = '\0';
    reverse(result);
    return result;
}


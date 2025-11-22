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
    // реализация 2: решето Эратосфена
    const char msg[] = "-- Log[library_2]: function prime_count using sieve\n";
    write(STDOUT_FILENO, msg, sizeof(msg));

    int numbers[b + 1];
    // 0 - простое
    // 1 - составное
    for (int i = 0; i <= b; ++i) {
        numbers[i] = 0;
    }


    for (int i = 2; i <= b; ++i) {
        if (numbers[i] == 1) continue;
        for (int j = i + i; j <= b; j += i) {
            numbers[j] = 1;
        }
    }

    int result = 0;
    for (int i = a; i <= b; ++i) {
        if (!numbers[i]) ++result;
    }
    return result;
}

char *convert(int x) {
    // перевод из 10 с/с в 3 с/с
    const char msg[] = "-- Log[library_2]: function convert to 3 c/c\n";
    write(STDOUT_FILENO, msg, sizeof(msg));

    char *result = (char*)malloc(sizeof(char) * 100);
    if (!result) return NULL;
    int index = 0;
    
    if (x == 0) {
        result[index] = '0';
        ++index;
    }

    int is_negative = 0;
    if (x < 0) {
        is_negative = 1;
        x *= -1;
    }

    while (x != 0) {
        result[index] = (x % 3) + '0';
        ++index;
        x /= 3;
    }

    if (is_negative) {
        result[index] = '-';
        ++index;
    }
    result[index] = '\0';
    reverse(result);
    return result;
}
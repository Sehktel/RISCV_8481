#include "../include/string.h"

/**
 * @brief Сравнивает две NUL-терминированные строки (bare-metal аналог strcmp).
 * @param s1 Указатель на первую строку.
 * @param s2 Указатель на вторую строку.
 * @return 0, если строки равны; разница кодов первых различающихся символов
 *         в противном случае (положительная, если s1 > s2).
 * @note Досрочный выход происходит, когда *s1 == 0 или символы различны,
 *       поэтому корректно сравнивает строки разной длины.
 */
int kstrcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/**
 * @brief Вычисляет длину NUL-терминированной строки.
 * @param s Указатель на строку.
 * @return Количество символов в строке без учёта завершающего '\0'.
 */
int kstrlen(const char* s) {
    int len = 0;
    while (s[len])
        len++;
    return len;
}
#pragma once

#ifdef __cplusplus
extern "C" {
void strncap(char* dst, char* src, unsigned size);
char* strlpadch(char* dst, char* src, char c, unsigned size);
char* strlpad(char* str, int size, char c);
}
#else
void strncap(char* dst, char* src, unsigned size);
char* strlpadch(char* dst, char* src, char c, unsigned size);
char* strlpad(char* str, int size, char c);
#endif
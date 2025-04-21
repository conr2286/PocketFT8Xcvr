#pragma once

#ifdef __cplusplus
extern "C" {
void strncap(char* dst, char* src, unsigned size);
char* strlpad(char* dst, char* src, char c, unsigned size);
}
#else
void strncap(char* dst, char* src, unsigned size);
char* strlpad(char* dst, char* src, char c, unsigned size);
#endif
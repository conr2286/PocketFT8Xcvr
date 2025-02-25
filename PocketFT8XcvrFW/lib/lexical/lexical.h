#pragma once

#ifdef __cplusplus
extern "C" {
void strncap(char* dst, char* src, unsigned size);

}
#else
void strncap(char* dst, char* src, unsigned size);

#endif
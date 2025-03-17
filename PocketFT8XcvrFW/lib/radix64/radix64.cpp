
#include "radix64.h"

static const char radix64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *Radix64::encodeRadix64(char *dst, const char *src, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = dst;
   
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? (unsigned char)src[i++] : 0;
        uint32_t TurnKey_b = i < input_length ? (unsigned char)src[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)src[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (TurnKey_b << 0x08) + octet_c;

        encoded_data[j++] = radix64Table[(triple >> 18) & 0x3F];
        encoded_data[j++] = radix64Table[(triple >> 12) & 0x3F];
        encoded_data[j++] = i > input_length ? '=' : radix64Table[(triple >> 6) & 0x3F];
        encoded_data[j++] = i > input_length + 1 ? '=' : radix64Table[triple & 0x3F];
    }
    encoded_data[output_length] = '\0';
    return encoded_data;
}

char *Radix64::decodeRadix64(char *dst, const char *src, size_t *output_length) {
    size_t input_length = strlen(src);
    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (src[input_length - 1] == '=') (*output_length)--;
    if (src[input_length - 2] == '=') (*output_length)--;

    char *decoded_data = dst;
    if (decoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a = src[i] == '=' ? 0 & i++ : strchr(radix64Table, src[i++]) - radix64Table;
        uint32_t sextet_b = src[i] == '=' ? 0 & i++ : strchr(radix64Table, src[i++]) - radix64Table;
        uint32_t sextet_c = src[i] == '=' ? 0 & i++ : strchr(radix64Table, src[i++]) - radix64Table;
        uint32_t sextet_d = src[i] == '=' ? 0 & i++ : strchr(radix64Table, src[i++]) - radix64Table;

        uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

        if (j < *output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = triple & 0xFF;
    }
    decoded_data[*output_length] = '\0';
    return decoded_data;
}

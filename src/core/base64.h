#ifndef BASE64_H
#define BASE64_H

/*
 * Decode a base64 encoded string into a newly allocated buffer.
 *
 * The caller is responsible for freeing the returned buffer.
 * Returns NULL if the input is not valid base64.
 * */
unsigned char *base64_decode(const char *encoded_src, int *out_len);

#endif  // BASE64_H

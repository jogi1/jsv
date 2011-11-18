#ifndef MD4_H
#define MD4_H
unsigned MD4_BlockChecksum (void *buffer, int length);
void MD4_BlockFullChecksum (void *buffer, int len, unsigned char *outbuf);
#endif

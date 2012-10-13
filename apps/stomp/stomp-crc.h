#ifndef CRC_H_
#define CRC_H_

/* Return a 32-bit CRC of the contents of the buffer. */
unsigned long stomp_crc_crc32(const unsigned char *str, unsigned int len);

#endif /* CRC_H_ */

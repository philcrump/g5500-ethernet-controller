#ifndef __CRC_H__
#define __CRC_H__

uint8_t crc8(const uint8_t *buf, uint32_t size);

uint32_t crc32(const uint8_t *buf, uint32_t size);

#endif /* __CRC_H__ */
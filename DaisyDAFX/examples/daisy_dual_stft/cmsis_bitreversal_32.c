#include <stdint.h>

void arm_bitreversal_32(uint32_t *pSrc, const uint16_t bitRevLen,
                        const uint16_t *pBitRevTab) {
  uint8_t *base = (uint8_t *)pSrc;

  for (uint16_t i = 0; i + 1u < bitRevLen; i += 2u) {
    uint32_t *a = (uint32_t *)(base + pBitRevTab[i]);
    uint32_t *b = (uint32_t *)(base + pBitRevTab[i + 1u]);

    uint32_t tmp = a[0];
    a[0] = b[0];
    b[0] = tmp;

    tmp = a[1];
    a[1] = b[1];
    b[1] = tmp;
  }
}

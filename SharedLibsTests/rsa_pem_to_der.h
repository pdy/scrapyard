#include <cstddef>
#include <cstdint>

#ifndef RSA_PEM_TO_DER_H_
#define RSA_PEM_TO_DER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
  uint8_t *bytes;
  size_t size;
} Buffer;

void freeBuffer(Buffer *buf);

Buffer rsa_pem_to_der(const char *pem);

void logLibOsslVersion();


#ifdef __cplusplus
}
#endif

#endif // RSA_POEM_TO_DER_H_

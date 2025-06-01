#include "rsa_pem_to_der.h"
#include "simplelog/simplelog.hpp"

#include <openssl/pem.h>

Buffer rsa_pem_to_der(const char *pem)
{
  return Buffer{nullptr, 0};
}

void freeBuffer(Buffer *buf)
{
  if(buf && buf->bytes)
    delete[] buf->bytes;
}

void logLibOsslVersion()
{
  LOG << "Shared Object is using " << OpenSSL_version(OPENSSL_VERSION);
}

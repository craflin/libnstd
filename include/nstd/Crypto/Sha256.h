
#pragma once

#include <nstd/Memory.h>

class Sha256
{
public:
  static const usize blockSize = 64;
  static const usize digestSize = 32;

  Sha256() {reset();}

  void reset();

  void update(const byte* data, usize size);
  void finalize(byte (&digest)[digestSize]);

  static void hash(const byte* data, usize size, byte (&result)[digestSize])
  {
    Sha256 sha256;
    sha256.update(data, size);
    sha256.finalize(result);
  }

  static void hmac(const byte* key, usize keySize, const byte* message, usize messageSize, byte (&result)[digestSize])
  {
    Sha256 sha256;
    byte hashKey[blockSize];
    if(keySize > blockSize)
    {
      sha256.update(key, keySize);
      sha256.finalize((byte (&)[digestSize])hashKey);
      Memory::zero(hashKey + 32, 32);
    }
    else
    {
      Memory::copy(hashKey, key, keySize);
      if(keySize < blockSize)
        Memory::zero(hashKey + keySize, blockSize - keySize);
    }
    
    byte oKeyPad[blockSize];
    byte iKeyPad[blockSize];
    for(int i = 0; i < 64; ++i)
    {
      oKeyPad[i] = hashKey[i] ^ 0x5c;
      iKeyPad[i] = hashKey[i] ^ 0x36;
    }
    byte hash[digestSize];
    sha256.update(iKeyPad, blockSize);
    sha256.update(message, messageSize);
    sha256.finalize(hash);
    sha256.update(oKeyPad, blockSize);
    sha256.update(hash, digestSize);
    sha256.finalize(result);
  }

private:
  uint32 state[8];
  uint64 count;
  byte buffer[64];

private:
  class Private;
};

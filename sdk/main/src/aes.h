/*****************************************************************************
*
* aes.h: AES - ECB and CBC Encryption and Decryption Algorithm
*        This implementation contains modifications to allow for verbose
*.        and buffered output compared to the orignal source
*
* @auth   Jason Tsang
*         Kokke: https://github.com/kokke/tiny-AES-c
*
* @date   2018/02/08
*
******************************************************************************/

#ifndef SRC_AES_H_
#define SRC_AES_H_

#include <stdint.h>
#include <string.h> // CBC mode, for memset
#include <stdio.h>

// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES encryption in CBC-mode of operation.
// ECB enables the basic ECB 16-byte block algorithm. All can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.
#ifndef CBC
  #define CBC 1
#endif

#ifndef ECB
  #define ECB 1
#endif

typedef uint8_t bool;
#define true 1
#define false 0

#define AES_BLOCKLEN 16 //Block length in bytes AES is 128b block only

#define AES_KEYLEN 16   // Key length in bytes
#define AES_keyExpSize 176


struct AES_ctx
{
  uint8_t RoundKey[AES_keyExpSize];
#if (defined(CBC) && (CBC == 1))
  uint8_t Iv[AES_BLOCKLEN];
#endif
};

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key);
#if (defined(CBC) && (CBC == 1))
void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);
void AES_ctx_set_iv(struct AES_ctx* ctx, const uint8_t* iv);
#endif

#if defined(ECB) && (ECB == 1)
// buffer size is exactly AES_BLOCKLEN bytes;
// you need only AES_init_ctx as IV is not used in ECB
// NB: ECB is considered insecure for most uses
void AES_ECB_encrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length, bool verbose);
void AES_ECB_decrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length, bool verbose);

#endif // #if defined(ECB) && (ECB == !)


#if defined(CBC) && (CBC == 1)
// buffer size MUST be mutile of AES_BLOCKLEN;
// Suggest https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx via AES_init_ctx_iv() or AES_ctx_set_iv()
//        no IV should ever be reused with the same key
void AES_CBC_encrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length, bool verbose);
void AES_CBC_decrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length, bool verbose);

#endif // #if defined(CBC) && (CBC == 1)


typedef enum {SW0 = 0x2b7e, SW1 = 0x1516, SW2 = 0x28ae, SW3 = 0xd2a6, SW4 = 0xabf7, SW5 = 0x1588, SW6 = 0x09cf, SW7 = 0x4f3c} SWITCH_MAPPING;


#endif /* SRC_AES_H_ */

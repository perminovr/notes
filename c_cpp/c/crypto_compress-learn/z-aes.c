#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/if_packet.h>

#include <libssh/libssh.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <zlib.h>



#define BUF_SIZE_MAX    1024



static void DumpHex(const void* data, size_t size);
static Bytef *z_cmprs(const Bytef *buf, uInt size, uInt *resultsize);
static Bytef *z_uncmprs(const Bytef *buf, uInt size, uInt *resultsize);
int encrypt(uint8_t *plaintext, int plaintext_len, uint8_t *key, uint8_t *iv, uint8_t *ciphertext);
int decrypt(uint8_t *ciphertext, int ciphertext_len, uint8_t *key, uint8_t *iv, uint8_t *plaintext);



int main()
{
    // openssl enc -nosalt -aes-256-cbc -k SECRETPHRASE -P
    uint8_t *key = (uint8_t *)"01234567890123456789012345678901";
    uint8_t *iv = (uint8_t *)"0123456789012345";
    uint8_t *plaintext = (uint8_t *)"\e[1;31;41m\e[2;3fSOME_TEXT      "
    "\e[1;31;41m\e[2;3fsdlgknaonfkal"
    "\e[1;31;41m\e[2;3f1312667854tdf"
    "\e[1;31;41m\e[2;3fa'mdpanflasfasaasdasdas"
    "\e[1;31;41m\e[2;3fdqwndlnjnal  adlkmaslkd akdkjasd ad"
    "\e[1;31;41m\e[2;3f][q]weps, dfaodjouaidbgv1eqhgfj anvs"
    "\e[1;31;41m\e[2;3f651adsd61sf5x0bd 35161z 0dfzvsdz "
    "\e[1;31;41m\e[2;3f-=)(_*92-=[1PKM .QFWA"
    "\e[1;31;41m\e[2;3fN:IOBdvGJ KMpiBJ NQl dNHbQOBD ASdasl"
    "\e[1;31;41m\e[2;3f}{QLKJ8gaubd k.:JPHBO: K>Nd"
    "";
    uLong zSize;
    Bytef *p;
    uint8_t ciphertext[BUF_SIZE_MAX] = {0};
    uint8_t decryptedtext[BUF_SIZE_MAX] = {0};
    int ciphertext_len;
    int decryptedtext_len;

    printf("plaintext: %d\n", strlen(plaintext)+1);

    p = z_cmprs(plaintext, strlen(plaintext)+1, &zSize);
    ciphertext_len = encrypt(p, zSize, key, iv, ciphertext);
    
    printf("ciphertext size: %d\n", ciphertext_len);
    DumpHex(ciphertext, ciphertext_len);
    
    decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv, decryptedtext);
    p = z_uncmprs(decryptedtext, decryptedtext_len, &zSize);

    DumpHex(p, zSize);

    return 0;
}



static Bytef *z_cmprs(const Bytef *buf, uInt size, uInt *resultsize)
{
    static Bytef outbuf[BUF_SIZE_MAX];

    z_stream zstrm;
    zstrm.zalloc = 0;
    zstrm.zfree = 0;
    zstrm.opaque = 0;
    zstrm.next_in = buf; // input char array
    zstrm.avail_in = size; // size of input, string + terminator
    zstrm.next_out = outbuf; // output char array
    zstrm.avail_out = BUF_SIZE_MAX; // size of output

    deflateInit(&zstrm, Z_BEST_COMPRESSION);
    deflate(&zstrm, Z_FINISH);
    deflateEnd(&zstrm);

    *resultsize = zstrm.total_out;
    printf("Compressed size is: %lu\n", zstrm.total_out);

    return outbuf;
}



static Bytef *z_uncmprs(const Bytef *buf, uInt size, uInt *resultsize)
{
    static Bytef outbuf[BUF_SIZE_MAX];

    z_stream zstrm;
    zstrm.zalloc = 0;
    zstrm.zfree = 0;
    zstrm.opaque = 0;
    zstrm.next_in = buf; // input char array
    zstrm.avail_in = size; // size of input, string + terminator
    zstrm.next_out = outbuf; // output char array
    zstrm.avail_out = BUF_SIZE_MAX; // size of output

    inflateInit(&zstrm);
    inflate(&zstrm, Z_NO_FLUSH);
    inflateEnd(&zstrm);

    *resultsize = zstrm.total_out;
    printf("Uncompressed size is: %lu\n", zstrm.total_out);

    return outbuf;
}



int encrypt(uint8_t *plaintext, int plaintext_len, uint8_t *key,
            uint8_t *iv, uint8_t *ciphertext)
{
    int len;
    int ciphertext_len;

    // create context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        ;
    }

    // init enc operation (alg)
    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        ;
    }

    // encryption
    if (!EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        ;
    }
    ciphertext_len = len;

    // padding
    if (!EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        ;
    }
    ciphertext_len += len;

    // clean up
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}



int decrypt(uint8_t *ciphertext, int ciphertext_len, uint8_t *key,
            uint8_t *iv, uint8_t *plaintext)
{
    int len;
    int plaintext_len;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        ;
    }


    // init dec operation (alg)
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        ;
    }

    // decrypt
    if (!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
        ;
    }
    plaintext_len = len;

    // padding
    if (!EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        ;
    }
    plaintext_len += len;

    // clean up
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}



static void DumpHex(const void* data, size_t size)
{
    #define DUMPHEX_WIDTH 16
	char ascii[DUMPHEX_WIDTH+1];
	size_t i, j;
	ascii[DUMPHEX_WIDTH] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((uint8_t*)data)[i]);
		if (((uint8_t*)data)[i] >= ' ' && ((uint8_t*)data)[i] <= '~') {
			ascii[i % DUMPHEX_WIDTH] = ((uint8_t*)data)[i];
		} else {
			ascii[i % DUMPHEX_WIDTH] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % DUMPHEX_WIDTH == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % DUMPHEX_WIDTH] = '\0';
				if ((i+1) % DUMPHEX_WIDTH <= 8) {
					printf(" ");
				}
				for (j = (i+1) % DUMPHEX_WIDTH; j < DUMPHEX_WIDTH; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}
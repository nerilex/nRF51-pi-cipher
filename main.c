
/**
* This example shows how to configure GPIO pins as outputs which can also be used to drive LEDs.
* Each LED is set on one at a time and each state lasts 100 milliseconds.
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hexdump.h"

#include "nrf.h"


#include <pi16cipher.h>
#include <pi32cipher.h>
#include <pi64cipher.h>

#define PSTR(x) x

/**
 * \brief dumps the contents of a buffer to the console
 */
void hexdump_block(const void* data, size_t length, uint8_t indent, uint8_t width){
    uint16_t i;
    uint8_t  j;
    for (i = 0; i < length; ++i){
        if (i % width == 0){
            putchar('\n');
            for(j = 0; j < indent; ++j){
                putchar(' ');
            }
        }
        printf("%02x ", *((uint8_t*)data));
        data = (uint8_t*)data +1;
    }
}

NRF_TIMER_Type *timer = (NRF_TIMER_Type *)NRF_TIMER0_BASE;

#define INIT_TIMER() do { \
        timer->TASKS_STOP = 1;                              \
        timer->BITMODE = TIMER_BITMODE_BITMODE_32Bit;       \
        timer->PRESCALER = 0;                               \
        timer->MODE = TIMER_MODE_MODE_Timer;                \
} while (0)

#define START_TIMER() do { \
        timer->TASKS_CLEAR = 1;                            \
        timer->TASKS_START = 1;                            \
} while (0)

#define STOP_TIMER() ( \
        timer->TASKS_STOP = 1,                             \
        timer->TASKS_CAPTURE[0] = 1,                       \
        timer->CC[0] )


#if 0
void aes_test(void) {
/*    uint8_t key[16] = { 0, 1, 2 , 3, 4, 5, 6, 7,
                        8, 9, 10, 11, 12, 13, 14, 15 };
    uint8_t plain[16] = {
            0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
            0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    };
    */
    uint8_t key[16] = {
            0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
            0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    uint8_t plain[16] = {
            0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
            0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
    };
    uint8_t cipher[16];
    aes128_ctx_t ctx;


    DUMP(key);
    DUMP(plain);
    aes128_init(key, &ctx);
    memcpy(cipher, plain, sizeof(cipher));
    timer->TASKS_CLEAR = 1;
    timer->TASKS_START = 1;
    aes128_enc(cipher, &ctx);
    timer->TASKS_STOP = 1;
    timer->TASKS_CAPTURE[0] = 1;
    DUMP(cipher);

    memcpy(plain, cipher, sizeof(plain));
    timer->TASKS_CLEAR = 1;
    timer->TASKS_START = 1;
    aes128_dec(plain, &ctx);
    timer->TASKS_STOP = 1;
    timer->TASKS_CAPTURE[1] = 1;
    DUMP(plain);

    printf("\tencrypt cycles: %d\n", timer->CC[0]);
    printf("\tdecrypt cycles: %d\n", timer->CC[1]);
}
#endif

void testrun_performance_pi16cipher(void){
    pi16_ctx_t ctx;
    uint32_t t;
    const uint8_t key[16] = { 15, 14, 13, 12 , 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
    uint8_t msg[19] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
    const uint8_t ad[17] = { 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34 };
    uint8_t nsec[16] = { 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t npub[4] = { 10, 11, 12, 13 };
    uint8_t crypt[16 + 19 + 16];
    uint8_t *tag = &crypt[16 + 19];
//    size_t crypt_len, tag_len, msg_len = sizeof(msg);
//    int v;
    INIT_TIMER();

    START_TIMER();
    pi16_init(&ctx, key, sizeof(key), npub, sizeof(npub));
    t = STOP_TIMER();
    printf(PSTR("\tinit time (16 + 4)           : %10"PRIu32"\n"), t);
    START_TIMER();
    pi16_process_ad_block(&ctx, ad, 1);
    t = STOP_TIMER();
    printf(PSTR("\tprocess ad(16)               : %10"PRIu32"\n"), t);
    START_TIMER();
    pi16_process_ad_last_block(&ctx, &ad[16], 1, 2);
    t = STOP_TIMER();
    printf(PSTR("\tprocess last ad(1)           : %10"PRIu32"\n"), t);
    START_TIMER();
    pi16_encrypt_smn(&ctx, crypt, nsec);
    t = STOP_TIMER();
    printf(PSTR("\tprocess smn(16)              : %10"PRIu32"\n"), t);
    START_TIMER();
    pi16_encrypt_block(&ctx, &crypt[16], msg, 1);
    t = STOP_TIMER();
    printf(PSTR("\tprocess encrypt block(16)    : %10"PRIu32"\n"), t);
    START_TIMER();
    pi16_encrypt_last_block(&ctx, &crypt[32], &msg[16], 3, 2);
    t = STOP_TIMER();
    printf(PSTR("\tprocess encrypt last block(3): %10"PRIu32"\n"), t);
    START_TIMER();
    pi16_extract_tag(&ctx, tag);
    t = STOP_TIMER();
    printf(PSTR("\tprocess extract tag(16)      : %10"PRIu32"\n"), t);
}

void testrun_performance_pi32cipher(void){
    pi32_ctx_t ctx;
    uint32_t t;
    const uint8_t key[16] = { 15, 14, 13, 12 , 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
    uint8_t msg[39] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
                        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
    const uint8_t ad[35] = { 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                             18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34 };
    uint8_t nsec[32] = { 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
                         0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00};
    const uint8_t npub[4] = { 10, 11, 12, 13 };
    uint8_t crypt[sizeof(nsec) + sizeof(msg) + 32];
    uint8_t *tag = &crypt[sizeof(crypt) - 32];
//    size_t crypt_len, tag_len, msg_len = sizeof(msg);
//    int v;
    INIT_TIMER();

    START_TIMER();
    pi32_init(&ctx, key, sizeof(key), npub, sizeof(npub));
    t = STOP_TIMER();
    printf(PSTR("\tinit time (16 + 4)           : %10"PRIu32"\n"), t);
    START_TIMER();
    pi32_process_ad_block(&ctx, ad, 1);
    t = STOP_TIMER();
    printf(PSTR("\tprocess ad(32)               : %10"PRIu32"\n"), t);
    START_TIMER();
    pi32_process_ad_last_block(&ctx, &ad[32], 3, 2);
    t = STOP_TIMER();
    printf(PSTR("\tprocess last ad(3)           : %10"PRIu32"\n"), t);
    START_TIMER();
    pi32_encrypt_smn(&ctx, crypt, nsec);
    t = STOP_TIMER();
    printf(PSTR("\tprocess smn(32)              : %10"PRIu32"\n"), t);
    START_TIMER();
    pi32_encrypt_block(&ctx, &crypt[32], msg, 1);
    t = STOP_TIMER();
    printf(PSTR("\tprocess encrypt block(32)    : %10"PRIu32"\n"), t);
    START_TIMER();
    pi32_encrypt_last_block(&ctx, &crypt[64], &msg[32], 7, 2);
    t = STOP_TIMER();
    printf(PSTR("\tprocess encrypt last block(7): %10"PRIu32"\n"), t);
    START_TIMER();
    pi32_extract_tag(&ctx, tag);
    t = STOP_TIMER();
    printf(PSTR("\tprocess extract tag(32)      : %10"PRIu32"\n"), t);
}


void testrun_pi16(void)
{
    const uint8_t key[16] = { 15, 14, 13, 12 , 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
    uint8_t msg[19] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
    const uint8_t ad[17] = { 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34 };
    uint8_t nsec[16] = { 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t npub[4] = { 10, 11, 12, 13 };
    uint8_t crypt[16 + 19 + 16];
    size_t crypt_len, msg_len = sizeof(msg);
    int v;
//    printf("crypt = %p, crypt_len = %d, tag = %p, tag_len = %d\n", crypt, crypt_len, tag, tag_len);
    pi16_encrypt_simple(crypt, &crypt_len, msg, sizeof(msg), ad, sizeof(ad), nsec, npub, sizeof(npub), key, sizeof(key));
//    printf("crypt = %p, crypt_len = %d, tag = %p, tag_len = %d\n", crypt, crypt_len, tag, tag_len);
    DUMP(key);
    DUMP(msg);
    DUMP(ad);
    DUMP(nsec);
    DUMP(npub);
    DUMP_LEN("crypt", crypt, crypt_len);
    puts("");
    crypt[0] ^= 0;
    v = pi16_decrypt_simple(msg, &msg_len, nsec, crypt, crypt_len, ad, sizeof(ad), npub, sizeof(npub), key, sizeof(key));
    DUMP(key);
    DUMP(msg);
    DUMP(ad);
    DUMP(nsec);
    DUMP(npub);
    DUMP_LEN("crypt", crypt, crypt_len);
    printf("\nverification: >> %s (%d) <<\n", v ? "FAILED!" : "ok", v);
    puts("");
}

void testrun_pi32(void)
{
    const uint8_t key[16] = { 15, 14, 13, 12 , 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
    uint8_t msg[39] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
                        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
    const uint8_t ad[35] = { 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                             18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34};
    uint8_t nsec[32] = { 0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
                         0xff, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t npub[16] = { 10, 11, 12, 13, 14, 15, 16, 17,
                               18, 19, 20, 21, 22, 23, 24, 25 };
    uint8_t crypt[sizeof(nsec) + sizeof(msg) + 32];
    size_t crypt_len, msg_len = sizeof(msg);
    int v;
//    printf("crypt = %p, crypt_len = %d, tag = %p, tag_len = %d\n", crypt, crypt_len, tag, tag_len);
    pi32_encrypt_simple(crypt, &crypt_len, msg, sizeof(msg), ad, sizeof(ad), nsec, npub, sizeof(npub), key, sizeof(key));
//    printf("crypt = %p, crypt_len = %d, tag = %p, tag_len = %d\n", crypt, crypt_len, tag, tag_len);
    DUMP(key);
    DUMP(msg);
    DUMP(ad);
    DUMP(nsec);
    DUMP(npub);
    DUMP_LEN("crypt", crypt, crypt_len);
    puts("");
    crypt[0] ^= 0;
    v = pi32_decrypt_simple(msg, &msg_len, nsec, crypt, crypt_len, ad, sizeof(ad), npub, sizeof(npub), key, sizeof(key));
    DUMP(key);
    DUMP(msg);
    DUMP(ad);
    DUMP(nsec);
    DUMP(npub);
    DUMP_LEN("crypt", crypt, crypt_len);
    printf("\nverification: >> %s (%d) <<\n", v ? "FAILED!" : "ok", v);
    puts("");
}

void testrun_pi64(void)
{
    const uint8_t key[16] = { 0 };
    const uint8_t msg[1] = { 0xf };
    const uint8_t ad[1] = { 0 };
    const uint8_t nsec[PI64_SMN_LENGTH_BYTES] = { 0 };
    const uint8_t npub[16] = { 0 };
    uint8_t crypt[sizeof(nsec) + sizeof(msg) + 64];
    size_t crypt_len;
    pi64_encrypt_simple(crypt, &crypt_len, msg, sizeof(msg), ad, sizeof(ad), nsec, npub, sizeof(npub), key, sizeof(key));
    DUMP(key);
    DUMP(msg);
    DUMP(ad);
    DUMP(nsec);
    DUMP(npub);
    DUMP(crypt);
}

void testrun(void) {
    testrun_pi16();
    testrun_pi32();
    testrun_pi64();
}


int main()
{

    testrun_pi32();
    testrun_performance_pi16cipher();
    testrun_performance_pi32cipher();
    _Exit(EXIT_SUCCESS);
    for (;;)
        ;

    return 0;
}

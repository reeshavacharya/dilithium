// bench_dilithium_ctx.c
// Benchmark for CRYSTALS-Dilithium (reference) using context-aware APIs.
//
// Compile (adjust paths/libs):
//   cc -O3 -g -Wall -Wextra -std=c11 \
//      bench_dilithium_ctx.c \
//      -I.. \
//      -o bench_dilithium_ctx
//
// (Optional debug) add: -fsanitize=address -fno-omit-frame-pointer

#define _POSIX_C_SOURCE 199309L
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../randombytes.h"
#include "../api.h"
#include "../params.h"

// Number of iterations per benchmark
#ifndef NTESTS
#define NTESTS 10000
#endif

#ifndef DILITHIUM_MODE
#warning "DILITHIUM_MODE not defined; defaulting to 2"
#define DILITHIUM_MODE 2
#endif

// Map symbols for the chosen mode (reference build)
#if DILITHIUM_MODE == 2
  #define crypto_sign_keypair   pqcrystals_dilithium2_ref_keypair
  // context-aware detached signature
  // int crypto_sign_signature(uint8_t *sig, size_t *siglen, const uint8_t *m, size_t mlen,
  //                           const uint8_t *ctx, size_t ctxlen, const uint8_t *sk);
  #define crypto_sign_signature pqcrystals_dilithium2_ref_signature
  // context-aware combined sign
  // int crypto_sign(uint8_t *sm, size_t *smlen, const uint8_t *m, size_t mlen,
  //                 const uint8_t *ctx, size_t ctxlen, const uint8_t *sk);
  #define crypto_sign           pqcrystals_dilithium2_ref
  // context-aware open
  // int crypto_sign_open(uint8_t *m, size_t *mlen, const uint8_t *sm, size_t smlen,
  //                      const uint8_t *ctx, size_t ctxlen, const uint8_t *pk);
  #define crypto_sign_open      pqcrystals_dilithium2_ref_open
#elif DILITHIUM_MODE == 3
  #define crypto_sign_keypair   pqcrystals_dilithium3_ref_keypair
  #define crypto_sign_signature pqcrystals_dilithium3_ref_signature
  #define crypto_sign           pqcrystals_dilithium3_ref
  #define crypto_sign_open      pqcrystals_dilithium3_ref_open
#elif DILITHIUM_MODE == 5
  #define crypto_sign_keypair   pqcrystals_dilithium5_ref_keypair
  #define crypto_sign_signature pqcrystals_dilithium5_ref_signature
  #define crypto_sign           pqcrystals_dilithium5_ref
  #define crypto_sign_open      pqcrystals_dilithium5_ref_open
#else
  #error "Unsupported DILITHIUM_MODE (expected 2, 3, or 5)"
#endif

static inline uint64_t get_time_ns(void) {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp); // stable for benchmarking
    return (uint64_t)tp.tv_sec * 1000000000ull + (uint64_t)tp.tv_nsec;
}

static void print_results(const char *desc, const uint64_t *times, size_t ntests) {
    uint64_t min = times[0], max = times[0], sum = times[0];
    for (size_t i = 1; i < ntests; i++) {
        if (times[i] < min) min = times[i];
        if (times[i] > max) max = times[i];
        sum += times[i];
    }
    double avg = (double)sum / (double)ntests;
    printf("\n%s:\n", desc);
    printf("  min: %.3f ms\n", (double)min / 1e6);
    printf("  max: %.3f ms\n", (double)max / 1e6);
    printf("  avg: %.3f ms\n", (double)avg / 1e6);
}

int main(void) {
    // Context string for context-aware APIs (adjust as you like)
    static const uint8_t ctx[] = "dilithium-bench";
    const size_t ctxlen = sizeof(ctx) - 1;

    printf("DILITHIUM_MODE=%d\n", DILITHIUM_MODE);
    printf("CRYPTO_PUBLICKEYBYTES=%zu, CRYPTO_SECRETKEYBYTES=%zu, CRYPTO_BYTES=%zu\n",
           (size_t)CRYPTO_PUBLICKEYBYTES,
           (size_t)CRYPTO_SECRETKEYBYTES,
           (size_t)CRYPTO_BYTES);
    printf("Context: \"%s\" (len=%zu)\n", (const char*)ctx, ctxlen);
    printf("Running %d iterations for each operation...\n", NTESTS);

    // Allocate once
    uint8_t *pk    = (uint8_t *)malloc(CRYPTO_PUBLICKEYBYTES);
    uint8_t *sk    = (uint8_t *)malloc(CRYPTO_SECRETKEYBYTES);
    uint8_t *msg   = (uint8_t *)malloc(100);
    uint8_t *sig   = (uint8_t *)malloc(CRYPTO_BYTES);
    uint8_t *sm    = (uint8_t *)malloc(CRYPTO_BYTES + 100);
    uint8_t *m2    = (uint8_t *)malloc(100);
    uint64_t *times = (uint64_t *)malloc(NTESTS * sizeof(uint64_t));

    if (!pk || !sk || !msg || !sig || !sm || !m2 || !times) {
        fprintf(stderr, "Allocation failure\n");
        free(pk); free(sk); free(msg); free(sig); free(sm); free(m2); free(times);
        return 1;
    }

    size_t mlen = 100;
    randombytes(msg, mlen);

    // -------- Key Generation --------
    for (int i = 0; i < 5; i++) (void)crypto_sign_keypair(pk, sk); // warm-up

    for (size_t i = 0; i < NTESTS; i++) {
        uint64_t t0 = get_time_ns();
        if (crypto_sign_keypair(pk, sk) != 0) {
            fprintf(stderr, "keypair() failed at iter %zu\n", i);
            goto cleanup;
        }
        uint64_t t1 = get_time_ns();
        times[i] = t1 - t0;
    }
    print_results("Key Generation", times, NTESTS);

    // Fresh keypair for signing/verifying
    if (crypto_sign_keypair(pk, sk) != 0) {
        fprintf(stderr, "keypair() failed (post-bench)\n");
        goto cleanup;
    }

    // -------- Signing (detached) --------
    size_t siglen = 0;
    (void)crypto_sign_signature(sig, &siglen, msg, mlen, ctx, ctxlen, sk); // warm-up

    for (size_t i = 0; i < NTESTS; i++) {
        uint64_t t0 = get_time_ns();
        if (crypto_sign_signature(sig, &siglen, msg, mlen, ctx, ctxlen, sk) != 0) {
            fprintf(stderr, "crypto_sign_signature() failed at iter %zu\n", i);
            goto cleanup;
        }
        uint64_t t1 = get_time_ns();
        times[i] = t1 - t0;
    }
    print_results("Signing (detached)", times, NTESTS);

    // -------- Combined Sign / Open --------
    size_t sm_len = 0;
    if (crypto_sign(sm, &sm_len, msg, mlen, ctx, ctxlen, sk) != 0) {
        fprintf(stderr, "crypto_sign() failed\n");
        goto cleanup;
    }

    // Warm-up open
    {
        size_t m2_len = 0;
        if (crypto_sign_open(m2, &m2_len, sm, sm_len, ctx, ctxlen, pk) != 0) {
            fprintf(stderr, "crypto_sign_open() warm-up failed\n");
            goto cleanup;
        }
    }

    for (size_t i = 0; i < NTESTS; i++) {
        size_t m2_len = 0;
        uint64_t t0 = get_time_ns();
        if (crypto_sign_open(m2, &m2_len, sm, sm_len, ctx, ctxlen, pk) != 0) {
            fprintf(stderr, "Verification failed at iter %zu\n", i);
            goto cleanup;
        }
        uint64_t t1 = get_time_ns();
        times[i] = t1 - t0;
    }
    print_results("Verification (open)", times, NTESTS);

cleanup:
    free(pk);
    free(sk);
    free(msg);
    free(sig);
    free(sm);
    free(m2);
    free(times);
    return 0;
}

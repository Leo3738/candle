#ifndef MACROS_H
#define MACROS_H

#define _GNU_SOURCE
#ifndef ARGNUM
#define ARGNUM(...) \
    _ARGNUM(_0, ##__VA_ARGS__, \
    62,61,60,                   \
    59,58,57,56,55,54,53,52,51,50, \
    49,48,47,46,45,44,43,42,41,40, \
    39,38,37,36,35,34,33,32,31,30, \
    29,28,27,26,25,24,23,22,21,20, \
    19,18,17,16,15,14,13,12,11,10, \
     9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define _ARGNUM( \
     _0, _1, _2, _3, _4, _5, _6, _7, _8,_9, \
    _10,_11,_12,_13,_14,_15,_16,_17,_18,_19, \
    _20,_21,_22,_23,_24,_25,_26,_27,_28,_29, \
    _30,_31,_32,_33,_34,_35,_36,_37,_38,_39, \
    _40,_41,_42,_43,_44,_45,_46,_47,_48,_49, \
    _50,_51,_52,_53,_54,_55,_56,_57,_58,_59, \
    _60,_61,_62, N, ...) N


#endif

#define ECMHASH(str) murmur_hash(str, strlen(str), 0)
#define sig(str) murmur_hash(str, strlen(str), 0)
#define ref(str) murmur_hash(str, strlen(str), 0)
#define TP2(a, b) a##b
#define TP(a, b) TP2(a, b)

typedef unsigned int uint;
__attribute__((always_inline))
__attribute__((optimize("unroll-loops")))
static inline uint murmur_hash(const void *key, int len, uint seed)
{
    /* 32-Bit MurmurHash3: https://code.google.com/p/smhasher/wiki/MurmurHash3*/
    #define ROTL(x,r) ((x) << (r) | ((x) >> (32 - r)))
    union {const uint *i; const char *b;} conv = {0};
    const char *data = (const char*)key;
    const int nblocks = len/4;
    uint h1 = seed;
    const uint c1 = 0xcc9e2d51;
    const uint c2 = 0x1b873593;
    const char *tail;
    const uint *blocks;
    uint k1;
    int i;

    /* body */
    if (!key) return 0;
    conv.b = (data + nblocks*4);
    blocks = (const uint*)conv.i;
    for (i = -nblocks; i; ++i) {
        k1 = blocks[i];
        k1 *= c1;
        k1 = ROTL(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1 = ROTL(h1,13);
        h1 = h1*5+0xe6546b64;
    }

    /* tail */
    tail = (const char*)(data + nblocks*4);
    k1 = 0;
    switch (len & 3) {
    case 3: k1 ^= (uint)(tail[2] << 16); /* fallthrough */
    case 2: k1 ^= (uint)(tail[1] << 8u); /* fallthrough */
    case 1: k1 ^= tail[0];
            k1 *= c1;
            k1 = ROTL(k1,15);
            k1 *= c2;
            h1 ^= k1;
            break;
    default: break;
    }

    /* finalization */
    h1 ^= (uint)len;
    /* fmix32 */
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    #undef ROTL
    return h1;
}


#endif /* !MACROS_H */

//
// Created by alexey on 30.09.2019.
//

#include "MurMurHash3.h"

inline unsigned int rotl32(unsigned int x, unsigned char r) {
    return (x << r) | (x >> 32 - r);
}

inline unsigned int getblock32(const unsigned int *p, int i) {
    return p[i];
}

inline unsigned int finalmix32(unsigned long long h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

unsigned long MurMurHash3_32(const void *key, int len, unsigned int seed) {
    auto *data = (const unsigned char *) key;
    const int nblocks = len / 4;

    unsigned long h1 = seed;

    const unsigned int c1 = 0xcc9e2d51;
    const unsigned int c2 = 0x1b873593;

    auto *blocks = (const unsigned int *) (data + nblocks * 4);

    for (int i = -nblocks; i; i++) {
        unsigned int k1 = getblock32(blocks, i);

        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl32(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }

    const char *tail = (const char *) (data + nblocks * 4);

    unsigned long k1 = 0;

    switch (len & 3) {
        case 3:
            k1 ^= tail[2] << 16;
        case 2:
            k1 ^= tail[1] << 8;
        case 1:
            k1 ^= tail[0];
            k1 *= c1;
            k1 = rotl32(k1, 15);
            k1 *= c2;
            h1 ^= k1;
    };

    h1 ^= len;
    h1 = finalmix32(h1);
    return h1;
}
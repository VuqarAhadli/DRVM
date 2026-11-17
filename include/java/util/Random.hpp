#include <bit>
#include <stdexcept>
#include <Common.hpp>
#include <Types.hpp>

/**
 * This implemenation is based on Oracle's documentation
 * Source:
 * https://docs.oracle.com/javase/8/docs/api/java/util/Random.html#Random--
 */

/// Random number generator: (seed ^ 0x5DEECE66DL) & ((1L << 48) - 1)

class JavaRandom {
private:
    U8 seed;

    static constexpr U8 mult = 0x5DEECE66DULL;   // (U8)25214903917ULL
    static constexpr U8 add = 0xBULL;            // 11
    static constexpr U8 mask = (1ULL << 48) - 1; // 281474976710655ULL

    U4 next(int bits){}

public:
    explicit JavaRandom(S8 initialSeed){}

    void setSeed(S8 initialSeed){}

    S4 nextInt(){}

    S4 nextInt(S4 bound){}
};

/**
 * TODO:
 * Implement all Random class functions
 */
#include <java/util/Random.hpp>
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

    U4 next(int bits)
    {
        seed = (seed * mult + add) & mask;
        return static_cast<U4>(seed >> (48 - bits));
    }

public:
    explicit JavaRandom(S8 initialSeed) 
    {
        setSeed(initialSeed);
    }

    void setSeed(S8 initialSeed) 
    {
        seed = (static_cast<U8>(initialSeed) ^ mult) & mask;
    }

    S4 nextInt()
    {
        return std::bit_cast<S4>(next(32));
    }

    S4 nextInt(S4 bound)
     {
        if (bound <= 0) {
            throw std::runtime_error("IllegalArgumentException: bound must be positive");
        }

        U4 r = next(31);

        // Optimised when bound is a power of two.
        if ((bound & -bound) == bound)
        {
            return static_cast<S4>((static_cast<U8>(bound) * r) >> 31);
        }

        S4 bits, val;

        do 
        {
            bits = static_cast<S4>(r);
            val = bits % bound;
            r = next(31);
        } 
        while (bits - val + (bound - 1) < 0);

        return val;
    }
};

/**
 * TODO:
 * Implement all Random class functions
 */
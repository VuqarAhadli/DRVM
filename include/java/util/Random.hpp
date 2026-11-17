/*
 * DRVM (drvm)
 * Copyright (C) 2026 Vugar Ahadli
 * Contact: vuqarahadli17@gmail.com | vuqar@div.edu.az
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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

    U4 next(int bits);

public:
    explicit JavaRandom(S8 initialSeed);
    JavaRandom();
    void setSeed(S8 initialSeed);

    S4 nextInt();

    S4 nextInt(S4 bound);
};

/**
 * TODO:
 * Implement all Random class functions
 */
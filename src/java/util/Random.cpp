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

#include <java/util/Random.hpp>
/**
 * This implemenation is based on Oracle's documentation
 * Source:
 * https://docs.oracle.com/javase/8/docs/api/java/util/Random.html#Random--
 */

/// Random number generator: (seed ^ 0x5DEECE66DL) & ((1L << 48) - 1)


U4 JavaRandom::next(int bits)
{
	seed = (seed * mult + add) & mask;
	return static_cast<U4>(seed >> (48 - bits));
}

JavaRandom::JavaRandom(S8 initialSeed)
{
	setSeed(initialSeed);
}

JavaRandom::JavaRandom()
{
    setSeed(static_cast<S8>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    ));
}

void JavaRandom::setSeed(S8 initialSeed)
{
	seed = (static_cast<U8>(initialSeed) ^ mult) & mask;
}

S4 JavaRandom::nextInt()
{
	return std::bit_cast<S4>(next(32));
}

S4 JavaRandom::nextInt(S4 bound)
{
	if (bound <= 0)
	{
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
	}
	while (bits - val + (bound - 1) < 0 && (r = next(31), true));

	return val;
}


/**
 * TODO:
 * Implement all Random class functions
 */
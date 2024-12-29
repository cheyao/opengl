#include "components/noise.hpp"

#include "utils.hpp"

#include <SDL3/SDL.h>

NoiseGenerator::NoiseGenerator()
	: NoiseGenerator((static_cast<decltype(mSeed)>(SDL_rand_bits()) << sizeof(Sint32) ^
			  static_cast<decltype(mSeed)>(SDL_rand_bits())) &
			 0x7FFFFFFFFFFFFFFF) {}

NoiseGenerator::NoiseGenerator(const std::uint64_t seed) : mSeed(seed) {}

double NoiseGenerator::getNoise(std::int64_t x) const {
	x *= -1 * (x < 0);
	x ^= mSeed;
	const double d = static_cast<double>(x); // Oh well...

	return 0.4 * (1.0 * SDL_sin(0.2 * d)		// Random 1
		      - 0.0 * SDL_sin(-0.9 * EULER * d) // Random 2
		      + 0.9 * SDL_sin(0.1 * PI * d));	// Ransom 3
	//
	// We are only using 2 degrees of randomness
}

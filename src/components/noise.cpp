#include "components/noise.hpp"

#include "utils.hpp"

#include <SDL3/SDL.h>

NoiseGenerator::NoiseGenerator()
	: NoiseGenerator(static_cast<decltype(mSeed)>(SDL_rand_bits()) << sizeof(Sint32) ^
			 static_cast<decltype(mSeed)>(SDL_rand_bits())) {}

NoiseGenerator::NoiseGenerator(const std::uint64_t seed) : mSeed(seed) {}

double NoiseGenerator::getNoise(std::uint64_t x) const {
	x ^= mSeed; // Oh well...

	return 0.4 * (1.0 * SDL_sin(0.2 * x) - 0.0 * SDL_sin(-0.9 * EULER * x) + 0.9 * SDL_sin(0.1 * PI * x));
}

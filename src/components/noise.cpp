#include "components/noise.hpp"

#include "utils.hpp"

#include <SDL3/SDL.h>
#include <random>

NoiseGenerator::NoiseGenerator()
	: NoiseGenerator((static_cast<decltype(mSeed)>(SDL_rand_bits()) << sizeof(Sint32) ^
			  static_cast<decltype(mSeed)>(SDL_rand_bits())) &
			 0x7FFFFFFFFFFFFFFF) {}

NoiseGenerator::NoiseGenerator(const std::uint64_t seed) : mSeed(seed), mRng(seed) {}

double NoiseGenerator::getNoise(std::int64_t x) const {
	// Remove the sign -> unsigned x
	const auto ux = x * ((x < 0) ? -1 : 1);
	// Add the seed -> random x
	const auto rx = ux ^ mSeed;
	// Cast to double -> double x
	const double dx = static_cast<double>(rx); // Oh well...

	const double noise = 0.4 * (1.0 * SDL_sin(0.2 * dx)		 // Random 1
				    + 0.9 * SDL_sin(0.1 * PI * dx)	 // Ransom 3
				    - 0.0 * SDL_sin(-0.9 * EULER * dx)); // Random 2
									 // We are only using 2 degrees of randomness

	return noise;
}

float NoiseGenerator::randf() {
	static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	return distribution(mRng);
}

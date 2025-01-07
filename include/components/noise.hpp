#pragma once

#include <cstdint>
#include <random>

// Basic 1D noise
class NoiseGenerator {
      public:
	NoiseGenerator();
	NoiseGenerator(std::uint64_t seed);

	NoiseGenerator(NoiseGenerator&&) = default;
	NoiseGenerator(const NoiseGenerator&) = default;
	NoiseGenerator& operator=(NoiseGenerator&&) = default;
	NoiseGenerator& operator=(const NoiseGenerator&) = default;
	~NoiseGenerator() = default;

	double getNoise(std::int64_t x) const;
	// Generates a random double between 0.0f and 1.0f
	float randf();
	std::uint64_t getSeed() const { return mSeed; }
	void setSeed(std::uint64_t seed) { mSeed = seed; }

      private:
	std::uint64_t mSeed;
	std::mt19937_64 mRng;
};

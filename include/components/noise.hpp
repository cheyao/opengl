#pragma once

#include <cstdint>

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

      private:
	std::uint64_t mSeed;
};

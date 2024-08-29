#pragma once

#include <SDL3/SDL.h>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <cstddef>

class GameIOStream : public Assimp::IOStream {
	friend class GameIOSystem;

      public:
	~GameIOStream();
	size_t Read(void* pvBuffer, std::size_t pSize, std::size_t pCount) override;
	size_t Write(const void* pvBuffer, std::size_t pSize, std::size_t pCount) override;
	aiReturn Seek(std::size_t pOffset, aiOrigin pOrigin) override;
	size_t Tell() const override;
	size_t FileSize() const override;
	void Flush() override;

      protected:
	GameIOStream(const char* pFile, const char* pMode = "rb");

      private:
	SDL_IOStream* mIO;
};

class GameIOSystem : public Assimp::IOSystem {
      public:
	GameIOSystem();
	~GameIOSystem();

	bool Exists(const char* pFile) const override;
	char getOsSeparator() const override;

	Assimp::IOStream* Open(const char* pFile, const char* pMode = "rb") override;
	void Close(Assimp::IOStream* pFile) override;
};

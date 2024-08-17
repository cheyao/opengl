#include "managers/IOManager.hpp"

#include "utils.hpp"

#include <SDL3/SDL.h>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <cstdint>
#include <stdexcept>

GameIOStream::GameIOStream(const char* pFile, const char* pMode) {
	mIO = SDL_IOFromFile(pFile, pMode);

	if (mIO == nullptr) {
		throw std::runtime_error("Assimp IO Open failed");
	}
}

GameIOStream::~GameIOStream() { SDL_CloseIO(mIO); }

size_t GameIOStream::Read(void* pvBuffer, size_t pSize, size_t pCount) {
	for (size_t i = 0; i < pCount; ++i) {
		const size_t status = SDL_ReadIO(mIO, static_cast<char*>(pvBuffer) + i * pSize, pSize);

		if (status == 0) {
			// SDL_Log("Read error: %s", SDL_GetError());

			return status;
		}
	}

	return pCount;
}

size_t GameIOStream::Write(const void* pvBuffer, size_t pSize, size_t pCount) {
	for (size_t i = 0; i < pCount; ++i) {
		const size_t status = SDL_WriteIO(mIO, static_cast<const char*>(pvBuffer) + i * pSize, pSize);

		if (status < pSize) {
			SDL_Log("Read write: %s", SDL_GetError());

			return status;
		}
	}

	return pCount;
}

aiReturn GameIOStream::Seek(size_t pOffset, aiOrigin pOrigin) {
	SDL_IOWhence whence;

	switch (pOrigin) {
		case aiOrigin_CUR:
			whence = SDL_IO_SEEK_CUR;
			break;
		case aiOrigin_SET:
			whence = SDL_IO_SEEK_SET;
			break;
		case aiOrigin_END:
			whence = SDL_IO_SEEK_END;
			break;
		default:
			return aiReturn_FAILURE;
	}

	int64_t status = SDL_SeekIO(mIO, pOffset, whence);

	[[unlikely]] if (status == -1) {
		return aiReturn_FAILURE;
	} else [[unlikely]] if (status < 0) {
		SDL_Log("Seek error %s", SDL_GetError());

		return aiReturn_OUTOFMEMORY;
	} else [[likely]] {
		return aiReturn_SUCCESS;
	}
}

size_t GameIOStream::Tell() const { return SDL_TellIO(mIO); }

size_t GameIOStream::FileSize() const { return SDL_GetIOSize(mIO); }

void GameIOStream::Flush() { /* NOTE: SDL probably doesn't need flush */ }

GameIOSystem::GameIOSystem() {}

GameIOSystem::~GameIOSystem() {}

bool GameIOSystem::Exists(const char* pFile) const {
#ifndef ANDROID
	int stat = SDL_GetPathInfo(pFile, nullptr);

	if (stat == 0) {
		return true;
	} else {
		// SDL_Log("File %s doesn't exist: %s", pFile, SDL_GetError());

		return false;
	}
#else
	// TODO: Somehow make better
	SDL_IOStream* file = SDL_IOFromFile(pFile, "r+b");
	if (file == nullptr) {
		// SDL_Log("File %s doesn't exist: %s", pFile, SDL_GetError());

		return false;
	} else {
		SDL_CloseIO(file);

		return true;
	}
#endif
}

char GameIOSystem::getOsSeparator() const { return SEPARATOR[0]; }

Assimp::IOStream* GameIOSystem::Open(const char* pFile, const char* pMode) {
	try {
		return new GameIOStream(pFile, pMode);
	} catch (const std::runtime_error& e) {
		// SDL_Log("GameIO Error: %s: %s", e.what(), SDL_GetError());

		return nullptr;
	}
}

void GameIOSystem::Close(Assimp::IOStream* pFile) {
	GameIOStream* IO = dynamic_cast<GameIOStream*>(pFile);

	if (IO == nullptr) {
		throw std::runtime_error("Assimp IO Close dynamic cast failed");
	}

	delete IO;
}

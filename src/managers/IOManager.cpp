#include "managers/IOManager.hpp"

#include <SDL3/SDL.h>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

GameIOStream::GameIOStream(const char* pFile, const char* pMode) {
	mIO = SDL_IOFromFile(pFile, pMode);

	if (mIO == nullptr) {
		throw std::runtime_error("Assimp IO Open failed");
	}
}

GameIOStream::~GameIOStream() { SDL_CloseIO(mIO); }

size_t GameIOStream::Read(void* pvBuffer, const std::size_t pSize, const std::size_t pCount) {
	for (std::size_t i = 0; i < pCount; ++i) {
		const std::size_t status = SDL_ReadIO(mIO, static_cast<char*>(pvBuffer) + i * pSize, pSize);

		if (status == 0) {
			return status;
		}
	}

	return pCount;
}

size_t GameIOStream::Write(const void* pvBuffer, const std::size_t pSize, const std::size_t pCount) {
	for (size_t i = 0; i < pCount; ++i) {
		const size_t status = SDL_WriteIO(mIO, static_cast<const char*>(pvBuffer) + i * pSize, pSize);

		if (status < pSize) {
			return status;
		}
	}

	return pCount;
}

aiReturn GameIOStream::Seek(const std::size_t pOffset, const aiOrigin pOrigin) {
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
		[[unlikely]] default:
			return aiReturn_FAILURE;
	}

	int64_t status = SDL_SeekIO(mIO, pOffset, whence);

	[[unlikely]] if (status == -1) {
		return aiReturn_FAILURE;
	} else [[unlikely]] if (status < 0) {
		SDL_Log("Seek error %s", SDL_GetError());

		return aiReturn_OUTOFMEMORY;
	}

	return aiReturn_SUCCESS;
}

size_t GameIOStream::Tell() const { return SDL_TellIO(mIO); }

size_t GameIOStream::FileSize() const { return SDL_GetIOSize(mIO); }

void GameIOStream::Flush() { SDL_FlushIO(mIO); }

GameIOSystem::GameIOSystem() {}

GameIOSystem::~GameIOSystem() {}

bool GameIOSystem::Exists(const char* pFile) const {
#ifdef ANDROID
	// Somehow get path info doesn't work with android
	SDL_IOStream* file = SDL_IOFromFile(pFile, "r+b");
	if (file == nullptr) {
		return false;
	}

	SDL_CloseIO(file);
#else
	if (!SDL_GetPathInfo(pFile, nullptr)) {
		return false;
	}
#endif

	return true;
}

char GameIOSystem::getOsSeparator() const { return '/'; }

Assimp::IOStream* GameIOSystem::Open(const char* pFile, const char* pMode) {
	try {
		return new GameIOStream(pFile, pMode);
	} catch (const std::runtime_error& e) {
		return nullptr;
	}
}

void GameIOSystem::Close(Assimp::IOStream* pFile) {
	delete static_cast<GameIOStream*>(pFile);
}

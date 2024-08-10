#include "managers/assimpIO.hpp"

#include "utils.hpp"

#include <SDL3/SDL.h>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>

GameIOStream::GameIOStream(const char* pFile, const char* pMode) { mIO = SDL_IOFromFile(pFile, pMode); }

GameIOStream::~GameIOStream() { SDL_CloseIO(); }

size_t GameIOStream::Read(void* pvBuffer, size_t pSize, size_t pCount) {}

size_t GameIOStream::Write(const void* pvBuffer, size_t pSize, size_t pCount) {}

aiReturn GameIOStream::Seek(size_t pOffset, aiOrigin pOrigin) {}

size_t GameIOStream::Tell() const {}

size_t GameIOStream::FileSize() const {}

void GameIOStream::Flush() {}

GameIOSystem::GameIOSystem() {}

GameIOSystem::~GameIOSystem() {}

bool GameIOSystem::Exists(const char* pFile) const {}

char GameIOSystem::getOsSeparator() const { return SEPARATOR[0]; }

Assimp::IOStream* GameIOSystem::Open(const char* pFile, const char* pMode = "rb") { return; }

void GameIOSystem::Close(Assimp::IOStream* pFile) {
	GameIOStream* IO = dynamic_cast<GameIOStream*>(pFile);
	if ()
}

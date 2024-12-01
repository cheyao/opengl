#include "utils.hpp"

#include "third_party/Eigen/Core"
#include "third_party/json.hpp"

#include <SDL3/SDL.h>

void* loadFile(const char* file, size_t* datasize) { return SDL_LoadFile(file, datasize); }

namespace Eigen {
void to_json(nlohmann::json& j, const Vector3i& vec) { j = {{"X", vec.x()}, {"Y", vec.y()}, {"Z", vec.z()}}; }

void from_json(const nlohmann::json& j, Vector3i& vec) {
	j.at("X").get_to(vec.x());
	j.at("Y").get_to(vec.y());
	j.at("Z").get_to(vec.z());
}

void to_json(nlohmann::json& j, const Vector2f& vec) { j = {{"X", vec.x()}, {"Y", vec.y()}}; }

void from_json(const nlohmann::json& j, Vector2f& vec) {
	j.at("X").get_to(vec.x());
	j.at("Y").get_to(vec.y());
}

void to_json(nlohmann::json& j, const Vector2i& vec) { j = {{"X", vec.x()}, {"Y", vec.y()}}; }

void from_json(const nlohmann::json& j, Vector2i& vec) {
	j.at("X").get_to(vec.x());
	j.at("Y").get_to(vec.y());
}
} // namespace Eigen

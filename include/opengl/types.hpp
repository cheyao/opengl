#pragma once

#include "third_party/Eigen/Core"

struct Vertex {
	Eigen::Vector3f position;
	Eigen::Vector3f normal;
	Eigen::Vector2f texturePos;
};

typedef enum TextueType { DIFFUSE, SPECULAR, HEIGHT, AMBIENT } TextureType;

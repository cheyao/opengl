#pragma once

class VertexArray {
  public:
	VertexArray(const float* vertices, unsigned int numVertices, const unsigned int* indices,
				unsigned int numIndices);
	VertexArray(VertexArray&&) = delete;
	VertexArray(const VertexArray&) = delete;
	VertexArray& operator=(VertexArray&&) = delete;
	VertexArray& operator=(const VertexArray&) = delete;
	~VertexArray();

	void activate() const;

	unsigned int indices() const { return mIndices; }
	unsigned int vertices() const { return mVertices; }

  private:
	unsigned int mVertices;
	unsigned int mIndices;

	unsigned int mVBO;
	unsigned int mEBO;
	unsigned int mVAO;
};

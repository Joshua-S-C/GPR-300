/*
*	Author: Eric Winebrenner
*/

#include "model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <assimp/scene.h>
#include <glm/glm.hpp>

namespace ew {
	ew::Mesh processAiMesh(aiMesh* aiMesh);

	Model::Model(const std::string& filePath)
	{
		Assimp::Importer importer;
		const aiScene* aiScene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
		for (size_t i = 0; i < aiScene->mNumMeshes; i++)
		{
			aiMesh* aiMesh = aiScene->mMeshes[i];
			m_meshes.push_back(processAiMesh(aiMesh));
		}
	}

	void Model::draw()
	{
		for (size_t i = 0; i < m_meshes.size(); i++)
		{
			m_meshes[i].draw();
		}
	}

	glm::vec3 convertAIVec3(const aiVector3D& v) {
		return glm::vec3(v.x, v.y, v.z);
	}

	//Utility functions local to this file
	ew::Mesh processAiMesh(aiMesh* aiMesh) {
		ew::MeshData meshData;
		for (size_t i = 0; i < aiMesh->mNumVertices; i++)
		{
			ew::Vertex vertex;
			vertex.pos = convertAIVec3(aiMesh->mVertices[i]);
			if (aiMesh->HasNormals()) {
				vertex.normal = convertAIVec3(aiMesh->mNormals[i]);
			}
			if (aiMesh->HasTextureCoords(0)) {
				vertex.uv = glm::vec2(convertAIVec3(aiMesh->mTextureCoords[0][i]));
			}
			meshData.vertices.push_back(vertex);
		}
		//Convert faces to indices
		for (size_t i = 0; i < aiMesh->mNumFaces; i++)
		{
			for (size_t j = 0; j < aiMesh->mFaces[i].mNumIndices; j++)
			{
				meshData.indices.push_back(aiMesh->mFaces[i].mIndices[j]);
			}
		}
		return ew::Mesh(meshData);
	}

}

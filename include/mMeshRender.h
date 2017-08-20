#pragma once
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mShader.h"

class MeshEntry {
public:
    enum BUFFERS {
        VERTEX_BUFFER, TEXCOORD_BUFFER, NORMAL_BUFFER,INDEX_BUFFER
    };
    GLuint VAO;
    GLuint VBO[4];
    unsigned int elmNum;

    MeshEntry(aiMesh * mesh, GLuint vao);
    ~MeshEntry();

    void render();
};

class mMeshRender {
public:

    std::vector<MeshEntry *> meshEntries;


    mMeshRender(glm::mat4 &view, glm::mat4 &proj, mShader * shader);
    ~mMeshRender();
    void addMesh(std::string mesh_path);
    void render(std::vector<float> vertexs, std::vector<unsigned int> indices, glm::mat4  &model);
private:
    GLuint VAO;
    glm::mat4 VP; // VP”≥…‰æÿ’Û
    mShader * shader;
};

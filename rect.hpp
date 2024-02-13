#pragma once

#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"

class Rect {
public:
    float fillWidth;
    float fillHeight;
    glm::vec2 center;
    glm::ivec3 fillColor = glm::ivec3(0, 0, 0); // 0-255 inclusive.
    glm::ivec3 strokeColor = glm::ivec3(0, 0, 0); // 0-255 inclusive.
    float strokeThickness = 0.0f; // Increasing thickness goes outwards from center. Negative stroke will not be rendered.
    
    // Designed under the assumption that a circle will always have an opaque fill color.
    Rect(float fillWidth, float fillHeight, glm::vec2 center) {
        this->fillWidth = fillWidth;
        this->fillHeight = fillHeight;
        this->center = center;

        genVertices();
        initOpenGL();
    }

    void draw() {
        // MVP matrix defintions
        // ---------------------
        // Model
        glm::mat4 model;
        glm::mat4 strokeScale(1.0f);
        glm::mat4 translate(1.0f);
        strokeScale = glm::scale(strokeScale, glm::vec3(this->fillWidth + this->strokeThickness, this->fillHeight + this->strokeThickness, 0.0f));
        translate = glm::translate(translate, glm::vec3(this->center.x, this->center.y, 0.0f));
        model = translate * strokeScale;
        // View
        glm::mat4 view = glm::mat4(1.0f);
        // Projection
        glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);

        m_shader->use();

        // Bind shader uniforms
        m_shader->setMat4("model", model);
        m_shader->setMat4("view", view);
        m_shader->setMat4("projection", projection);
        m_shader->setVec3i("color", this->strokeColor);

        glBindVertexArray(m_vao);
        if (strokeThickness > 0.0f) glDrawElements(GL_TRIANGLES, m_indicesCount, GL_UNSIGNED_INT, 0); // Optimisation: Only draw extra circle if needed

        glm::mat4 fillScale(1.0f);
        fillScale = glm::scale(fillScale, glm::vec3(this->fillWidth, this->fillHeight, 0.0f));
        model = translate * fillScale;
        m_shader->setMat4("model", model);
        m_shader->setVec3i("color", this->fillColor);
        glDrawElements(GL_TRIANGLES, m_indicesCount, GL_UNSIGNED_INT, 0);
    } 

    /* 
     * WARNING:
     * Rect object must be instanstiated dynamically and cleaned up before the glfwTerminate() call. 
     * Static instanstiations will result in destrcutor being called after glfwTerminate() which results in a segfault
     * because the OpenGL context no longer exists. 
     */ 
    ~Rect() {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ebo);
        delete m_shader;
    }
private:
    const int m_verticesCount = 8; // 4 points each with xy coords.
    const int m_indicesCount = 6; // 2 triangles, each with 3 points. 
    float* m_vertices = new float[m_verticesCount];
    unsigned int* m_indices = new unsigned int[m_indicesCount];

    unsigned int m_vao, m_vbo, m_ebo;
    Shader* m_shader = new Shader("shape.vs", "shape.fs");

    void genVertices() {
        // Vertices
        // --------
        // Top left
        m_vertices[0] = -0.5f;
        m_vertices[1] = 0.5f;
        // Bottom left
        m_vertices[2] = -0.5f; 
        m_vertices[3] = -0.5f;
        // Bottom right
        m_vertices[4] = 0.5f; 
        m_vertices[5] = -0.5f;
        // Top Right
        m_vertices[6] = 0.5f; 
        m_vertices[7] = 0.5f;

        // Indices
        // -------
        // Triangle 1
        m_indices[0] = 0;
        m_indices[1] = 1;
        m_indices[2] = 2;
        // Triangle 2
        m_indices[3] = 0;
        m_indices[4] = 2;
        m_indices[5] = 3;
    }

    void initOpenGL() {
        // Create buffers
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        // Bind buffers 
        // ------------
        // VAO
        glBindVertexArray(m_vao);
        // VBO
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_verticesCount * sizeof(float), m_vertices, GL_STATIC_DRAW);
        // EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indicesCount * sizeof(unsigned int), m_indices, GL_STATIC_DRAW);

        // Original arrays already copied to GPU buffer so are no longer needed.
        delete[] m_vertices;
        delete[] m_indices;

        // VAO positions pointer
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); // Each vertex stores xy coorindates.
        glEnableVertexAttribArray(0);
    } 

};

#ifndef SMOKE_HPP
#define SMOKE_HPP

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include "Shader.hpp" // Make sure this points to your gps::Shader class

struct SmokeParticle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;      // 1.0 = born, 0.0 = dead
    float scale;
};

class Smoke {
private:
    std::vector<SmokeParticle> particles;
    GLuint VAO, VBO;

    glm::vec3 emitterPos;
    int maxParticles;

    // Buffer for sending data to GPU (x, y, z, life)
    std::vector<float> renderBuffer;

public:
    Smoke(glm::vec3 position, int count) {
        emitterPos = position;
        maxParticles = count;
        particles.resize(maxParticles);

        // Initialize particles as "dead" (-1.0) so they spawn naturally one by one
        for (auto& p : particles) {
            p.life = -1.0f;
        }

        InitOpenGL();
    }

    ~Smoke() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    void Update(float deltaTime) {
        renderBuffer.clear();

        for (auto& p : particles) {
            p.life -= deltaTime * 0.5f; // Dying speed (Lower = lasts longer)

            if (p.life > 0.0f) {
                // Rise Up + Wind Drift logic
                p.velocity.y += deltaTime * 0.2f; // Accelerate up
                p.position += p.velocity * deltaTime;

                // Add to render buffer
                renderBuffer.push_back(p.position.x);
                renderBuffer.push_back(p.position.y);
                renderBuffer.push_back(p.position.z);
                renderBuffer.push_back(p.life);
            }
            else {
                // Respawn Logic
                p.life = ((rand() % 100) / 100.0f); // Random start life to desync them
                p.position = emitterPos;

                // Random drift velocity
                float randX = ((rand() % 100) / 100.0f - 0.5f) * 9.0f;
                float randZ = ((rand() % 100) / 100.0f - 0.5f) * 9.0f;
                p.velocity = glm::vec3(randX, 3.0f, randZ); // 1.5f = Upward speed
            }
        }

        // Upload new positions to GPU
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, renderBuffer.size() * sizeof(float), renderBuffer.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Accepts the Shader passed from Main
    void Render(gps::Shader& shader) {
        if (renderBuffer.empty()) return;

        glBindVertexArray(VAO);

        // Enable modifying point size in vertex shader
        glEnable(GL_PROGRAM_POINT_SIZE);

        // Draw the points
        // We divide size by 4 because each vertex has 4 floats (x,y,z,life)
        glDrawArrays(GL_POINTS, 0, renderBuffer.size() / 4);

        glBindVertexArray(0);
    }

private:
    void InitOpenGL() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // Attribute 0: Position (vec3) + Life (float) = vec4
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
};

#endif
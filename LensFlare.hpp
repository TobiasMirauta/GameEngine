#ifndef LENSFLARE_HPP
#define LENSFLARE_HPP

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.hpp"

class LensFlare {
private:
    GLuint VAO, VBO;
    gps::Shader flareShader;
    std::vector<GLuint> textures;

    // Spacing configuration for the 8 flares along the line
    // 0.0 = At the moon, 1.0 = At screen center, > 1.0 = Opposite side
    std::vector<float> spacing = { 0.0f, 0.2f, 0.5f, 0.9f, 1.0f, 1.4f, 2.0f, 2.4f };
    std::vector<float> scales = { 0.5f, 0.1f, 0.15f, 0.3f, 0.1f, 0.2f, 0.5f, 0.6f };

public:
    LensFlare() {
        InitOpenGL();
        InitShader();
    }

    // Call this to load your 8 images
    void AddTexture(const char* path) {
        textures.push_back(LoadTexture(path));
    }

    void Render(glm::vec3 lightPosWorld, glm::mat4 view, glm::mat4 projection) {
        if (textures.empty()) return;

        // 1. Calculate Sun/Moon position in Clip Space
        glm::vec4 clipPos = projection * view * glm::vec4(lightPosWorld, 1.0f);

        // 2. Perform Perspective Division to get NDC [-1, 1]
        glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

        // 3. Check visibility
        // If z > 1.0, it's behind the far plane. If w < 0, it's behind the camera.
        // We also check if x/y are roughly on screen [-1.5 to 1.5] allowing slight off-screen bleed
        if (clipPos.w <= 0.0f || ndcPos.z > 1.0f) return;

        // 4. Calculate Intensity based on distance to center (0,0)
        // Distance from center of screen
        glm::vec2 lightPos2D = glm::vec2(ndcPos.x, ndcPos.y);
        float distToCenter = glm::length(lightPos2D);

        // Brighter when close to center, fade out as it leaves screen
        // 1.0 - dist means: Center = 1.0 brightness, Edge = 0.0 brightness
        float intensity = 1.0f - distToCenter;
        if (intensity <= 0.0f) return; // Too far off screen

        // 5. Calculate the Vector line
        // Vector pointing FROM light TO center
        glm::vec2 center(0.0f);
        glm::vec2 toCenter = center - lightPos2D;

        // 6. Draw Flares
        flareShader.useShaderProgram();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending (Glowing)
        glDisable(GL_DEPTH_TEST);          // Draw on top of everything

        glBindVertexArray(VAO);

        for (size_t i = 0; i < textures.size() && i < spacing.size(); i++) {
            // Position along the line
            glm::vec2 pos = lightPos2D + (toCenter * spacing[i]);

            glUniform2f(glGetUniformLocation(flareShader.shaderProgram, "position"), pos.x, pos.y);
            glUniform1f(glGetUniformLocation(flareShader.shaderProgram, "scale"), scales[i]);
            glUniform1f(glGetUniformLocation(flareShader.shaderProgram, "brightness"), intensity); // You can tweak this per flare if you want

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glUniform1i(glGetUniformLocation(flareShader.shaderProgram, "flareTexture"), 0);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(0);
    }

private:
    void InitOpenGL() {
        // Simple Quad for 2D rendering
        float quadVertices[] = {
            // Pos      // Tex
            -0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.0f, 0.0f,
             0.5f,  0.5f,  1.0f, 1.0f,
             0.5f, -0.5f,  1.0f, 0.0f,
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);
    }

    void InitShader() {
        flareShader.loadShader("shaders/flare.vert", "shaders/flare.frag");
    }

    // Reuse your existing texture loading logic or a simplified one here
    GLuint LoadTexture(const char* path) {
        // ... (Paste your standard stbi_load logic here) ...
        // Ensure you set GL_CLAMP_TO_EDGE so you don't see repeating edges!
        GLuint textureID;
        glGenTextures(1, &textureID);
        int width, height, nrChannels;
        unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Critical for lens flares
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Critical for lens flares
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            stbi_image_free(data);
        }
        return textureID;
    }
};
#endif
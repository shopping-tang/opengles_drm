#include <gles_base.h>


GLuint bind_array()
{
    GLfloat vertices[] = {
        // Positions              // Texture Coords
         1.0f,  1.0f, 0.0f,      1.0f, 1.0f, // Top Right
         1.0f, -1.0f, 0.0f,      1.0f, 0.0f, // Bottom Right
        -1.0f, -1.0f, 0.0f,      0.0f, 0.0f, // Bottom Left
        -1.0f,  1.0f, 0.0f,      0.0f, 1.0f  // Top Left
    };
    GLuint indices[] = {  // Note that we start from 0!
        0, 1, 3, // First Triangle
        1, 2, 3  // Second Triangle
    };
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // TexCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind VAO
    
    return VAO;
}

void init_texture(GLuint *texture)
{
    // ====================
    // Texture 1
    // ====================
    glGenTextures(1, &(texture[0]));
    glBindTexture(GL_TEXTURE_2D, texture[0]); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // Set texture wrapping to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

    // ===================
    // Texture 2
    // ===================
    glGenTextures(1, &(texture[1]));
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // ===================
    // Texture 3
    // ===================
    glGenTextures(1, &(texture[2]));
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    glBindTexture(GL_TEXTURE_2D, 0);
}

void draw(GLuint Program, GLuint *textures, unsigned char *data)
{
    unsigned char *plane[3];

    plane[0] = (unsigned char *)malloc(sizeof(unsigned char)*IMAGE_WIDTH * IMAGE_HEIGHT);; // Y pointer
    plane[1] = (unsigned char *)malloc(sizeof(unsigned char)*IMAGE_WIDTH * IMAGE_HEIGHT/2) ; // U pointer
    plane[2] = (unsigned char *)malloc(sizeof(unsigned char)*IMAGE_WIDTH * IMAGE_HEIGHT/2) ; // U pointer
        

    int i = 0, j = 0, z = 0;

    for (int row = 0; row < BUFFER_SIZE_src ; )
    {
        //YUYV YUYV YUYV YUYV .
        (plane[0])[i++] = data[row++];
        (plane[1])[j++] = data[row++];
        (plane[0])[i++] = data[row++];
        (plane[2])[z++] = data[row++];

    }
    // Bind Textures using texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMAGE_WIDTH, IMAGE_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, plane[0]);
    glUniform1i(glGetUniformLocation(Program, "tex_y"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMAGE_WIDTH/2, IMAGE_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, plane[1]);
    glUniform1i(glGetUniformLocation(Program, "tex_u"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, IMAGE_WIDTH/2, IMAGE_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, plane[2]);
    glUniform1i(glGetUniformLocation(Program, "tex_v"), 2);
    // Draw container »æ ÖÆ ´ú Âë  .
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    free(plane[0]);
    free(plane[1]);
    free(plane[2]);
}


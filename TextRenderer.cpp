#include "TextRenderer.h"

#include <glad/glad.h>
#include <iostream>
#include "../config.h"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>

namespace
{
    const int MAX_WIDTH = 1024;
}


TextRenderer::TextRenderer()
    :m_shader("text", "text")
{
    
    // set uniforms

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(Config::windowX), 0.0f, static_cast<float>(Config::windowY));
    m_shader.useProgram();
    m_shader.setValueFast("projection", projection);

    glm::vec3 color = { 0,1,0 };
    m_shader.setValueFast("textColor", color);

    //texture unit
    m_shader.setValueFast("text", 18);

    //uniforms for translate model and texture coordinates are dynamic 
    m_shader.add("model");
    m_shader.add("coord");

    //create texture atlas
    createAtlas();



    float xpos = 0.0;
    float ypos = 0.0;
    float l = 1.0f;

    std::vector<GLfloat> textureCoord{ 0.0f, 0.0f,
                                 0.0f,1.0f,
                                 1.0f, 1.0f,
                                 0.0f, 0.0f,
                                 1.0f,1.0f,
                                 1.0f, 0.0f };

    std::vector<GLfloat> vertices{ xpos,ypos+l,
                                   xpos,ypos,
                                   xpos+l,ypos,
                                   xpos,ypos+l,
                                   xpos+l,ypos,
                                   xpos+l,ypos+l};

    m_VAO = m_loader.loadToVAO(vertices, textureCoord);


}

TextRenderer::~TextRenderer()
{
    glDeleteTextures(1, &m_texture);
}




void TextRenderer::createAtlas()
{
    FT_Library ft;

    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    std::string font_name = "Res/Fonts/Antonio-Bold.ttf";

    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font " << font_name << std::endl;
        return;
    }

    // set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);
    
    FT_GlyphSlot g = face->glyph;

    auto skipCharacter = [](int c)->bool
    {
        const static std::string s = "'|@~/><^¨{}[]\\\"";
        auto t = s.find(c);
        return t != std::string::npos;
    };

 
    unsigned int row_width = 0;
    unsigned int row_height = 0;

    /* Find minimum size for a texture holding all visible ASCII characters */
    for (int i = 32; i < 128; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Char " << i <<std::endl;
            continue;
        }
        
        //ignore some chars to make the atlas smaller
        if (skipCharacter(i))
        {
            continue;
        }

        if (row_width + g->bitmap.width + 1 >= MAX_WIDTH)
        {           
            m_width = std::max(m_width, row_width);
            m_height += row_height;
            row_width = 0;
            row_height = 0;
        }
        row_width += g->bitmap.width + 1;
        row_height = std::max(row_height, g->bitmap.rows);
    }

    m_width = std::max(m_width, row_width);
    m_height += row_height;

    glActiveTexture(GL_TEXTURE18);
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
  

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    /* We require 1 byte alignment when uploading texture data */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    row_height = 0;
    int xOffset = 0;
    int yOffset = 0;

    for (int i = 32; i < 128; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYPE: Failed to load Char " << i << std::endl;
            continue;
        }

        //ignore some chars to make the atlas smaller
        if (skipCharacter(i))
        {
            continue;
        }


        if (xOffset + g->bitmap.width + 1 >= MAX_WIDTH) {
            yOffset += row_height;
            row_height = 0;
            xOffset = 0;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

        m_characters[i].ax = g->advance.x >> 6;
        m_characters[i].ay = g->advance.y >> 6;

        m_characters[i].bw = g->bitmap.width;
        m_characters[i].bh = g->bitmap.rows;

        m_characters[i].bl = g->bitmap_left;
        m_characters[i].bt = g->bitmap_top;

        m_characters[i].tx = xOffset / (float)m_width;
        m_characters[i].ty = yOffset / (float)m_height;

        row_height = std::max(row_height, g->bitmap.rows);
        xOffset += g->bitmap.width + 1;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

}


void TextRenderer::RenderText()
{

    m_shader.useProgram();

    glEnable(GL_BLEND);
      
    glBindVertexArray(m_VAO);

    int y = -30;

    RenderText("ABZDFGH:0124:opyuiv:wq\\", 5, Config::windowY + y, 0.5f);

    glBindVertexArray(0);

    glDisable(GL_BLEND);

}


void TextRenderer::RenderText(const std::string &text, float x, int y, float scale)
{
    
    for (auto ch : text)
    {       

        if (m_characters[ch].ax == 0)
        {
            //must be a character in the skip list
            continue;
        }
        // position on screen
        float xpos = x + m_characters[ch].bl;
        float ypos = y - (m_characters[ch].bh - m_characters[ch].bt);
               
        float w = m_characters[ch].bw * scale;
        float h = m_characters[ch].bh * scale;

        glm::mat4 matrix{ 1.0f };
        matrix = glm::translate(matrix, { xpos, ypos, 0.0 });
        matrix = glm::scale(matrix, { w,h,0.0f });

        // pick texture coordinates
        glm::vec4 vec;
        vec.x = m_characters[ch].tx;        //x pos in textureatlas
        vec.y = m_characters[ch].ty;        //y pos in textureatlas

        vec.z = m_characters[ch].bw / (float)m_width;   //x texture coordinate
        vec.w = m_characters[ch].bh / (float)m_height;  //y texture coordinate
        
  
        m_shader.setValue("model", matrix);
        m_shader.setValue("coord", vec);
  
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // advance cursors for next glyph     
        x += m_characters[ch].ax * scale;
    }

  

}


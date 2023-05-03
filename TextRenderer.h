#pragma once

#include <vector>
#include <map>

#include "../Shaders/BasicShader.h"
#include <any>
#include <functional>
#include "../observer.h"
#include "../PostProcess/loader.h"
#include <ft2build.h>
#include FT_FREETYPE_H


enum class TextType : size_t
{
    FPS=0,
    POS=1,
    FOG=2,
    TIME=3,

};

class TextRenderer
{

public:
    TextRenderer();
    ~TextRenderer();
    void RenderText();
  
private:
   
    void GameEvent(int, std::any);
    void createAtlas();

    SimpleShader  m_shader;
    std::map<TextType, std::string> m_info;

    void addText(TextType, const std::string &s);
    void RenderText(const std::string &text, float  x, int y, float scale);

    Loader m_loader; 
    
    GLuint m_texture{};		

    unsigned int m_width{};			// width of texture in pixels
    unsigned int m_height{};			// height of texture in pixels

    struct Character {
        FT_Int ax;	// advance.x
        FT_Int ay;	// advance.y

        FT_Int bw;	// bitmap.width;
        FT_Int bh;	// bitmap.height;

        FT_Int bl;	// bitmap_left;
        FT_Int bt;	// bitmap_top;

        float tx;	// x offset of glyph in texture coordinates
        float ty;	// y offset of glyph in texture coordinates
    };
    unsigned int m_VAO{};


    std::array<Character, 128> m_characters{};

};

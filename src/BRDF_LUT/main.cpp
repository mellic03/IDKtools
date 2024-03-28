#include <libidk/idk_gl.hpp>
#include <libidk/idk_gl_headless.hpp>
#include <libidk/idk_image.hpp>

#include <fstream>


#define LUT_TEXTURE_SIZE 512


int main()
{
    idk::initHeadlessGLContext(4, 6);

    idk::RawImageHeader header = {
        .major    = 1,
        .minor    = 1,
        .format   = idk::IDK_IMAGEFORMAT_F32,
        .layout   = idk::IDK_IMAGELAYOUT_RG,
        .w        = LUT_TEXTURE_SIZE,
        .h        = LUT_TEXTURE_SIZE
    };

    header.nbytes = idk::RawImage_nbytes(header);

    idk::glTextureConfig config = {
        .internalformat = GL_RG16F,
        .format         = GL_RG,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_CLAMP_TO_EDGE,
        .wrap_t         = GL_CLAMP_TO_EDGE,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };


    GLuint texture = idk::gltools::loadTexture2D(header.w, header.h, nullptr, config);
    idk::gl::bindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);

    idk::glShaderProgram program("brdf-lut.comp");
    program.bind();
    program.dispatch(LUT_TEXTURE_SIZE/8, LUT_TEXTURE_SIZE/8, 1);
    idk::gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    program.unbind();

    void *pixels = std::malloc(header.nbytes);
    idk::gl::getTextureImage(texture, 0, GL_RG, GL_FLOAT, header.nbytes, pixels);
    idk::RawImage_write("brdf.lut", header, pixels);
    std::free(pixels);


    return 0;
}

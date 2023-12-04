#include "IDKengine/IDKengine.hpp"


int main( int argc, char **argv )
{
    std::string input_path = argv[1];

    idk::RenderEngine ren;
    ren.init("TextureConvert", 64, 64, idk::RenderEngine::INIT_HEADLESS);


    SDL_Surface     *tmp    = IMG_Load(input_path.c_str());
    SDL_PixelFormat *target = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
    SDL_Surface     *img    = SDL_ConvertSurface(tmp, target, 0);


    std::ofstream stream("out.tex", std::ios::binary);

    stream.write((const char *)(img->pixels), 3*1024*1024);
    stream.close();


    SDL_FreeFormat(target);
    SDL_FreeSurface(tmp);
    SDL_FreeSurface(img);


    return 0;
}


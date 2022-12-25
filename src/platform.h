#include <cstdint>
#include <SDL2/SDL.h>

// SDL is used to render and receive input in a multi-platform way
class Platform {
    public:
        // Constructor
        Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);

        // Destructor
        ~Platform();

        // Functions
        void Update(void const* buffer, int pitch);
        bool ProcessInput(uint8_t* keys);

    private:
        SDL_Window* window{};
        SDL_Renderer* renderer{};
	    SDL_Texture* texture{};
};
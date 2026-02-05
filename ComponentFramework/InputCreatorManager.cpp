#include "pch.h"
#include "InputCreatorManager.h"
#include "InputManager.h"
#include "SDL_scancode.h"

int InputCreatorManager::getInputState(std::string input)
{
    InputManager& im = InputManager::getInstance();

    SDL_Scancode scancode = SDL_GetScancodeFromName(input.c_str());

    if (scancode == SDL_SCANCODE_UNKNOWN) {
        std::cerr << "Unknown scancode: " << input << std::endl;
        return -1;
    }

    int result = (int)im.getKeyboardMap()->getInputState(scancode);



    return result;
}




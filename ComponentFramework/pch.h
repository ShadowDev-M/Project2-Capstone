#pragma once

// pre-compiled header file to (hopefully) reduce build times
// only store headers here which will not get edited often, i.e core engine/library headers
// those shouldn't get edited as often so this is a good place to shove them

// Standard Library (STD::)
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <filesystem>
#include <vector>
#include <fstream>
#include <iostream>
#include <string.h>
#include <map>
#include <tuple>
#include <utility>
#include <thread>
#include <chrono>
#include <algorithm>

// SDL & OpenGL
#include <SDL.h>
#include <SDL_image.h>
#include <glew.h>

// ImGui
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "ImGuizmo.h"

// sol & lua
#include <sol/sol.hpp>

// XML
#include "tinyxml2.h"

// irklang
#include <irrKlang.h>

// Math Library 
#include <Vector.h>
#include <MMath.h>
#include <Quaternion.h>
#include <Plane.h>
#include <Matrix.h>
#include <QMath.h>
#include <Euler.h>
#include <VMath.h>
#include <Sphere.h>
#include <PMath.h>
#include <EMath.h>

// Components (this depends on which components are fully finished and won't get edited anytime soon)
#include "Component.h"

// Windows (same goes for windows as components)

// Extra
#include "Debug.h"
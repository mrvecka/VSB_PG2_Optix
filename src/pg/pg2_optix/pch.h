#ifndef PCH_H
#define PCH_H

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

// std libs
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>
#include <tchar.h>
#include <vector>
#include <map>
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdexcept>

// Nvidia OptiX 6.0.0
#include <optix.h>
#include <optix_world.h>

// Dear ImGui (bloat-free graphical user interface library for C++)
#include <imgui.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#endif //PCH_H

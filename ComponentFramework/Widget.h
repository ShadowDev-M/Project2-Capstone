#pragma once

#include "VMath.h"
#include "Actor.h"
#include "CameraActor.h"
#include <vector>

#include "SceneGraph.h"
#include "AssetManager.h"


using namespace MATH;

class Widget {

public:
    // Convert an object ID into a color for selection buffer
    static Vec3 encodeIDToColor(uint32_t id) {
        return Vec3(
            ((id >> 16) & 0xFF) / 255.0f,
            ((id >> 8) & 0xFF) / 255.0f,
            (id & 0xFF) / 255.0f
        );
    }

    // Convert a color from selection buffer back into an object ID
    static uint32_t decodeColorToID(const Vec3& color) {
        uint8_t r = static_cast<uint8_t>(color.x * 255.0f);
        uint8_t g = static_cast<uint8_t>(color.y * 255.0f);
        uint8_t b = static_cast<uint8_t>(color.z * 255.0f);
        return (r << 16) | (g << 8) | b;
    }

};
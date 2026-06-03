#pragma once

// Visual Studio 기본 공통 헤더를 포함합니다.
#include "framework.h"

// 월드 좌표계의 단위 정의입니다. 과제 구현에서는 float 1.0f를 1m로 해석합니다.
inline constexpr float GP_WORLD_UNITS_PER_METER = 1.0f;

// 기존 지형은 약 44.8m 폭이었고, 과제 수정 요구에 맞춰 약 8배 넓은 지형으로 확장합니다.
inline constexpr float GP_TERRAIN_BASE_CELL_METERS = 1.4f;
inline constexpr float GP_TERRAIN_SIZE_SCALE = 8.0f;
inline constexpr int GP_TERRAIN_BASE_CELL_COUNT = 32;
inline constexpr int GP_TERRAIN_GRID_CELL_COUNT = 128;
inline constexpr int GP_TERRAIN_GRID_VERTEX_COUNT = GP_TERRAIN_GRID_CELL_COUNT + 1;
inline constexpr float GP_TERRAIN_CELL_METERS =
    GP_TERRAIN_BASE_CELL_METERS * GP_TERRAIN_SIZE_SCALE *
    static_cast<float>(GP_TERRAIN_BASE_CELL_COUNT) /
    static_cast<float>(GP_TERRAIN_GRID_CELL_COUNT);
inline constexpr float GP_TERRAIN_HALF_SIZE_METERS =
    GP_TERRAIN_CELL_METERS * static_cast<float>(GP_TERRAIN_GRID_CELL_COUNT) * 0.5f;

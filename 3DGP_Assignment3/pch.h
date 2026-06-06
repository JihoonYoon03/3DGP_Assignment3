#pragma once

// Visual Studio 기본 공통 헤더를 포함합니다.
#include "framework.h"

// 월드 좌표계의 단위 정의입니다. 과제 구현에서는 float 1.0f를 1m로 해석합니다.
inline constexpr float GP_WORLD_UNITS_PER_METER = 1.0f;

// 기존 지형은 약 44.8m 크기였고, 수정 요구에 맞춰 약 8배 넓은 지형으로 확장합니다.
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

// Apache.txt 원본 모델 단위를 월드 미터로 변환하는 배율입니다.
inline constexpr float GP_APACHE_SOURCE_UNIT_TO_METER = 0.09f;
inline constexpr float GP_APACHE_MODEL_SCALE = GP_APACHE_SOURCE_UNIT_TO_METER * GP_WORLD_UNITS_PER_METER;

// 실제 Apache 모델을 사용할 때 총구를 기체 앞쪽으로 충분히 이동시키는 거리입니다.
inline constexpr float GP_APACHE_MUZZLE_OFFSET_METERS = 5.2f;

// Unreal Landscape 스케일 값을 우리 월드 미터 단위로 맞추기 위한 하이트맵 설정입니다.
inline constexpr float GP_UNREAL_CENTIMETER_TO_METER = 0.01f;
inline constexpr float GP_HEIGHTMAP_UNREAL_X_SCALE = 1578.38f;
inline constexpr float GP_HEIGHTMAP_UNREAL_Y_SCALE = 1578.38f;
inline constexpr float GP_HEIGHTMAP_UNREAL_Z_SCALE = 75.39f;
inline constexpr float GP_HEIGHTMAP_PROJECT_HORIZONTAL_SCALE = 0.10f;
inline constexpr float GP_HEIGHTMAP_PROJECT_VERTICAL_SCALE = 0.25f;
inline constexpr float GP_HEIGHTMAP_CELL_X_METERS =
    GP_HEIGHTMAP_UNREAL_X_SCALE * GP_UNREAL_CENTIMETER_TO_METER * GP_HEIGHTMAP_PROJECT_HORIZONTAL_SCALE;
inline constexpr float GP_HEIGHTMAP_CELL_Z_METERS =
    GP_HEIGHTMAP_UNREAL_Y_SCALE * GP_UNREAL_CENTIMETER_TO_METER * GP_HEIGHTMAP_PROJECT_HORIZONTAL_SCALE;
inline constexpr float GP_HEIGHTMAP_MAX_HEIGHT_METERS =
    GP_HEIGHTMAP_UNREAL_Z_SCALE * GP_HEIGHTMAP_PROJECT_VERTICAL_SCALE;
// 하이트맵 지형 전체를 아래로 내려 헬기와 표적이 시작 시 지면에 파묻히지 않게 합니다.
inline constexpr float GP_TERRAIN_BASE_HEIGHT_OFFSET_METERS = -12.0f;
// 지형 높이 충돌을 적용할 때 각 객체 중심이 지면보다 떠 있어야 하는 기본 높이입니다.
inline constexpr float GP_PLAYER_TERRAIN_CLEARANCE_METERS = 4.0f;
inline constexpr float GP_ENEMY_TERRAIN_CLEARANCE_METERS = 0.8f;

// 태양 방향성 광원과 Phong 조명 항입니다.
inline constexpr float GP_LIGHT_AMBIENT_STRENGTH = 0.32f;
inline constexpr float GP_LIGHT_DIFFUSE_STRENGTH = 0.78f;
inline constexpr float GP_LIGHT_SPECULAR_STRENGTH = 0.35f;
inline constexpr float GP_LIGHT_SPECULAR_POWER = 32.0f;

// Level-1에 배치할 적 개수입니다. 값을 바꾸면 ResetLevel에서 자동으로 재배치합니다.
inline constexpr int GP_LEVEL_TARGET_COUNT = 12;

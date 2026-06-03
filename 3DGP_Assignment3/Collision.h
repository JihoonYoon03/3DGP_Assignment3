#pragma once

// 조준 광선과 간단한 충돌 판정 유틸리티입니다.
#include <DirectXMath.h>

#include <algorithm>
#include <cmath>

namespace Collision
{
    // 광선은 시작점과 정규화된 방향으로 표현합니다.
    struct Ray
    {
        DirectX::XMFLOAT3 origin{};
        DirectX::XMFLOAT3 direction{ 0.0f, 0.0f, 1.0f };
    };

    // 광선 충돌 결과는 충돌 여부, 거리, 월드 위치를 보관합니다.
    struct HitResult
    {
        bool hit = false;
        float distance = 0.0f;
        DirectX::XMFLOAT3 position{};
    };

    // 두 벡터를 더합니다.
    inline DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return { a.x + b.x, a.y + b.y, a.z + b.z };
    }

    // 벡터에 스칼라를 곱합니다.
    inline DirectX::XMFLOAT3 Scale(const DirectX::XMFLOAT3& v, float scale)
    {
        return { v.x * scale, v.y * scale, v.z * scale };
    }

    // 두 벡터의 내적을 계산합니다.
    inline float Dot(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    // 벡터 길이를 1로 맞춥니다.
    inline DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& v)
    {
        const float length = std::sqrt(std::max(0.000001f, Dot(v, v)));
        return { v.x / length, v.y / length, v.z / length };
    }

    // 광선 위의 특정 거리 지점을 계산합니다.
    inline DirectX::XMFLOAT3 PointAt(const Ray& ray, float distance)
    {
        return Add(ray.origin, Scale(ray.direction, distance));
    }

    // y = planeY 평면과 광선의 교차점을 계산합니다.
    inline HitResult RaycastPlaneY(const Ray& ray, float planeY, float maxDistance)
    {
        HitResult result{};
        if (std::fabs(ray.direction.y) < 0.0001f)
        {
            return result;
        }

        const float distance = (planeY - ray.origin.y) / ray.direction.y;
        if (distance <= 0.0f || distance > maxDistance)
        {
            return result;
        }

        result.hit = true;
        result.distance = distance;
        result.position = PointAt(ray, distance);
        return result;
    }

    // 표적 박스는 현재 임시 구현이므로 광선-구 충돌로 가볍게 판정합니다.
    inline HitResult RaycastSphere(const Ray& ray, const DirectX::XMFLOAT3& center, float radius, float maxDistance)
    {
        const DirectX::XMFLOAT3 originToCenter
        {
            ray.origin.x - center.x,
            ray.origin.y - center.y,
            ray.origin.z - center.z
        };

        const float b = Dot(originToCenter, ray.direction);
        const float c = Dot(originToCenter, originToCenter) - radius * radius;
        const float discriminant = b * b - c;
        HitResult result{};
        if (discriminant < 0.0f)
        {
            return result;
        }

        const float distance = -b - std::sqrt(discriminant);
        if (distance <= 0.0f || distance > maxDistance)
        {
            return result;
        }

        result.hit = true;
        result.distance = distance;
        result.position = PointAt(ray, distance);
        return result;
    }
}

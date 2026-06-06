#include "GameFramework.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;


XMMATRIX AssignmentGame::ProjectionMatrix() const
{
    // 창 비율에 맞춘 왼손 좌표계 원근 투영입니다.
    const float aspectRatio = static_cast<float>(std::max(1u, m_width)) / static_cast<float>(std::max(1u, m_height));
    return XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), aspectRatio, 0.1f, 1200.0f);
}

XMMATRIX AssignmentGame::LevelViewMatrix() const
{
    // 3인칭 카메라는 헬리콥터 뒤쪽 위에서 본체를 바라봅니다.
    const XMFLOAT3 forward = ForwardDirection();
    const XMFLOAT3 flatForward{ std::sinf(m_helicopterYaw), 0.0f, std::cosf(m_helicopterYaw) };
    const XMFLOAT3 eye =
    {
        m_helicopterPosition.x - flatForward.x * 7.0f,
        m_helicopterPosition.y + 4.0f,
        m_helicopterPosition.z - flatForward.z * 7.0f
    };
    const XMFLOAT3 target =
    {
        m_helicopterPosition.x + forward.x * 3.0f,
        m_helicopterPosition.y + 0.65f + forward.y * 3.0f,
        m_helicopterPosition.z + forward.z * 3.0f
    };

    return XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

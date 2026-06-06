#include "GameFramework.h"

#include <stdexcept>


UINT AssignmentGame::AlignConstantBufferSize(UINT byteSize)
{
    // D3D12 상수 버퍼는 256바이트 경계에 맞춰야 합니다.
    constexpr UINT alignment = 256;
    return (byteSize + alignment - 1) & ~(alignment - 1);
}

void AssignmentGame::WaitForGpu()
{
    // GPU에 Fence 값을 신호하고 완료될 때까지 기다립니다.
    const UINT64 fenceToWaitFor = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor));
    ++m_fenceValue;

    if (m_fence->GetCompletedValue() < fenceToWaitFor)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void AssignmentGame::FlushCommandQueue()
{
    // 리소스 재생성 전 모든 GPU 작업이 끝났는지 보장합니다.
    WaitForGpu();
}

D3D12_HEAP_PROPERTIES AssignmentGame::HeapProperties(D3D12_HEAP_TYPE type)
{
    // D3D12 헬퍼 헤더 없이도 힙 속성을 만들기 위한 작은 유틸리티입니다.
    D3D12_HEAP_PROPERTIES properties{};
    properties.Type = type;
    properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    properties.CreationNodeMask = 1;
    properties.VisibleNodeMask = 1;
    return properties;
}

D3D12_RESOURCE_DESC AssignmentGame::BufferResourceDesc(UINT64 byteSize)
{
    // 버퍼 리소스 설명은 정점/인덱스/상수 업로드 버퍼에 공통으로 사용합니다.
    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = byteSize;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    return desc;
}

D3D12_RESOURCE_BARRIER AssignmentGame::TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    // 리소스 상태 전환 배리어를 명확하게 생성합니다.
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = beforeState;
    barrier.Transition.StateAfter = afterState;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    return barrier;
}

void AssignmentGame::ThrowIfFailed(HRESULT hr)
{
    // 실패 HRESULT는 WinMain에서 메시지 박스로 보여 줄 예외로 바꿉니다.
    if (FAILED(hr))
    {
        throw std::runtime_error("Direct3D 12 호출이 실패했습니다.");
    }
}

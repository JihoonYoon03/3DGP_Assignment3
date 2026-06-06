#include "GameFramework.h"

#include <d3dcompiler.h>

#include <algorithm>
#include <cstring>

using Microsoft::WRL::ComPtr;
using namespace DirectX;


void AssignmentGame::CreatePipelineState()
{
    // 루트 시그니처는 객체별 상수 버퍼 하나만 셰이더에 전달합니다.
    D3D12_ROOT_PARAMETER rootParameter{};
    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter.Descriptor.ShaderRegister = 0;
    rootParameter.Descriptor.RegisterSpace = 0;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &rootParameter;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob));
    ThrowIfFailed(m_device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

    // 앰비언트, 디퓨즈, 스페큘러 항을 갖는 방향성 광원 셰이더입니다.
    const char* shaderSource = R"(
cbuffer ObjectConstants : register(b0)
{
    float4x4 gWorld;
    float4x4 gWorldInverseTranspose;
    float4x4 gWorldViewProjection;
    float4 gColor;
    float4 gCameraPosition;
    float4 gLightDirection;
    float4 gAmbientColor;
    float4 gDiffuseColor;
    float4 gSpecularColor;
    float4 gLightingOptions;
};

struct VertexIn
{
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
};

struct VertexOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 worldPosition : WORLDPOSITION;
    float3 normal : NORMAL;
};

VertexOut VSMain(VertexIn input)
{
    VertexOut output;
    float4 worldPosition = mul(float4(input.position, 1.0f), gWorld);
    output.position = mul(float4(input.position, 1.0f), gWorldViewProjection);
    output.color = input.color * gColor;
    output.worldPosition = worldPosition.xyz;
    output.normal = normalize(mul(float4(input.normal, 0.0f), gWorldInverseTranspose).xyz);
    return output;
}

float4 PSMain(VertexOut input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float3 lightToSurface = normalize(gLightDirection.xyz);
    float3 surfaceToLight = -lightToSurface;
    float diffuseFactor = saturate(dot(normal, surfaceToLight));

    float3 viewDirection = normalize(gCameraPosition.xyz - input.worldPosition);
    float3 reflectDirection = reflect(lightToSurface, normal);
    float specularFactor = pow(saturate(dot(viewDirection, reflectDirection)), gLightingOptions.x);

    float3 litColor =
        gAmbientColor.rgb +
        gDiffuseColor.rgb * diffuseFactor +
        gSpecularColor.rgb * specularFactor;
    return float4(input.color.rgb * litColor, input.color.a);
}
)";

    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    // Debug 빌드에서는 셰이더 디버깅 정보를 남깁니다.
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ThrowIfFailed(D3DCompile(shaderSource, std::strlen(shaderSource), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &errorBlob));
    ThrowIfFailed(D3DCompile(shaderSource, std::strlen(shaderSource), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &errorBlob));

    // 정점 입력 레이아웃은 위치, 색상, 노멀 세 요소로 구성됩니다.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 래스터라이저는 지형 스트립의 양면을 모두 볼 수 있게 컬링을 끕니다.
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // 블렌딩은 사용하지 않고 색상을 그대로 씁니다.
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // 깊이 테스트를 켜서 3D 객체가 올바른 순서로 보이게 합니다.
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    // 그래픽 파이프라인 상태 객체는 셰이더와 렌더 상태를 하나로 묶습니다.
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void AssignmentGame::Render()
{
    // 현재 씬의 DrawItem 목록을 만든 뒤 씬에 맞는 카메라를 선택합니다.
    BuildDrawItems();

    const XMMATRIX view =
        (m_scene == SceneMode::Level1)
        ? LevelViewMatrix()
        : XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -8.5f, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    const XMMATRIX viewProjection = view * ProjectionMatrix();

    // 명령 목록을 기록하고 GPU에 제출합니다.
    PopulateCommandList(viewProjection);
    ID3D12CommandList* commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    // 수직 동기화 1로 Present하여 과제 실행 중 화면 찢김을 줄입니다.
    ThrowIfFailed(m_swapChain->Present(1, 0));
    WaitForGpu();
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void AssignmentGame::PopulateCommandList(const XMMATRIX& viewProjection)
{
    // 이전 프레임이 끝난 뒤 같은 할당자를 재사용합니다.
    ThrowIfFailed(m_commandAllocator->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

    // 현재 객체들의 상수 버퍼를 먼저 채웁니다.
    UploadObjectConstants(viewProjection);

    // 현재 백 버퍼를 렌더 타깃 상태로 전환합니다.
    const D3D12_RESOURCE_BARRIER toRenderTarget = TransitionBarrier(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &toRenderTarget);

    // 렌더링 상태와 출력 버퍼를 설정합니다.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += static_cast<SIZE_T>(m_frameIndex) * m_rtvDescriptorSize;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 씬별 배경색은 시작/메뉴와 Level-1을 구분하기 위해 다르게 둡니다.
    const float clearColorStart[4] = { 0.03f, 0.05f, 0.09f, 1.0f };
    const float clearColorLevel[4] = { 0.38f, 0.55f, 0.78f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, (m_scene == SceneMode::Level1) ? clearColorLevel : clearColorStart, 0, nullptr);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 모든 렌더 항목을 메시 종류에 맞게 그립니다.
    const UINT constantBufferStride = AlignConstantBufferSize(sizeof(ObjectConstants));
    for (std::size_t itemIndex = 0; itemIndex < m_drawItems.size(); ++itemIndex)
    {
        const DrawItem& item = m_drawItems[itemIndex];
        const MeshResource* mesh = &m_meshes[static_cast<std::size_t>(item.mesh)];
        if (item.mesh == MeshKind::Apache)
        {
            if (item.meshPartIndex >= m_apacheParts.size())
            {
                continue;
            }
            mesh = &m_apacheParts[item.meshPartIndex].mesh;
        }

        if (mesh->indexCount == 0 || !mesh->vertexBuffer || !mesh->indexBuffer)
        {
            // 모델 파일을 찾지 못한 메시 항목은 앱을 중단하지 않고 건너뜁니다.
            continue;
        }

        m_commandList->IASetPrimitiveTopology(mesh->topology);
        m_commandList->IASetVertexBuffers(0, 1, &mesh->vertexBufferView);
        m_commandList->IASetIndexBuffer(&mesh->indexBufferView);
        m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + itemIndex * constantBufferStride);
        m_commandList->DrawIndexedInstanced(mesh->indexCount, 1, 0, 0, 0);
    }

    // 백 버퍼를 다시 Present 상태로 돌려 화면에 표시할 준비를 합니다.
    const D3D12_RESOURCE_BARRIER toPresent = TransitionBarrier(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &toPresent);
    ThrowIfFailed(m_commandList->Close());
}

void AssignmentGame::UploadObjectConstants(const XMMATRIX& viewProjection)
{
    // DrawItem 수가 상수 버퍼 한도를 넘으면 앞쪽 항목만 안전하게 업로드합니다.
    if (m_drawItems.size() > MaxDrawItems)
    {
        m_drawItems.resize(MaxDrawItems);
    }

    const UINT constantBufferStride = AlignConstantBufferSize(sizeof(ObjectConstants));
    const XMFLOAT3 cameraPosition =
        (m_scene == SceneMode::Level1)
        ? LevelCameraPosition()
        : XMFLOAT3{ 0.0f, 0.0f, -8.5f };

    const XMVECTOR lightDirectionVector = XMVector3Normalize(XMVectorSet(-0.45f, -0.85f, 0.25f, 0.0f));
    XMFLOAT3 lightDirection{};
    XMStoreFloat3(&lightDirection, lightDirectionVector);

    for (std::size_t itemIndex = 0; itemIndex < m_drawItems.size(); ++itemIndex)
    {
        const DrawItem& item = m_drawItems[itemIndex];
        const XMMATRIX world = XMLoadFloat4x4(&item.world);
        const XMMATRIX worldInverseTranspose = XMMatrixInverse(nullptr, world);
        const XMMATRIX worldViewProjection = XMMatrixTranspose(world * viewProjection);

        ObjectConstants constants{};
        XMStoreFloat4x4(&constants.world, XMMatrixTranspose(world));
        XMStoreFloat4x4(&constants.worldInverseTranspose, XMMatrixTranspose(worldInverseTranspose));
        XMStoreFloat4x4(&constants.worldViewProjection, worldViewProjection);
        constants.color = item.color;
        constants.cameraPosition = { cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f };
        constants.lightDirection = { lightDirection.x, lightDirection.y, lightDirection.z, 0.0f };
        constants.ambientColor = { GP_LIGHT_AMBIENT_STRENGTH, GP_LIGHT_AMBIENT_STRENGTH, GP_LIGHT_AMBIENT_STRENGTH, 1.0f };
        constants.diffuseColor = { GP_LIGHT_DIFFUSE_STRENGTH, GP_LIGHT_DIFFUSE_STRENGTH, GP_LIGHT_DIFFUSE_STRENGTH, 1.0f };
        constants.specularColor = { GP_LIGHT_SPECULAR_STRENGTH, GP_LIGHT_SPECULAR_STRENGTH, GP_LIGHT_SPECULAR_STRENGTH, 1.0f };
        constants.lightingOptions = { GP_LIGHT_SPECULAR_POWER, 0.0f, 0.0f, 0.0f };

        std::memcpy(m_mappedConstantBuffer + itemIndex * constantBufferStride, &constants, sizeof(constants));
    }
}

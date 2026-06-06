// 객체별 월드 행렬, 색상, 카메라 위치, 조명 정보를 받는 상수 버퍼입니다.
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

// CPU 정점 버퍼에서 전달되는 위치, 색상, 노멀 데이터입니다.
struct VertexIn
{
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
};

// 래스터라이저와 픽셀 셰이더로 넘길 보간 데이터입니다.
struct VertexOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 worldPosition : WORLDPOSITION;
    float3 normal : NORMAL;
};

// 정점을 클립 공간으로 변환하고 조명 계산용 월드 좌표와 노멀을 넘깁니다.
VertexOut VSMain(VertexIn input)
{
    VertexOut output;
    const float4 worldPosition = mul(float4(input.position, 1.0f), gWorld);
    output.position = mul(float4(input.position, 1.0f), gWorldViewProjection);
    output.color = input.color * gColor;
    output.worldPosition = worldPosition.xyz;
    output.normal = normalize(mul(float4(input.normal, 0.0f), gWorldInverseTranspose).xyz);
    return output;
}

// 방향성 광원 하나로 앰비언트, 디퓨즈, 스페큘러 조명을 계산합니다.
float4 PSMain(VertexOut input) : SV_TARGET
{
    const float3 normal = normalize(input.normal);
    const float3 lightToSurface = normalize(gLightDirection.xyz);
    const float3 surfaceToLight = -lightToSurface;
    const float diffuseFactor = saturate(dot(normal, surfaceToLight));

    const float3 viewDirection = normalize(gCameraPosition.xyz - input.worldPosition);
    const float3 reflectDirection = reflect(lightToSurface, normal);
    const float specularFactor = pow(saturate(dot(viewDirection, reflectDirection)), gLightingOptions.x);

    const float3 litColor =
        gAmbientColor.rgb +
        gDiffuseColor.rgb * diffuseFactor +
        gSpecularColor.rgb * specularFactor;
    return float4(input.color.rgb * litColor, input.color.a);
}

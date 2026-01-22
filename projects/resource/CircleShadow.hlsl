// ----------------------------------------------------------
// 丸影シェーダー.
// ----------------------------------------------------------
// カメラ定数バッファ.
cbuffer CBPerCamera : register(b8)
{
    row_major float4x4 view;
    row_major float4x4 projection;
    float3 cameraPosW;
    float cameraNear;
    float3 cameraForwardW;
    float cameraFar;
    float4 time;
};

// オブジェクト定数バッファ.
cbuffer CBPerObject : register(b9)
{
    float4x4 world;
};

// 頂点シェーダーへ入力するデータ.
struct VSInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
};

// 頂点シェーダーから出力するデータ.
struct PSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

// 頂点シェーダー.
PSInput VS(VSInput vin)
{
    PSInput Out;
    float4 p = float4(vin.pos.xyz, 1);
    p = mul(p, world);
    p = mul(p, view);
    p = mul(p, projection);
    Out.pos = p;
    Out.uv = vin.uv;
    return Out;
}

// ----------------------------------------------------------
// ピクセル.
// ----------------------------------------------------------
// ピクセルシェーダー.
float4 PS(PSInput In) : SV_Target0
{
    // UV座標の中心からの距離を計算.
    float2 center = float2(0.5, 0.5);
    float dist = distance(In.uv, center);

    // 円形のマスクを生成（中心は濃く、端はフェードアウト）.
    float radius = 0.5;
    float softness = 0.3;

    // 円の外側は完全に透明.
    if (dist > radius)
    {
        discard;
    }

    // 中心から端に向かってフェードアウト.
    float alpha = 1.0 - (dist / radius);
    alpha = alpha * alpha;  // よりソフトなフェード.

    // 影の色（半透明の黒）.
    float shadowIntensity = 0.5;
    return float4(0.0, 0.0, 0.0, alpha * shadowIntensity);
}

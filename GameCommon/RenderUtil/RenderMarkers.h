#pragma once
#include <d3d9.h>
#include <wchar.h>

// Wraps D3DPERF_BeginEvent / EndEvent (built into d3d9.dll — no extra lib needed).
// These appear as labeled regions in PIX, RenderDoc, and Nsight captures.

struct RenderGpuScope
{
    RenderGpuScope(const wchar_t* label, D3DCOLOR color = D3DCOLOR_XRGB(0, 128, 255))
    {
        D3DPERF_BeginEvent(color, label);
    }
    ~RenderGpuScope() { D3DPERF_EndEvent(); }
};

#define RENDER_GPU_SCOPE(wlabel)        RenderGpuScope _rgs_##__LINE__(wlabel)
#define RENDER_GPU_SCOPE_COL(wl, col)   RenderGpuScope _rgs_##__LINE__(wl, col)
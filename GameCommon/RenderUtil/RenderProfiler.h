#pragma once
#include <stdwininclude.h>
#include <d3d9.h>
#include <d3d9helper.h>
#include <string>
#include <vector>
#include <cstdio>

// ---------------------------------------------------------------------------
// CPU scope timer — measures wall time of any render block.
// Usage:  RENDER_PROFILE_SCOPE("Flush/Particles");
// ---------------------------------------------------------------------------
struct RenderCpuSample
{
    const char* label;
    double      ms;
};

class RenderProfiler
{
public:
    static RenderProfiler& Get() { static RenderProfiler inst; return inst; }

    void BeginFrame()
    {
        mSamples.clear();
        QueryPerformanceFrequency(&mFreq);
    }

    void PushSample(const char* label, LARGE_INTEGER start, LARGE_INTEGER end)
    {
        double ms = (double)(end.QuadPart - start.QuadPart) * 1000.0 / (double)mFreq.QuadPart;
        mSamples.push_back({ label, ms });
    }

    // Call once per frame (e.g. after Present) to dump to Output / log
    void DumpFrame()
    {
        char buf[256];
        for (const auto& s : mSamples)
        {
            sprintf_s(buf, "[RenderProfiler] %-40s  %.3f ms\n", s.label, s.ms);
            OutputDebugStringA(buf);
        }
    }

    const std::vector<RenderCpuSample>& GetSamples() const { return mSamples; }

private:
    LARGE_INTEGER                  mFreq{};
    std::vector<RenderCpuSample>   mSamples;
};

struct RenderScopeTimer
{
    RenderScopeTimer(const char* label) : mLabel(label)
    {
        QueryPerformanceCounter(&mStart);
    }
    ~RenderScopeTimer()
    {
        LARGE_INTEGER end;
        QueryPerformanceCounter(&end);
        RenderProfiler::Get().PushSample(mLabel, mStart, end);
    }
    const char*   mLabel;
    LARGE_INTEGER mStart{};
};

#define RENDER_PROFILE_SCOPE(label)  RenderScopeTimer _rps_##__LINE__(label)
#pragma once

#include <array>
#include <cstdint>

class Filter {
public:
    Filter(float sampleRate) : sampleRate(sampleRate) { }

    void setParams(float cutoff, float resonance);
    float lowPass(float input);
    float highPass(float input);
    float bandPass(float input);

private:
    std::array<float, 3> v_lp = { 0, 0, 0};
    std::array<float, 3> v_hp = { 0, 0, 0};
    std::array<float, 3> v_bp = { 0, 0, 0};

    float sampleRate;
    std::array<float, 3> a_lp;
    std::array<float, 3> a_hp;
    std::array<float, 3> a_bp;
    std::array<float, 2> b;
};

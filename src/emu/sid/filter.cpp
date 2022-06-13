#include "filter.h"

#include <cmath>

// based on formulas from http://www.earlevel.com/main/2011/01/02/biquad-formulas/
void Filter::setParams(float cutoff, float resonance) {
    float K = tanf(M_PI * cutoff / sampleRate);
    float norm = 1 / (1 + K / resonance + K * K);

    b = { 2 * (K * K - 1) * norm, (1 - K / resonance + K * K) * norm };

    float a0_lp = K * K * norm;
    a_lp = { a0_lp, 2 * a0_lp, a0_lp};

    a_hp = { norm, -2 * norm, norm };

    float a0_bp = K / resonance * norm;
    a_bp = { a0_bp, 0, -a0_bp };
}

// based on maxiBiquad::play() in Maximilian: https://github.com/micknoise/Maximilian/blob/master/src/maximilian.h
float Filter::lowPass(float input) {
    v_lp[0] = input - b[0] * v_lp[1] - b[1] * v_lp[2];
    float output = a_lp[0] * v_lp[0] + a_lp[1] * v_lp[1] + a_lp[2] * v_lp[2];
    v_lp[2] = v_lp[1];
    v_lp[1] = v_lp[0];
    return output;
}

// based on maxiBiquad::play() in Maximilian: https://github.com/micknoise/Maximilian/blob/master/src/maximilian.h
float Filter::highPass(float input) {
    v_hp[0] = input - b[0] * v_hp[1] - b[1] * v_hp[2];
    float output = a_hp[0] * v_hp[0] + a_hp[1] * v_hp[1] + a_hp[2] * v_hp[2];
    v_hp[2] = v_hp[1];
    v_hp[1] = v_hp[0];
    return output;
}

// based on maxiBiquad::play() in Maximilian: https://github.com/micknoise/Maximilian/blob/master/src/maximilian.h
float Filter::bandPass(float input) {
    v_bp[0] = input - b[0] * v_bp[1] - b[1] * v_bp[2];
    float output = a_bp[0] * v_bp[0] + a_bp[1] * v_bp[1] + a_bp[2] * v_bp[2];
    v_bp[2] = v_bp[1];
    v_bp[1] = v_bp[0];
    return output;
}

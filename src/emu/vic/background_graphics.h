#pragma once

class VIC;

class BackgroundGraphics {
public:
    BackgroundGraphics(VIC* vic) : vic(vic) { }

    void cAccess();
    void gAccess();

    void standardTextModeCAccess();
    void standardTextModeGAccess();

public:
    VIC *vic;
};

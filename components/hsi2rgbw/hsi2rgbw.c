#include "hsi2rgbw.h"
#include <math.h>

void hsi2rgbw(float H, float S, float I, int *rgbw) {
    int r = 0, g = 0, b = 0, w = 0;
    float cos_h, cos_1047_h;

    H = fmodf(H, 360.0f);
    H = (float)M_PI * H / 180.0f;
    S = fminf(fmaxf(S, 0.0f), 1.0f);
    I = fminf(fmaxf(I, 0.0f), 1.0f);

    if (H < 2.09439f) {
        cos_h = cosf(H);
        cos_1047_h = cosf(1.047196667f - H);
        r = (int)roundf(S * 255 * I / 3 * (1 + cos_h / cos_1047_h));
        g = (int)roundf(S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h)));
        w = (int)roundf(255 * (1 - S) * I);
    } else if (H < 4.188787f) {
        H -= 2.09439f;
        cos_h = cosf(H);
        cos_1047_h = cosf(1.047196667f - H);
        g = (int)roundf(S * 255 * I / 3 * (1 + cos_h / cos_1047_h));
        b = (int)roundf(S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h)));
        w = (int)roundf(255 * (1 - S) * I);
    } else {
        H -= 4.188787f;
        cos_h = cosf(H);
        cos_1047_h = cosf(1.047196667f - H);
        b = (int)roundf(S * 255 * I / 3 * (1 + cos_h / cos_1047_h));
        r = (int)roundf(S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h)));
        w = (int)roundf(255 * (1 - S) * I);
    }

    rgbw[0] = r;
    rgbw[1] = g;
    rgbw[2] = b;
    rgbw[3] = w;
}

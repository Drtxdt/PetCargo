#ifndef PETCARGO_OLED_H
#define PETCARGO_OLED_H

#include <stdint.h>

enum oled_face {
    FACE_SMUG = 0,
    FACE_HAPPY,
    FACE_FEAR,
    FACE_DIZZY,
    FACE_SLEEP,
    FACE_HOT,
    FACE_MUSIC,
    FACE_FORWARD,
    FACE_BACKWARD,
    FACE_LEFT,
    FACE_RIGHT,
    FACE_STOP
};

void oled_init(void);
void oled_draw_face(uint8_t face);

#endif

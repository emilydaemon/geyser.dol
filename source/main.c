/*
	geyser.dol by Korbosoft et al.
	(Generative Emulator Yielding Sequential Entropic Rectangles)

	TODO: licensing information
*/

#include <grrlib.h>

#include <ogc/lwp_watchdog.h>   // Needed for gettime and ticks_to_millisecs
#include <ogcsys.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <wiiuse/wpad.h>
#include <aesndlib.h>

#include "video.h"

#include "freemonobold_ttf.h"

#define SAMPLE_RATE 48000
#define MIN_FREQ 440
#define MAX_FREQ 1760

extern u32 rectColors[MAX_RECTANGLES];

static void VoiceCallBack(AESNDPB *pb, u32 state)
{
    if (state == VOICE_STATE_STOPPED)
        AESND_FreeVoice(pb);
}

static s16 *generate_sine(void) {
    double freq = ((double)rand() / (double)RAND_MAX) * (MAX_FREQ - MIN_FREQ) + MIN_FREQ;

    s16 *output_buffer = malloc(SAMPLE_RATE * 2);
    if (!output_buffer) {
        return NULL;
    }

    double phase = 0.0;
    double phase_step = (2.0 * M_PI * freq) / (double)SAMPLE_RATE;

    for (u16 i = 0; i < SAMPLE_RATE; i++) {
        output_buffer[i] = (s16)(sin(phase) * 32767);

        phase += phase_step;

        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
    }

    return output_buffer;
}

static void delay(unsigned s) {
    unsigned long long start, end;
    start = gettime();
    while (1)
    {
        end = gettime();
        if (diff_sec(start,end) >= s)
            break;
    }
}

static u8 rectangleCount = 4;

int main() {
    unsigned long long start = gettime(), end = gettime();
    s16 slide = -1;

    AESND_Init();

    AESNDPB* voice;

    Rect rects[MAX_RECTANGLES];

    srand(time(0));

    GRRLIB_Init();

    GRRLIB_ttfFont *font = GRRLIB_LoadTTF(freemonobold_ttf, freemonobold_ttf_size);

    GRRLIB_SetBackgroundColour(255, 255, 255, 255);

    WPAD_Init();

    get_rect_colors(rectangleCount);

    GRRLIB_FillScreen(GRRLIB_WHITE);
    GRRLIB_Render();

    delay(1);
    s16 *soundBuf = generate_sine();
    voice = AESND_AllocateVoice(VoiceCallBack);
    AESND_PlayVoice(voice, VOICE_MONO16, soundBuf, SAMPLE_RATE * 2, SAMPLE_RATE, 0, false);

    while(1) {
        char str[25];
        if (diff_sec(start,end)) {
            // get_rect_colors(rectangleCount);

            if (slide == 9999) break;

            soundBuf = generate_sine();
            AESND_SetVoiceStop(voice, true);
            voice = AESND_AllocateVoice(VoiceCallBack);
            AESND_PlayVoice(voice, VOICE_MONO16, soundBuf, SAMPLE_RATE * 2, SAMPLE_RATE, 0, false);
            free(soundBuf);
            for (u8 i = 0; i < rectangleCount; i++) {
                rects[i] = random_rectangle(rmode->fbWidth, rmode->efbHeight);
            }

            start = gettime();
            end = gettime();

            slide++;
        } else end = gettime();
        WPAD_ScanPads();
        const u32 wpaddown = WPAD_ButtonsDown(0);
        // const u32 wpadheld = WPAD_ButtonsHeld(0);

        if (wpaddown & WPAD_BUTTON_HOME) break;

        GRRLIB_FillScreen(GRRLIB_WHITE);
        for (u8 i = 0; i < rectangleCount; i++) {
            GRRLIB_Rectangle(
                rects[i].x,
                rects[i].y,
                rects[i].w,
                rects[i].h,
                rectColors[i],
                true
            );
        };

        sprintf(str, "geyser.dol - slide %04u", slide);
        GRRLIB_PrintfTTF(16, rmode->efbHeight - 43, font, str, 11, GRRLIB_BLACK);

        GRRLIB_Render();
    }

    AESND_FreeVoice(voice);

    GRRLIB_FreeTTF(font);
    GRRLIB_Exit();
    exit(0);
    return 0;
}



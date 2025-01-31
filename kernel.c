#include <stdint.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VIDEO_MEMORY 0xb8000

static uint8_t text_color = 0xF;  // Default: white
static uint8_t bg_color = 0x1;   // Default: blue

void clear_screen(void);

void clear_screen(void) {
    uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;
    uint16_t blank = ' ' | ((text_color | (bg_color << 4)) << 8);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        video_memory[i] = blank;
    }
}
void display_text(const char *text, uint16_t row, uint16_t col) {
    uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;
    uint16_t offset = row * SCREEN_WIDTH + col;
    while (*text) {
        video_memory[offset++] = *text | ((text_color | (bg_color << 4)) << 8);
        text++;
    }
}
void kmain(void) {
    clear_screen();
    display_text("Hello, world!", 1, 1);
    display_text("!^*&", 3, 1);
}
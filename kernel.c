#include <stdint.h>
#include <stdbool.h>
#include <string.h>  // Для работы со строками (strcmp, strtok)

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VIDEO_MEMORY 0xb8000
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

static uint8_t text_color = 0xF;  // Default: white
static uint8_t bg_color = 0x1;   // Default: blue
static uint16_t cursor_pos = 0;  // Позиция курсора
static char input_buffer[256];   // Буфер для ввода команд
static size_t input_index = 0;   // Индекс в буфере

// Прототипы функций
void clear_screen(void);
void display_text(const char *text, uint16_t row, uint16_t col);
void print_char(char c);
void update_cursor(uint16_t position);
void handle_keyboard(void);
void execute_command(const char *command);
uint16_t get_cursor_row(void);

int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}
int strncmp(const char *str1, const char *str2, size_t n) {
    while (n > 0 && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}
// Функция для чтения из порта ввода-вывода
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Функция для записи в порт ввода-вывода
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Функция для очистки экрана
void clear_screen(void) {
    uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;
    uint16_t blank = ' ' | ((text_color | (bg_color << 4)) << 8);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        video_memory[i] = blank;
    }
    cursor_pos = 0;  // Сброс позиции курсора
}

// Функция для вывода текста на экран
void display_text(const char *text, uint16_t row, uint16_t col) {
    uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;
    uint16_t offset = row * SCREEN_WIDTH + col;
    while (*text) {
        video_memory[offset++] = *text | ((text_color | (bg_color << 4)) << 8);
        text++;
    }
}

// Функция для вывода символа на экран
void print_char(char c) {
    uint16_t *video_memory = (uint16_t *)VIDEO_MEMORY;

    if (c == '\n') {
        // Переход на новую строку
        cursor_pos = (cursor_pos / SCREEN_WIDTH + 1) * SCREEN_WIDTH;
    } else {
        // Вывод символа на экран
        video_memory[cursor_pos] = c | ((text_color | (bg_color << 4)) << 8);
        cursor_pos++;
    }

    // Проверка выхода за границы экрана
    if (cursor_pos >= SCREEN_WIDTH * SCREEN_HEIGHT) {
        // Прокрутка экрана вверх (не реализована в этом примере)
        cursor_pos -= SCREEN_WIDTH;
    }
}

// Функция для обновления позиции курсора
void update_cursor(uint16_t position) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((position >> 8) & 0xFF));
}

// Функция для обработки ввода с клавиатуры
void handle_keyboard(void) {
    if ((inb(KEYBOARD_STATUS_PORT) & 0x01) == 0) {
        return;  // Если нет данных от клавиатуры, выходим
    }

    uint8_t scancode = inb(KEYBOARD_DATA_PORT);  // Считываем скан-код

    if (scancode & 0x80) {
        return;  // Игнорируем отпускание клавиши
    }

    char key = '\0';

    // Преобразуем скан-код в символ
    switch (scancode) {
        case 0x02: key = '1'; break;
        case 0x03: key = '2'; break;
        case 0x04: key = '3'; break;
        case 0x05: key = '4'; break;
        case 0x06: key = '5'; break;
        case 0x07: key = '6'; break;
        case 0x08: key = '7'; break;
        case 0x09: key = '8'; break;
        case 0x0A: key = '9'; break;
        case 0x0B: key = '0'; break;
        case 0x10: key = 'q'; break;
        case 0x11: key = 'w'; break;
        case 0x12: key = 'e'; break;
        case 0x13: key = 'r'; break;
        case 0x14: key = 't'; break;
        case 0x15: key = 'y'; break;
        case 0x16: key = 'u'; break;
        case 0x17: key = 'i'; break;
        case 0x18: key = 'o'; break;
        case 0x19: key = 'p'; break;
        case 0x1E: key = 'a'; break;
        case 0x1F: key = 's'; break;
        case 0x20: key = 'd'; break;
        case 0x21: key = 'f'; break;
        case 0x22: key = 'g'; break;
        case 0x23: key = 'h'; break;
        case 0x24: key = 'j'; break;
        case 0x25: key = 'k'; break;
        case 0x26: key = 'l'; break;
        case 0x2C: key = 'z'; break;
        case 0x2D: key = 'x'; break;
        case 0x2E: key = 'c'; break;
        case 0x2F: key = 'v'; break;
        case 0x30: key = 'b'; break;
        case 0x31: key = 'n'; break;
        case 0x32: key = 'm'; break;
        case 0x39: key = ' '; break;  // Пробел
        case 0x1C: key = '\n'; break; // Enter
        default: break;
    }

    if (key != '\0') {
        if (key == '\n') {
            // Обработка команды при нажатии Enter
            input_buffer[input_index] = '\0';  // Завершаем строку
            execute_command(input_buffer);     // Выполняем команду
            input_index = 0;                  // Сбрасываем индекс буфера
            cursor_pos = (get_cursor_row() + 1) * SCREEN_WIDTH;  // Переход на новую строку
            update_cursor(cursor_pos);        // Обновляем позицию курсора
        } else {
            // Добавляем символ в буфер и выводим на экран
            if (input_index < sizeof(input_buffer) - 1) {
                input_buffer[input_index++] = key;
                print_char(key);
            }
        }
        update_cursor(cursor_pos);  // Обновляем позицию курсора
    }
}

// Функция для выполнения команд
void execute_command(const char *command) {
    if (strcmp(command, "clear") == 0) {
        clear_screen();  // Очистка экрана
    } else if (strncmp(command, "echo ", 5) == 0) {
        // Вывод текста после команды echo
        const char *text = command + 5;  // Пропускаем "echo "
        display_text(text, get_cursor_row(), 0);
    } else {
        // Неизвестная команда
        display_text("Unknown command", get_cursor_row(), 0);
    }
}

// Функция для получения текущей строки курсора
uint16_t get_cursor_row(void) {
    return cursor_pos / SCREEN_WIDTH;
}

// Основная функция ядра
void kmain(void) {
    clear_screen();  // Очистка экрана
    display_text("Hello, world!", 1, 1);  // Вывод текста
    display_text("Type 'clear' to clear screen or 'echo <text>' to display text.", 3, 1);

    while (1) {
        handle_keyboard();  // Обработка ввода с клавиатуры
    }
}

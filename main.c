#include <stdint.h>
#include <stdnoreturn.h>

#define BYTE uint8_t
#define WORD uint16_t

#define WIDTH  320
#define HEIGHT 200
#define SECOND 1000000 /*micro-seconds*/

static void
initVGA()
{
        __asm__("mov $0x13, %%ax       \n"
                "int $0x10             \n"
                "mov $0xA000, %%ax     \n"
                "mov %%ax, %%es"
                :
                : : "ax");
}

static void
putpixel(WORD x, WORD y, BYTE color)
{
        WORD i = WIDTH * y + x;

        __asm__("mov  %1, %%di     \n"
                "movb %0, %%es:(%%di)"
                :
                : "r"(color), "r"(i)
                : "di");
}

static void
render_rect(WORD x, WORD y, WORD width, WORD height, BYTE color)
{
        for (WORD i = 0; i < width; i++) {
                for (WORD j = 0; j < height; j++) {
                        putpixel(x + i, y + j, color);
                }
        }
}

static void
sleep(uint32_t usec)
{
        WORD lo = usec;
        WORD hi = (usec >> 16);

        __asm__("mov    %0, %%cx        \n"
                "mov    %1, %%dx        \n"
                "mov $0x86, %%ah        \n"
                "int $0x15"
                :
                : "r"(hi), "r"(lo)
                : "ax", "cx", "dx");
}

static BYTE
read_key()
{
        BYTE key_code = 0;

        __asm__("mov $1, %%ah           \n"
                "int $0x16              \n"
                "jz fail                \n"
                "xor %%ah, %%ah         \n"
                "int $0x16              \n"
                "jmp end                \n"
                "fail:                  \n"
                "xor %%al, %%al         \n"
                "end:                   \n"
                "mov %%al, %0"

                : "=r"(key_code)
                : : "ax");

        return key_code;
}

extern void
_main()
{
        initVGA();

        #define BACKGROUND_COLOR        0x00
        #define PLAYER_COLOR            0x60
        #define PLAYER_WIDTH            80
        #define PLAYER_HEIGHT           2
        #define PLAYER_Y                150
        #define PlAYER_VEL              10
        #define BALL_COLOR              0x38
        #define BALL_SIZE               10
        #define BALL_VEL                2

        uint32_t FPS = 60;

        WORD x_pos = 0;
        WORD y_pos = 0;
        WORD x_vel = -BALL_VEL;
        WORD y_vel = -BALL_VEL;

        WORD player_x_pos = 0;
        BYTE game_over;

        do {
                render_rect(player_x_pos, PLAYER_Y, PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_COLOR);
                render_rect(x_pos, y_pos, BALL_SIZE, BALL_SIZE, BALL_COLOR);

                // check for collision of ball with border
                if (x_pos == 0 || (x_pos + BALL_SIZE) == WIDTH)  x_vel = -x_vel;
                if (y_pos == 0 || (y_pos + BALL_SIZE) == HEIGHT) y_vel = -y_vel;

                // check for collsion of ball with player
                if ((y_pos + BALL_SIZE) == PLAYER_Y
                    && (player_x_pos <= (x_pos + BALL_SIZE))
                    && ((player_x_pos + PLAYER_WIDTH) >= x_pos))
                {
                        y_vel = -y_vel;
                        FPS += 5;
                }

                // user-input
                BYTE code = read_key();

                if (code == 'a' && player_x_pos >= PlAYER_VEL) {
                        player_x_pos -= PlAYER_VEL;
                }

                if (code == 'd' && (player_x_pos + PLAYER_WIDTH) <= (WIDTH - PlAYER_VEL)) {
                        player_x_pos += PlAYER_VEL;
                }

                // increment
                x_pos += x_vel;
                y_pos += y_vel;

                game_over = (y_pos + BALL_SIZE >= HEIGHT);

                sleep(SECOND / FPS);
                render_rect(0, 0, WIDTH, HEIGHT, BACKGROUND_COLOR);
        }
        while (!game_over);

        _main();
}

#include <stdint.h>

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
render_score(BYTE score)
{
        #define TEXT_COLOR 0x08

        BYTE hi = '0' + (score / 10),
             lo = '0' + (score % 10);

        // set cursor pos
        __asm__("mov  $2, %ah           \n"
                "xor %bh, %bh           \n"
                "mov $0x1800, %dx       \n"
                "int $0x10");

        // render chars
        __asm__("mov   $0x0E, %%ah      \n"
                "xor    %%bh, %%bh      \n"
                "mov      %2, %%bl      \n"
                "mov      %0, %%al      \n"
                "int   $0x10            \n"
                "mov      %1, %%al      \n"
                "int   $0x10"
                :
                : "r"(hi), "r"(lo), "i"(TEXT_COLOR)
                : "ax", "bx");
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

static WORD
rand()
{
        WORD rnd;

        __asm__("xor %%ah, %%ah\n"
                "int $0x1A\n"
                "mov %%dx, %0"

                : "=r"(rnd)
                :
                : "ax", "cx", "dx");

        return rnd;
}

static BYTE
read_key()
{
        BYTE key_code = 0;

        __asm__("mov $1, %%ah           \n"
                "int $0x16              \n"
                "mov %%al, %0           \n"
                "jz end                 \n"
                "xor %%ah, %%ah         \n"
                "int $0x16              \n"
                "end:                   \n"

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
        #define PLAYER_VEL              10
        #define BALL_COLOR              0x38
        #define BALL_SIZE               10
        #define BALL_VEL                2
        #define TEXT_HEIGHT             8

        uint32_t FPS = 60;

        WORD x_pos = 10 * (rand() % (WIDTH / 10));
        WORD y_pos = 0;
        WORD x_vel = BALL_VEL;
        WORD y_vel = BALL_VEL;

        WORD player_x_pos = (WIDTH - PLAYER_WIDTH)/2;
        BYTE game_over;
        BYTE score = 0;

        do {
                render_rect(player_x_pos, PLAYER_Y, PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_COLOR);
                render_rect(x_pos, y_pos, BALL_SIZE, BALL_SIZE, BALL_COLOR);
                render_score(score);

                // increment
                x_pos += x_vel;
                y_pos += y_vel;


                // check for collision of ball with border
                if (x_pos == 0 || (x_pos + BALL_SIZE) == WIDTH)  x_vel = -x_vel;
                if (y_pos == 0 || (y_pos + BALL_SIZE) == HEIGHT) y_vel = -y_vel;


                // check for collision of ball with player
                if ((y_pos + BALL_SIZE) == PLAYER_Y
                    && (player_x_pos <= (x_pos + BALL_SIZE))
                    && ((player_x_pos + PLAYER_WIDTH) >= x_pos))
                {
                        y_vel = -y_vel;
                        FPS += 5;
                        ++score;
                }


                // user-input
                BYTE code = read_key();

                if (code == 'a' && player_x_pos >= PLAYER_VEL) {
                        player_x_pos -= PLAYER_VEL;
                }

                if (code == 'd' && (player_x_pos + PLAYER_WIDTH) <= (WIDTH - PLAYER_VEL)) {
                        player_x_pos += PLAYER_VEL;
                }


                // loop-hook
                game_over = (y_pos + BALL_SIZE >= HEIGHT);
                sleep(SECOND / FPS);

                // only re-render over text if necessary
                WORD RENDER_HEIGHT = HEIGHT;

                if (y_pos < (HEIGHT - TEXT_HEIGHT - BALL_SIZE)) {
                        RENDER_HEIGHT -= TEXT_HEIGHT;
                }

                // clear screen
                render_rect(0, 0, WIDTH, RENDER_HEIGHT, BACKGROUND_COLOR);
        }
        while (!game_over);

        _main();
}

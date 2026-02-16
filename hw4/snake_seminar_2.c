#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <inttypes.h>
#include <ctype.h>   
#include <string.h>
#include <unistd.h>

#define MIN_Y  2
enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10)};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10};
enum {MOVE_DELAY_MS = 100}; // задержка между движениями (мс)

#define CONTROLS 2 

// Здесь храним коды управления змейкой
struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
}control_buttons;

struct control_buttons default_controls[CONTROLS] = {
    {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT},
    {'s', 'w', 'a', 'd'}                      
};

/*
 Голова змейки содержит в себе
 x,y - координаты текущей позиции
 direction - направление движения
 tsize - размер хвоста
 *tail -  ссылка на хвост
 */
typedef struct snake_t
{
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail_t *tail;
    struct control_buttons controls;
} snake_t;

/*
 Хвост это массив состоящий из координат x,y
 */
typedef struct tail_t
{
    int x;
    int y;
} tail_t;

void mytimeout(long milliseconds)
{
    clock_t start = clock();
    while ((clock() - start) * 1000 / CLOCKS_PER_SEC < milliseconds);
}

void initTail(struct tail_t t[], size_t size)
{
    struct tail_t init_t={0,0};
    for(size_t i=0; i<size; i++)
    {
        t[i]=init_t;
    }
}
void initHead(struct snake_t *head, int x, int y)
{
    head->x = x;
    head->y = y;
    head->direction = RIGHT;
}

void initSnake(snake_t *head, size_t size, int x, int y)
{
    tail_t*  tail  = (tail_t*) malloc(MAX_TAIL_SIZE*sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head, x, y);
    head->tail = tail; // прикрепляем к голове хвост
    head->tsize = size+1;
    head->controls = default_controls[0]; 


    // Начальное положение тела
    head->tail[0].x = x;
    head->tail[0].y = y;
    for (int i = 1; i <= size; i++) {
        head->tail[i].x = x - i;
        head->tail[i].y = y;
    }
}

/*
 Освобождение памяти
*/
void freeSnake(snake_t *snake)
{
    free(snake->tail);
    free(snake);
}

/*
 Проверка на столкновения головы с хвостом
*/
int self_collision(snake_t *snake)
{
    for (size_t i = 1; i < snake->tsize; i++) {
        if (snake->tail[i].x == snake->x && snake->tail[i].y == snake->y)
            return 1;
    }
    return 0;
}

/*
 Движение головы с учетом текущего направления движения
 */
void go(snake_t *head)
{
    int new_x = head->x, new_y = head->y;
    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);

    switch (head->direction)
    {
        case LEFT:  new_x--; break;
        case RIGHT: new_x++; break;
        case UP:    new_y--; break;
        case DOWN:  new_y++; break;
        default: break;
    }

    // Обработка выхода за пределы экрана
    if (new_x < 0) new_x = max_x - 1;
    if (new_x >= max_x) new_x = 0;
    if (new_y < MIN_Y) new_y = max_y - 1;
    if (new_y >= max_y) new_y = MIN_Y;

    mvprintw(head->y, head->x, " ");
    head->x = new_x;
    head->y = new_y;
    mvprintw(head->y, head->x, "@");
    refresh();
}

int checkDirection(snake_t* snake, int32_t key) {
    int new_direction = -1;

    for (int i = 0; i < CONTROLS; i++) {
        struct control_buttons ctrl = default_controls[i];
        #define CMP(k1, k2) ((k1) == (k2) || (isalpha(k1) && tolower(k1) == tolower(k2)))

        if (CMP(key, ctrl.down))  new_direction = DOWN;
        else if (CMP(key, ctrl.up))    new_direction = UP;
        else if (CMP(key, ctrl.right)) new_direction = RIGHT;
        else if (CMP(key, ctrl.left))  new_direction = LEFT;

        if (new_direction != -1) break;
    }

    if (new_direction == -1) return 1;

    if ((snake->direction == LEFT && new_direction == RIGHT) ||
        (snake->direction == RIGHT && new_direction == LEFT) ||
        (snake->direction == UP && new_direction == DOWN) ||
        (snake->direction == DOWN && new_direction == UP))
        return 0;

    return 1;
}

void changeDirection(struct snake_t* snake, const int32_t key)
{
    int new_direction = -1;

    for (int i = 0; i < CONTROLS; i++) {
        struct control_buttons ctrl = default_controls[i];
        #define CMP(k1, k2) ((k1) == (k2) || (isalpha(k1) && tolower(k1) == tolower(k2)))

        if (CMP(key, ctrl.down))  new_direction = DOWN;
        else if (CMP(key, ctrl.up))    new_direction = UP;
        else if (CMP(key, ctrl.right)) new_direction = RIGHT;
        else if (CMP(key, ctrl.left))  new_direction = LEFT;

        if (new_direction != -1) break;
    }

    if (new_direction == -1) return;

    if (checkDirection(snake, key)) {
        snake->direction = new_direction;
    }
}

/*
 Движение хвоста с учетом движения головы
 */
void goTail(struct snake_t *head)
{
   mvprintw(head->tail[head->tsize - 1].y, head->tail[head->tsize - 1].x, " ");
    for (size_t i = head->tsize - 1; i > 0; i--)
    {
        head->tail[i] = head->tail[i - 1];
        if (head->tail[i].y || head->tail[i].x)
            mvprintw(head->tail[i].y, head->tail[i].x, "*");
    }

    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

/*
 Отрисовка начальной змейки
 */
void drawSnake(snake_t *snake)
{
    mvprintw(snake->y, snake->x, "@");
    for (size_t i = 1; i < snake->tsize; i++)
        mvprintw(snake->tail[i].y, snake->tail[i].x, "*");
    refresh();
}

int main()
{
    snake_t *snake = (snake_t*)malloc(sizeof(snake_t));
    initSnake(snake, START_TAIL_SIZE, 10, 10);
    initscr();
    keypad(stdscr, TRUE); // Включаем F1, F2, стрелки и т.д.
    raw();                // Откдючаем line buffering
    noecho();            // Отключаем echo() режим при вызове getch
    curs_set(FALSE);    //Отключаем курсор
    mvprintw(0, 0,"Use arrows for control. Press 'F10' for EXIT");
    nodelay(stdscr, TRUE); 

    drawSnake(snake);  
    int key_pressed=0;
    int game_over = 0;

    while (!game_over && key_pressed != STOP_GAME)
    {
        key_pressed = getch();
        if (key_pressed != ERR)
            changeDirection(snake, key_pressed);

        go(snake);
        goTail(snake);

        if (self_collision(snake))
            game_over = 1;

        // Замена timeout на clock
        mytimeout(MOVE_DELAY_MS);
    }

    if (game_over)
    {
        clear();
        mvprintw(0, 0, "GAME OVER! Press any key to exit...");
        refresh();
        nodelay(stdscr, FALSE);
        getch();
    }

    freeSnake(snake);
    endwin(); // Завершаем режим curses mod
    return 0;
}
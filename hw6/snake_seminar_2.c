#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MIN_Y  2
double DELAY = 0.1;
#define PLAYERS  2

enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10)};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10,SEED_NUMBER=3,CONTROLS=2};


// Здесь храним коды управления змейкой
struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
} control_buttons;

struct control_buttons default_controls[CONTROLS] = {{KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT},
                                                    {'s', 'w', 'a', 'd'}};

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
    int player_id; 
    int alive; 
} snake_t;

/*
 Хвост это массив состоящий из координат x,y
 */
typedef struct tail_t
{
    int x;
    int y;
} tail_t;
/*
 Еда — это массив точек, состоящий из координат x,y, времени,
 когда данная точка была установлена, и поля, сигнализирующего,
 была ли данная точка съедена.
 */
struct food
{
    int x;
    int y;
    time_t put_time;
    char point;
    uint8_t enable;
} food[MAX_FOOD_SIZE];

/*
 Выбор цвета для элемента
 */
void setColor(int objectType)
{
    attroff(COLOR_PAIR(1));
    attroff(COLOR_PAIR(2));
    attroff(COLOR_PAIR(3));
    switch (objectType)
    {
        case 1:      // SNAKE1
            attron(COLOR_PAIR(1));
            break;
        case 2:      // SNAKE2
            attron(COLOR_PAIR(2));
            break;
        case 3:      // FOOD
            attron(COLOR_PAIR(3));
            break;
        default:
            break;
    }
}


void initFood(struct food f[], size_t size)
{
    struct food init = {0,0,0,0,0};
    for(size_t i=0; i<size; i++)
    {
        f[i] = init;
    }
}
/*
 Обновить/разместить текущее зерно на поле
 */
void putFoodSeed(struct food *fp)
{
    int max_x=0, max_y=0;
    char spoint[2] = {0};
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(fp->y, fp->x, " ");
    fp->x = rand() % (max_x - 1);
    fp->y = rand() % (max_y - 2) + 1; //Не занимаем верхнюю строку
    fp->put_time = time(NULL);
    fp->point = '$';
    fp->enable = 1;
    spoint[0] = fp->point;
    setColor(3);
    mvprintw(fp->y, fp->x, "%s", spoint);
}

/*
 Разместить еду на поле
 */
void putFood(struct food f[], size_t number_seeds)
{
    for(size_t i=0; i<number_seeds; i++)
    {
        putFoodSeed(&f[i]);
    }
}

void refreshFood(struct food f[], int nfood)
{
    for(size_t i=0; i<nfood; i++)
    {
        if( f[i].put_time )
        {
            if( !f[i].enable || (time(NULL) - f[i].put_time) > FOOD_EXPIRE_SECONDS )
            {
                putFoodSeed(&f[i]);
            }
        }
    }
}
void initTail(struct tail_t t[], size_t size)
{
    struct tail_t init_t= {0,0};
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
//========================================================================
void initSnake(snake_t *head[], size_t size, int x, int y,int i)
{
    head[i]    = (snake_t*)malloc(sizeof(snake_t));
    tail_t*  tail  = (tail_t*) malloc(MAX_TAIL_SIZE*sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head[i], x, y);
    head[i]->tail     = tail; // прикрепляем к голове хвост
    head[i]->tsize    = size+1;
    head[i]->controls = default_controls[i];
    head[i]->player_id = i+1; 
    head[i]->alive = 1;
}

/*
 Движение головы с учетом текущего направления движения
 */
void go(struct snake_t *head)
{
    char ch = '@';
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);

    mvprintw(head->y, head->x, " ");

    switch (head->direction)
    {
    case LEFT:
        head->x--;
        if (head->x < 0)
            head->x = max_x - 1;
        break;
    case RIGHT:
        head->x++;
        if (head->x >= max_x)
            head->x = 0;
        break;
    case UP:
        head->y--;
        if (head->y < 1)
            head->y = max_y - 1;
        break;
    case DOWN:
        head->y++;
        if (head->y >= max_y)
            head->y = 1;
        break;
    default:
        break;
    }

    setColor(head->player_id);
    mvprintw(head->y, head->x, "%c", ch);
}

void changeDirection(struct snake_t* snake, const int32_t key)
{
    int new_dir = -1;

    if (key == snake->controls.down || (key < 0x100 && tolower(key) == tolower(snake->controls.down)))
        new_dir = DOWN;
    else if (key == snake->controls.up || (key < 0x100 && tolower(key) == tolower(snake->controls.up)))
        new_dir = UP;
    else if (key == snake->controls.right || (key < 0x100 && tolower(key) == tolower(snake->controls.right)))
        new_dir = RIGHT;
    else if (key == snake->controls.left || (key < 0x100 && tolower(key) == tolower(snake->controls.left)))
        new_dir = LEFT;

    if (new_dir != -1) {
        if (!((snake->direction == LEFT && new_dir == RIGHT) ||
              (snake->direction == RIGHT && new_dir == LEFT) ||
              (snake->direction == UP && new_dir == DOWN) ||
              (snake->direction == DOWN && new_dir == UP))) {
            snake->direction = new_dir;
        }
    }
}

/*
 Движение хвоста с учетом движения головы
 */
void goTail(struct snake_t *head)
{
    char ch = '*';
    mvprintw(head->tail[head->tsize-1].y, head->tail[head->tsize-1].x, " ");
    for(size_t i = head->tsize-1; i>0; i--)
    {
        head->tail[i] = head->tail[i-1];
        if( head->tail[i].y || head->tail[i].x)
            setColor(head->player_id); 
            mvprintw(head->tail[i].y, head->tail[i].x, "%c", ch);
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

//========================================================================
//Проверка того, является ли какое-то из зерен съеденным,
_Bool haveEat(struct snake_t *head, struct food f[])
{
    for (int i = 0; i < MAX_FOOD_SIZE; i++) {
        if (f[i].enable && f[i].x == head->x && f[i].y == head->y) {
            f[i].enable = 0;
            return 1;
        }
    }
}

/*
 Увеличение хвоста на 1 элемент
 */

void addTail(struct snake_t *head)
{
    if (head->tsize < MAX_TAIL_SIZE) {
        head->tail[head->tsize] = head->tail[head->tsize - 1];
        head->tsize++;
    }
}
//========================================================================

_Bool isCrush(snake_t * snake)
{
    for (size_t i = 1; i < snake->tsize; i++) {
        if (snake->tail[i].x == snake->x && snake->tail[i].y == snake->y)
            return 1;
    }
    return 0;
}
//========================================================================

void repairSeed(struct food f[], size_t nfood, struct snake_t *head)
{
    for( size_t i=0; i<head->tsize; i++ )
        for (size_t j = 0; j < nfood; j++) {
            if (f[j].enable && head->tail[i].x == f[j].x && head->tail[i].y == f[j].y) {
                putFoodSeed(&f[j]);
            }
        }
    for( size_t i=0; i<nfood; i++ )
        for (size_t j = i + 1; j < nfood; j++) {
            if (f[i].enable && f[j].enable && f[i].x == f[j].x && f[i].y == f[j].y) {
                putFoodSeed(&f[j]);
            }
        }
}

void update_collisions(snake_t *snakes[], int n) {
    for (int i = 0; i < n; i++) {
        if (snakes[i]->alive && isCrush(snakes[i])) {
            snakes[i]->alive = 0;
        }
    }

    for (int i = 0; i < n; i++) {
        if (!snakes[i]->alive) continue;
        for (int j = i + 1; j < n; j++) {
            if (!snakes[j]->alive) continue;

            if (snakes[i]->x == snakes[j]->x && snakes[i]->y == snakes[j]->y) {
                snakes[i]->alive = 0;
                snakes[j]->alive = 0;
                continue;
            }
            for (size_t k = 1; k < snakes[j]->tsize; k++) {
                if (snakes[i]->x == snakes[j]->tail[k].x && snakes[i]->y == snakes[j]->tail[k].y) {
                    snakes[i]->alive = 0;
                    break;
                }
            }

            if (snakes[i]->alive) {
                for (size_t k = 1; k < snakes[i]->tsize; k++) {
                    if (snakes[j]->x == snakes[i]->tail[k].x && snakes[j]->y == snakes[i]->tail[k].y) {
                        snakes[j]->alive = 0;
                        break;
                    }
                }
            }
        }
    }
}
//========================================================================
//Вынести тело цикла while из int main() в отдельную функцию update
//и посмотреть, как изменится профилирование
void update(struct snake_t *head, struct food f[], const int32_t key)
{
    go(head);
    goTail(head);
    changeDirection(head, key);
    refreshFood(food, SEED_NUMBER);// Обновляем еду
    if (haveEat(head,food))
    {
        addTail(head);
    }
    refresh();//Обновление экрана, вывели кадр анимации
    napms((int)(DELAY * 1000));
}

// ==================== МЕНЮ ====================
#define NUM_COLORS 7
int color_values[NUM_COLORS] = {
    COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE,
    COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE
};
char *color_names[NUM_COLORS] = {
    "Red", "Green", "Yellow", "Blue",
    "Magenta", "Cyan", "White"
};

void main_menu(int *color1_idx, int *color2_idx) {
    int ch;
    int running = 1;
    nodelay(stdscr, FALSE);
    while (running) {
        attron(COLOR_PAIR(0));    
        clear();
        mvprintw(2, 10, "=== SNAKE GAME ===");
        mvprintw(4, 5, "1. Snake 1 color: %s (press 1 to change)", color_names[*color1_idx]);
        mvprintw(5, 5, "2. Snake 2 color: %s (press 2 to change)", color_names[*color2_idx]);
        mvprintw(7, 5, "Controls:");
        mvprintw(8, 7, "Snake 1: Arrow keys");
        mvprintw(9, 7, "Snake 2: WASD (any case)");
        mvprintw(10, 7, "During game: F10 or Backspace to return to menu");
        mvprintw(12, 5, "Press 'S' to START game");
        mvprintw(13, 5, "Press 'Q' to QUIT");
        mvprintw(15, 5, "Your choice: ");
        refresh();

        ch = getch();
        if (ch == '1') {
            *color1_idx = (*color1_idx + 1) % NUM_COLORS;
        } else if (ch == '2') {
            *color2_idx = (*color2_idx + 1) % NUM_COLORS;
        } else if (ch == 's' || ch == 'S') {
            running = 0;
        } else if (ch == 'q' || ch == 'Q') {
            endwin();
            exit(0);
        }
    }

    nodelay(stdscr, TRUE);
    clear();
}

// ==================== ЭКРАН РЕЗУЛЬТАТА ====================
void show_game_over(snake_t *snakes[]) {
    nodelay(stdscr, FALSE); 
    standend();   
    attron(COLOR_PAIR(0));    
    clear();
    mvprintw(5, 10, "=== GAME OVER ===");

    int alive_count = 0;
    int winner = -1;
    for (int i = 0; i < PLAYERS; i++) {
        if (snakes[i]->alive) {
            alive_count++;
            winner = i;
        }
    }

    if (alive_count == 1) {
        int score = snakes[winner]->tsize - START_TAIL_SIZE;
        mvprintw(7, 10, "Player %d (Snake %d) wins!", winner+1, winner+1);
        mvprintw(8, 10, "Score: %d", score);
    } else if (alive_count == 0) {
        int score1 = snakes[0]->tsize - START_TAIL_SIZE;
        int score2 = snakes[1]->tsize - START_TAIL_SIZE;
        if (score1 > score2) {
            mvprintw(7, 10, "Player 1 wins by score!");
            mvprintw(8, 10, "Score: %d vs %d", score1, score2);
        } else if (score2 > score1) {
            mvprintw(7, 10, "Player 2 wins by score!");
            mvprintw(8, 10, "Score: %d vs %d", score2, score1);
        } else {
            mvprintw(7, 10, "It's a tie! Both died.");
            mvprintw(8, 10, "Score: %d each", score1);
        }
    }

    mvprintw(10, 10, "Press any key to return to menu...");
    refresh();
    getch();

    nodelay(stdscr, TRUE);
    clear();
}

// ==================== ИГРА ====================
void start_game(int color1_val, int color2_val) {
    init_pair(0, COLOR_WHITE, COLOR_BLACK); 
    init_pair(1, color1_val, COLOR_BLACK);
    init_pair(2, color2_val, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);

    snake_t* snakes[PLAYERS];
    for (int i = 0; i < PLAYERS; i++)
        initSnake(snakes, START_TAIL_SIZE, 10 + i*10, 10 + i*10, i);

    initFood(food, MAX_FOOD_SIZE);
    putFood(food, SEED_NUMBER);

    int key_pressed = 0;
    int game_running = 1;
    while (game_running && key_pressed != STOP_GAME) {
        key_pressed = getch();

        for (int i = 0; i < PLAYERS; i++) {
            if (snakes[i]->alive) {
                update(snakes[i], food, key_pressed);
            }
        }

        update_collisions(snakes, PLAYERS);
        int alive_count = 0;
        for (int i = 0; i < PLAYERS; i++)
            if (snakes[i]->alive) alive_count++;

        if (alive_count < 2) {
            game_running = 0;
        }

        if (key_pressed == KEY_BACKSPACE || key_pressed == 8 || key_pressed == 127)
            game_running = 0;
    }

    show_game_over(snakes);

    for (int i = 0; i < PLAYERS; i++) {
        free(snakes[i]->tail);
        free(snakes[i]);
    }
    clear();
}


int main()
{
    initscr();
    keypad(stdscr, TRUE); // Включаем F1, F2, стрелки и т.д.
    raw();                // Откдючаем line buffering
    noecho();            // Отключаем echo() режим при вызове getch
    curs_set(FALSE);    //Отключаем курсор
    nodelay(stdscr, TRUE); 

    if (has_colors() == FALSE) {
        endwin();
        printf("Ваш терминал не поддерживает цвет\n");
        return 1;
    }
    start_color();

    int color1_idx = 0;
    int color2_idx = 3;
    
    while (1) {
        main_menu(&color1_idx, &color2_idx);
        start_game(color_values[color1_idx], color_values[color2_idx]);
    }

    endwin();
    return 0;
}
/*
 * 1. Доработать функционал игры "Змейка", реализовать в игре ситуацию, когда змея врезается сама в себя.
 * 2.Выход за границы экрана.
 * 3. * Переписать timeout через clock.
 * */

/*
 *hw-6
 *Tasks
 *1. Реализовать пропущенный код
 *2. Внести изменения в исполняемый файл, чтобы в стандартном режиме змейка не погибала при самопересечении. Для этого необходимо найти вызов функции isCrush() и поменять вызов на нужное нам возвращаемое значение 0
 *3. Добавить цвет для двух змеек и еды.
 *4. Добавить стартовое меню (приветствие, выбор режима, выбор цвета змейки и тд)
 *Написать функцию void startMenu()
 *5. Сделать свои игровые механики
 *
 *
 * Игра "Змейка"
 * Функционал:
 * 1. Движение змейки
 * 2. Поедание еды
 * 3. Проверка столкновений с хвостом и границами
 * 4. Цветная змейка (зелёная)
 * 5. Окончание игры с выводом сообщения Game Over
 */



/*
 * 1. Develop Snake game functionality, implement situation when snake crashes into itself.
 * 2. Screen boundary exit.
 * 3. * Rewrite timeout using clock.
 * */

/*
 * hw-6
 * Tasks
 * 1. Implement missing code
 * 2. Modify executable so that in standard mode snake doesn't die from self-intersection. For this, find isCrush() function call and change it to return value 0
 * 3. Add colors for two snakes and food.
 * 4. Add start menu (greeting, mode selection, snake color selection, etc.)
 * Write function void startMenu()
 * 5. Create your own game mechanics
 *
 *
 * Snake Game
 * Features:
 * 1. Snake movement
 * 2. Eating food
 * 3. Collision detection with tail and boundaries
 * 4. Colored snake (green)
 * 5. Game over with message
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses/ncurses.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h> // for toupper()

#define NUM_SNAKES 2
#define MIN_Y  2  // Minimum Y coordinate to avoid writing over instructions
enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10)};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10};

// Structure for control keys
struct control_buttons {
    int down;
    int up;
    int left;
    int right;
};

// Standard control keys
struct control_buttons default_controls[NUM_SNAKES] = {
    {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT},     // arrows for first snake
    {'s', 'w', 'a', 'd'}                         // WASD for second snake
};

// Structure for one tail segment
typedef struct tail_t {
    int x;
    int y;
} tail_t;

// Snake structure
typedef struct snake_t {
    int x;  // head X coordinate
    int y;  // head Y coordinate
    int direction; // movement direction
    size_t tsize;  // current tail size
    struct tail_t *tail; // tail array
    struct control_buttons controls; // control keys
    int color_pair; // snake color
    int score;      // player score
    int is_alive;   // whether snake is alive
} snake_t;

// Food structure
typedef struct food_t {
    int x;
    int y;
    char symbol;
} food_t;

// Global settings variables
int snake_colors[NUM_SNAKES] = {1, 6}; // GREEN and CYAN by default
int game_running = 1;
int can_eat_snakes = 1; // Flag if snakes can eat each other
int sound_enabled = 1;   // Flag if sound is enabled

// Function to play sound signal
void playSound(int type) {
    if (!sound_enabled) return;

    switch(type) {
        case 1: // Food
            printf("\a"); // System beep
            fflush(stdout);
            break;
        case 2: // Snake eaten
            for(int i = 0; i < 3; i++) {
                printf("\a");
                fflush(stdout);
                usleep(100000);
            }
            break;
        case 3: // Game over
            for(int i = 0; i < 5; i++) {
                printf("\a");
                fflush(stdout);
                usleep(50000);
            }
            break;
        case 4: // Game start
            for(int i = 0; i < 2; i++) {
                printf("\a");
                fflush(stdout);
                usleep(200000);
            }
            break;
    }
}

// Initialize tail (zero coordinates)
void initTail(tail_t t[], size_t size) {
    tail_t init_t = {0,0};
    for(size_t i=0; i<size; i++) t[i] = init_t;
}

// Initialize snake head
void initHead(snake_t *head, int x, int y) {
    head->x = x;
    head->y = y;
    head->direction = RIGHT; // initial direction right
    head->score = 0;
    head->is_alive = 1;
}

// Initialize whole snake
void initSnake(snake_t *head, size_t size, int x, int y, int color_pair) {
    tail_t *tail = (tail_t*) malloc(MAX_TAIL_SIZE*sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head, x, y);
    head->tail = tail;
    head->tsize = size+1; // initial tail
    head->color_pair = color_pair;
}

// Initialize all snakes
void initAllSnakes(snake_t snakes[], size_t num_snakes, size_t start_size) {
    for (size_t i = 0; i < num_snakes; i++) {
        int start_x = 10 + i * 20;  // Different start positions
        int start_y = 10 + i * 5;
        initSnake(&snakes[i], start_size, start_x, start_y, snake_colors[i]);
        snakes[i].controls = default_controls[i];  // Different controls
    }
}

// Generate food at random position
void spawnFood(food_t *food, int max_x, int max_y) {
    food->x = rand() % max_x;
    food->y = (rand() % (max_y - MIN_Y)) + MIN_Y;
    food->symbol = '#'; // food symbol
}

// Draw food on screen
void drawFood(food_t *food) {
    attron(COLOR_PAIR(8)); // Color for food (red)
    mvprintw(food->y, food->x, "%c", food->symbol);
    attroff(COLOR_PAIR(8));
}

// Check if snake ate food
int checkFood(snake_t *snake, food_t *food) {
    if(snake->x == food->x && snake->y == food->y) {
        snake->tsize++; // increase tail
        snake->score += 10; // add points
        playSound(1); // Food eating sound
        return 1;       // food eaten
    }
    return 0;
}

// Move snake head and draw
void go(snake_t *head) {
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x); // terminal sizes

    // Erase previous head position
    mvprintw(head->y, head->x, " ");

    // Move head depending on direction
    switch(head->direction){
        case LEFT:
            head->x = (head->x <= 0) ? max_x - 1 : head->x - 1;
            break;
        case RIGHT:
            head->x = (head->x >= max_x - 1) ? 0 : head->x + 1;
            break;
        case UP:
            head->y = (head->y <= MIN_Y) ? max_y - 1 : head->y - 1;
            break;
        case DOWN:
            head->y = (head->y >= max_y - 1) ? MIN_Y : head->y + 1;
            break;
    }
}

// Change direction for all snakes
void changeAllDirections(snake_t snakes[], size_t num_snakes, const int32_t key) {
    for (size_t i = 0; i < num_snakes; i++) {
        if (!snakes[i].is_alive) continue; // Skip dead snakes

        // For second snake (WASD) handle both English and Russian layouts
        if (i == 1) {
            // Handle WASD in any layout
            switch(key) {
                // English layout (lowercase)
                case 'w': case 'W':
                // Russian layout
                case 1094: case 1062: // Ц ц in Russian layout
                    snakes[i].direction = UP;
                    break;

                case 's': case 'S':
                // Russian layout
                case 1099: case 1067: // Ы ы in Russian layout
                    snakes[i].direction = DOWN;
                    break;

                case 'a': case 'A':
                // Russian layout
                case 1092: case 1060: // Ф ф in Russian layout
                    snakes[i].direction = LEFT;
                    break;

                case 'd': case 'D':
                // Russian layout
                case 1074: case 1042: // В в in Russian layout
                    snakes[i].direction = RIGHT;
                    break;
            }
        } else {
            // For first snake use arrows (layout independent)
            if (key == snakes[i].controls.down)
                snakes[i].direction = DOWN;
            else if (key == snakes[i].controls.up)
                snakes[i].direction = UP;
            else if (key == snakes[i].controls.right)
                snakes[i].direction = RIGHT;
            else if (key == snakes[i].controls.left)
                snakes[i].direction = LEFT;
        }
    }
}

// Move snake tail
void goTail(snake_t *head){
    if (!head->is_alive) return; // Don't move dead snake's tail

    // Erase last tail segment
    mvprintw(head->tail[head->tsize-1].y, head->tail[head->tsize-1].x," ");

    // Shift all tail segments
    for(size_t i = head->tsize-1; i>0; i--){
        head->tail[i] = head->tail[i-1];
    }

    // First tail segment takes previous head position
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

// Draw all snakes with different colors
void drawAllSnakes(snake_t snakes[], size_t num_snakes) {
    for (size_t i = 0; i < num_snakes; i++) {
        if (!snakes[i].is_alive) continue; // Don't draw dead snakes

        attron(COLOR_PAIR(snakes[i].color_pair));  // Use selected color

        // Draw head
        mvprintw(snakes[i].y, snakes[i].x, "@");

        // Draw tail
        for (size_t j = 0; j < snakes[i].tsize; j++) {
            if (snakes[i].tail[j].y || snakes[i].tail[j].x) {
                mvprintw(snakes[i].tail[j].y, snakes[i].tail[j].x, "*");
            }
        }

        attroff(COLOR_PAIR(snakes[i].color_pair));
    }
}

// Check food eating for all snakes
int checkAllFood(snake_t snakes[], size_t num_snakes, food_t *food) {
    for (size_t i = 0; i < num_snakes; i++) {
        if (!snakes[i].is_alive) continue;
        if (checkFood(&snakes[i], food)) {
            return 1;
        }
    }
    return 0;
}

// Check head collision with own tail
int isCrush(snake_t *snake) {
    // Always return 0 to disable death from self-intersection
    return 0;
}

// Check screen boundary exit
int checkWallCollision(snake_t *snake, int max_x, int max_y) {
    if (snake->x < 0 || snake->x >= max_x || snake->y < MIN_Y || snake->y >= max_y) {
        return 1; // boundary collision
    }
    return 0;
}

// Check if one snake can eat another (catch from behind)
int checkSnakeEatSnake(snake_t snakes[], size_t num_snakes) {
    int snake_eaten = 0;

    for (size_t i = 0; i < num_snakes; i++) {
        if (!snakes[i].is_alive) continue;

        for (size_t j = 0; j < num_snakes; j++) {
            if (i == j || !snakes[j].is_alive) continue; // Skip self and dead

            // Check if head of snake i is on position of snake j's head
            if (snakes[i].x == snakes[j].x && snakes[i].y == snakes[j].y) {
                // Snake i ate snake j
                snakes[j].is_alive = 0;
                snakes[i].score += 50; // Bonus for eating another snake
                snake_eaten = 1;
                playSound(2); // Snake eating sound

                // Display eating message
                int max_x = 0, max_y = 0;
                getmaxyx(stdscr, max_y, max_x);
                mvprintw(3, 0, "Snake %zd ate snake %zd! +50 points!", i + 1, j + 1);
            }

            // Check if head of snake i is on tail of snake j
            for (size_t k = 0; k < snakes[j].tsize; k++) {
                if (snakes[i].x == snakes[j].tail[k].x && snakes[i].y == snakes[j].tail[k].y) {
                    // Snake i ate part of snake j's tail
                    snakes[j].tsize = k; // Shorten snake j's tail
                    snakes[i].score += 20; // Bonus for eating tail
                    snake_eaten = 1;
                    playSound(1); // Tail eating sound

                    // Display tail eating message
                    int max_x = 0, max_y = 0;
                    getmaxyx(stdscr, max_y, max_x);
                    mvprintw(4, 0, "Snake %zd bit tail of snake %zd! +20 points!", i + 1, j + 1);
                    break;
                }
            }
        }
    }

    return snake_eaten;
}

// Check all collisions for all snakes
int checkAllCollisions(snake_t snakes[], size_t num_snakes, int max_x, int max_y) {
    int alive_count = 0;

    for (size_t i = 0; i < num_snakes; i++) {
        if (!snakes[i].is_alive) continue;
        alive_count++;

        // Check wall collision (ENABLED)
        if (checkWallCollision(&snakes[i], max_x, max_y)) {
            snakes[i].is_alive = 0;
            playSound(3); // Wall collision sound
            mvprintw(2, 0, "Snake %zd crashed into wall!              ", i + 1);
        }

        // Check self-intersection (DISABLED - isCrush always returns 0)
        if (isCrush(&snakes[i])) {
            // This code never executes as isCrush always returns 0
            snakes[i].is_alive = 0;
        }
    }

    // Check snakes eating each other
    if (can_eat_snakes) {
        checkSnakeEatSnake(snakes, num_snakes);
    }

    // If only one snake or none left - game over
    return (alive_count <= 1);
}

// Function to display welcome screen
void showWelcomeScreen() {
    clear();

    // Title
    attron(A_BOLD);
    mvprintw(2, 15, "=== DOUBLE SNAKE GAME ===");
    attroff(A_BOLD);

    // Game description
    mvprintw(4, 5, "Welcome to 'Double Snake' game!");
    mvprintw(5, 5, "Control two snakes simultaneously and collect food.");

    // Main menu functions
    mvprintw(7, 5, "MAIN MENU FUNCTIONS:");
    mvprintw(8, 8, "• Color selection for each snake");
    mvprintw(9, 8, "• Control configuration");
    mvprintw(10, 8, "• Start game with selected settings");
    mvprintw(11, 8, "• Restart menu");
    mvprintw(12, 8, "• Sound on/off");
    mvprintw(13, 8, "• Exit game");

    // In-game controls
    mvprintw(15, 5, "IN-GAME CONTROLS:");
    mvprintw(16, 8, "Snake 1: Arrows ← ↑ → ↓");
    mvprintw(17, 8, "Snake 2: WASD (any layout)");
    mvprintw(18, 8, "Exit: F10");

    // Game features
    mvprintw(20, 5, "GAME FEATURES:");
    mvprintw(21, 8, "• Two independent snakes");
    mvprintw(22, 8, "• No death from self-intersection");
    mvprintw(23, 8, "• Teleportation through boundaries");
    mvprintw(24, 8, "• Colorful graphics");
    mvprintw(25, 8, "• Sound effects");
    mvprintw(26, 8, "• You can eat another snake if you catch it!");
    mvprintw(27, 8, "• +50 points for eating snake, +20 for tail");

    // Menu controls
    mvprintw(29, 5, "MENU CONTROLS:");
    mvprintw(30, 8, "Arrow keys ↑↓ - Navigate options");
    mvprintw(31, 8, "Arrow keys ←→ - Select color");
    mvprintw(32, 8, "TAB - Switch between snakes");
    mvprintw(33, 8, "ENTER - Select option");
    mvprintw(34, 8, "M - Toggle sound");
    mvprintw(35, 8, "Q or F10 - Exit");

    // Development info
    mvprintw(37, 5, "DEVELOPMENT:");
    mvprintw(38, 8, "Game created as educational project");
    mvprintw(39, 8, "using ncurses library");

    // Continue prompt
    attron(A_BOLD);
    mvprintw(41, 15, "Press any key to continue...");
    attroff(A_BOLD);

    refresh();
    getch(); // Wait for any key press
}

// Function to display start menu
int startMenu() {
    int selected_snake = 0; // 0 - first snake, 1 - second snake
    int selected_option = 0; // 0 - start game, 1 - restart, 2 - sound toggle, 3 - exit
    int color_index[NUM_SNAKES] = {0, 4}; // initial color indices

    // Array of available colors (using ncurses constants)
    int available_colors[] = {
        1, 2, 4, 3, 5, 6, 7  // GREEN, RED, BLUE, YELLOW, MAGENTA, CYAN, WHITE
    };
    char* color_names[] = {
        "Green", "Red", "Blue", "Yellow", "Magenta", "Cyan", "White"
    };
    int num_colors = 7;

    int key;

    while(1) {
        clear();

        // Title
        attron(A_BOLD);
        mvprintw(2, 10, "=== DOUBLE SNAKE GAME ===");
        attroff(A_BOLD);

        // Sound status
        mvprintw(3, 5, "Sound: %s", sound_enabled ? "ON" : "OFF");

        // Select snake for color configuration
        mvprintw(5, 5, "Select snake for color configuration:");
        for (int i = 0; i < NUM_SNAKES; i++) {
            if (i == selected_snake) {
                attron(A_REVERSE);
            }
            mvprintw(7, 5 + i * 20, "Snake %d", i + 1);
            if (i == selected_snake) {
                attroff(A_REVERSE);
            }
        }

        // Color selection for selected snake
        mvprintw(9, 5, "Color for snake %d:", selected_snake + 1);
        for (int i = 0; i < num_colors; i++) {
            attron(COLOR_PAIR(available_colors[i]));
            if (color_index[selected_snake] == i) {
                attron(A_REVERSE);
            }
            mvprintw(11, 5 + i * 12, "[%s]", color_names[i]);
            if (color_index[selected_snake] == i) {
                attroff(A_REVERSE);
            }
            attroff(COLOR_PAIR(available_colors[i]));
        }

        // Current control settings
        mvprintw(13, 5, "Current controls:");
        mvprintw(14, 8, "Snake 1: Arrow keys");
        mvprintw(15, 8, "Snake 2: WASD keys");

        // New mechanic
        mvprintw(16, 5, "New mechanic:");
        mvprintw(17, 8, "You can eat another snake! +50 points");

        // Menu options
        mvprintw(19, 5, "Options:");
        const char* options[] = {"Start Game", "Restart", "Sound ON/OFF", "Exit"};
        for (int i = 0; i < 4; i++) {
            if (i == selected_option) {
                attron(A_REVERSE);
            }
            mvprintw(21 + i, 8, "%s", options[i]);
            if (i == selected_option) {
                attroff(A_REVERSE);
            }
        }

        // Menu control instructions
        mvprintw(26, 5, "Menu controls:");
        mvprintw(27, 8, "Arrow keys ↑↓ - navigation, ENTER - select, TAB - switch snakes");
        mvprintw(28, 8, "M - toggle sound");

        refresh();

        key = getch();

        switch(key) {
            case KEY_UP:
                selected_option = (selected_option - 1 + 4) % 4;
                break;
            case KEY_DOWN:
                selected_option = (selected_option + 1) % 4;
                break;
            case KEY_LEFT:
                if (selected_option == 0) {
                    color_index[selected_snake] = (color_index[selected_snake] - 1 + num_colors) % num_colors;
                }
                break;
            case KEY_RIGHT:
                if (selected_option == 0) {
                    color_index[selected_snake] = (color_index[selected_snake] + 1) % num_colors;
                }
                break;
            case '\t': // TAB
                selected_snake = (selected_snake + 1) % NUM_SNAKES;
                break;
            case 'm':
            case 'M':
                sound_enabled = !sound_enabled;
                if (sound_enabled) {
                    playSound(4); // Sound on sound
                }
                break;
            case '\n': // ENTER
            case '\r':
                // Save selected colors
                for (int i = 0; i < NUM_SNAKES; i++) {
                    snake_colors[i] = available_colors[color_index[i]];
                }

                if (selected_option == 0) {
                    playSound(4); // Game start sound
                    return 1; // Start game
                }
                if (selected_option == 1) return 2; // Restart
                if (selected_option == 2) {
                    sound_enabled = !sound_enabled;
                    if (sound_enabled) {
                        playSound(4); // Sound on sound
                    }
                    continue; // Stay in menu
                }
                if (selected_option == 3) return 0; // Exit
                break;
            case 'q':
            case 'Q':
                return 0; // Exit with Q
            case STOP_GAME:
                return 0; // Exit with F10
        }
    }
}

// Main game function
void playGame() {
    // Initialize two snakes
    snake_t snakes[NUM_SNAKES];
    initAllSnakes(snakes, NUM_SNAKES, START_TAIL_SIZE);

    // Get screen dimensions
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);

    // Create first food
    food_t food;
    spawnFood(&food, max_x, max_y);

    int key_pressed = 0;
    int game_over = 0;
    timeout(100);

    // Start background music (simulation)
    if (sound_enabled) {
        playSound(4); // Game start sound
    }

    // ==================== MAIN GAME LOOP ======================
    while (key_pressed != STOP_GAME && !game_over && game_running) {
        key_pressed = getch();          // read pressed key

        // Check sound toggle during game
        if (key_pressed == 'm' || key_pressed == 'M') {
            sound_enabled = !sound_enabled;
            mvprintw(2, 0, "Sound %s                  ", sound_enabled ? "ON" : "OFF");
        }

        changeAllDirections(snakes, NUM_SNAKES, key_pressed); // change direction

        // Move all snakes
        for (size_t i = 0; i < NUM_SNAKES; i++) {
            if (snakes[i].is_alive) {
                go(&snakes[i]);
                goTail(&snakes[i]);
            }
        }

        // Draw game field
        clear();
        // Display score
        mvprintw(0, 0, "P1: %d points | P2: %d points | M - sound | F10 - exit",
                 snakes[0].score, snakes[1].score);
        mvprintw(1, 0, "Controls: Arrows / WASD | Eat the other snake!");

        drawAllSnakes(snakes, NUM_SNAKES);
        drawFood(&food);

        // Check eaten food
        if (checkAllFood(snakes, NUM_SNAKES, &food))
            spawnFood(&food, max_x, max_y);

        // Check collisions (self-intersection disabled via isCrush())
        if (checkAllCollisions(snakes, NUM_SNAKES, max_x, max_y)) {
            game_over = 1;
            playSound(3); // Game over sound
        }

        refresh();
        usleep(100000); // 100ms delay
    }

    // ===================== GAME OVER ==========================
    if (game_over) {
        clear();
        // Determine winner
        int winner = -1;
        int max_score = -1;

        for (size_t i = 0; i < NUM_SNAKES; i++) {
            if (snakes[i].is_alive) {
                winner = i;
            }
            if (snakes[i].score > max_score) {
                max_score = snakes[i].score;
                winner = i;
            }
        }

        if (winner != -1) {
            mvprintw(max_y/2, (max_x-20)/2, "Snake %d wins!", winner + 1);
            mvprintw(max_y/2 + 1, (max_x-20)/2, "Score: %d points", snakes[winner].score);
        } else {
            mvprintw(max_y/2, (max_x-9)/2, "Draw!");
        }

        mvprintw(max_y/2 + 2, (max_x-40)/2, "Press any key to return to menu...");
        refresh();
        getch(); // Wait for any key press
    }

    // ==================== GAME CLEANUP =======================
    for (size_t i = 0; i < NUM_SNAKES; i++) {
        free(snakes[i].tail);
    }
}

// ======================= MAIN FUNCTION ==========================
int main(){
    srand(time(NULL)); // random number generator

    // Initialize curses
    initscr();
    keypad(stdscr, TRUE);
    raw();
    noecho();
    curs_set(FALSE);

    // Check color support
    if(!has_colors()){
        endwin();
        printf("Your terminal does not support colors\n");
        return 1;
    }

    // Enable color and set color pairs
    start_color();
    // Use standard ncurses color constants
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    init_pair(8, COLOR_RED, COLOR_BLACK);    // Food

    // Show welcome screen on startup
    showWelcomeScreen();

    // Main program loop
    while (game_running) {
        int menu_result = startMenu();

        if (menu_result == 1) {
            // Start game
            playGame();
        } else if (menu_result == 2) {
            // Restart (do nothing, just continue loop)
            continue;
        } else {
            // Exit
            game_running = 0;
        }
    }

    endwin(); // end curses mode
    return 0;
}

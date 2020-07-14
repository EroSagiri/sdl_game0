#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define WINDOWWIDTH 600 // 窗口宽度
#define WINDOWHEIGHT 600 // 窗口高度
#define FPS 100 // 每秒渲染次数
#define MAPWINDOW 20 // 地图宽度
#define MAPHEIGHT 20 // 地图高度

#define MAP_TYPE_NULL 0 // 空的格子
#define MAP_TYPE_HEAD 1 // 蛇的身体格子
#define MAP_TYPE_FOOD 2 // 食物格子

#define HEAD_WIDTH 30 //  蛇身体的宽度
#define HEAD_COLOR 0xff, 0, 0, 0 // 身体颜色
#define FOOD_COLOR 0, 0, 0xff, 0 // 食物颜色

#define SNAKE_MOVE_LEFT 1 // 向左移动
#define SNAKE_MOVE_RIGHT 2 // 向右移动
#define SNAKE_MOVE_UP 3 // 向上移动
#define SNAKE_MOVE_DOWN 4 // 向下移动
#define SNAKE_MOVE_NULL 5 // 没有移动
#define SNAKE_SPEED 150 // 移动间隔(毫秒)
#define SNAKE_DEF_LEN 3 // 默认长度

SDL_Window *window; // 窗口
SDL_Renderer *renderer; // 渲染器
SDL_Event event; // 事件
SDL_Thread *renderer_thread; // 渲染线程
SDL_Thread *snakeer_thread; // ？？？
int map[MAPWINDOW][MAPHEIGHT] = {MAP_TYPE_NULL}; // 地图
int gameover = 0; // 游戏结束

int game_over();
static int game_init();
int game_reset();

// 贪吃蛇的身体结构体
typedef struct head {
    int x;
    int y;
    struct head *next;
} head;

head *snake;
int snake_t = SNAKE_MOVE_NULL; // 运动方向

// 创建一个食物
int create_food() {
    while(1) {
        srand(time(NULL));
        int food_x = rand() % MAPWINDOW;
        srand(time(NULL));
        int food_y = rand() % MAPHEIGHT;
        if(map[food_x][food_y] == MAP_TYPE_NULL) {
            map[food_x][food_y] = MAP_TYPE_FOOD;
            break;
        }
    }
    return 0;   
}
// 得到蛇长度
int get_snake_length() {
    int i = 0;
    head *t = snake;
    while(t != NULL) {
        i ++;
	// SDL_Log("x: %d  y: %d\n", t->x, t->y);
        t = t->next;
    }
    
    return i;
}
// 渲染线程 只负责渲染地图
static int renderThread() {
    while(event.type != SDL_QUIT) {
        SDL_RenderClear(renderer);
        for(int x = 0; x < MAPWINDOW; x++) {
            for(int y = 0; y < MAPHEIGHT; y++) {
                if(map[x][y] == MAP_TYPE_HEAD) {
                    SDL_SetRenderDrawColor(renderer, HEAD_COLOR);
                    SDL_Rect th = {x*HEAD_WIDTH, y*HEAD_WIDTH, HEAD_WIDTH, HEAD_WIDTH};
                    SDL_RenderFillRect(renderer, &th);
                }
                if(map[x][y] == MAP_TYPE_FOOD) {
                    SDL_SetRenderDrawColor(renderer, FOOD_COLOR);
                    SDL_Rect th = {x*HEAD_WIDTH, y*HEAD_WIDTH, HEAD_WIDTH, HEAD_WIDTH};
                    SDL_RenderFillRect(renderer, &th);
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderPresent(renderer);
        SDL_Delay((1000/FPS));
    }
    return 0;
}
// 蛇的移动处理线程
static int snake_thread() {
    while(event.type != SDL_QUIT) {
        if(snake_t != SNAKE_MOVE_NULL) {
            int px = snake->x;
            int py = snake->y;
            switch(snake_t) {
                case SNAKE_MOVE_UP :
                    py--;
                    break;
                case SNAKE_MOVE_DOWN :
                    py++;
                    break;
                case SNAKE_MOVE_LEFT :
                    px--;
                    break;
                case SNAKE_MOVE_RIGHT :
                    px++;
                    break;
            }
            
            if(px < 0 || px > MAPWINDOW-1 || py < 0 || py > MAPHEIGHT-1) { // 撞地图边界
                snake_t = SNAKE_MOVE_NULL;
                game_over();
            } else {
                if(map[px][py] == MAP_TYPE_FOOD) { // 吃到食物
                    create_food();
                    SDL_Log("吃到x:%d y:%d的食物 长度: %d\n", px, py, get_snake_length());
                    // 在最后插入一个节点
                    head *h = (head*) malloc(sizeof(head));
                    head *t = snake;
                    while(t->next != NULL) {
                        t = t->next;
                    }
                    h->x = t->x;
                    h->y = t->y;
                    h->next = NULL;
                    t->next = h;
                }
                if(map[px][py] == MAP_TYPE_HEAD) { // 吃到自己
                    snake_t = SNAKE_MOVE_NULL;
                    game_over();
                }
            
                // 上个节点旧的坐标，他的下个节点要移动的目标
                int oldX = snake->x;
                int oldY = snake->y;
                snake->x = px;
                snake->y = py;
                head *t = snake->next;
                while(t != NULL) {
                    if(oldX == t->x && oldY == t->y) {
                        t = t->next;
                    } else {
                        int tx = t->x;
                        int ty = t->y;
                        t->x = oldX;
                        t->y = oldY;
                        oldX = tx;
                        oldY = ty;
                        t = t->next;
                    }
                }
            }
        }
        
        // 更新蛇在地图
        for(int x = 0; x < MAPWINDOW; x++) {
            for(int y = 0; y < MAPHEIGHT; y++) {
                if(map[x][y] == MAP_TYPE_HEAD) {
                    map[x][y] = MAP_TYPE_NULL;
                }
            }
        }
        head *t = snake;
        while(t != NULL) {
            map[t->x][t->y] = MAP_TYPE_HEAD;
            t = t->next;
        }
        
        
        SDL_Delay(SNAKE_SPEED);
    }
    return 0;
}

// 蛇初始化
static head *snake_init() {
    head *s = (head *)malloc(sizeof(head));
    int defX = MAPWINDOW/2;
    int defY = MAPHEIGHT/2;
    s->x = defX;
    s->y = defY;
    s->next = NULL;
    head *tt = s;
    for(int i = 0; i < SNAKE_DEF_LEN - 1; i++) {
        head *t = (head*) malloc(sizeof(head));
        t->x = defX;
        t->y = defY;
        t->next = NULL;
        tt->next = t;
        tt = tt->next;
    }
    
    return s;
}
// 游戏初始化
static int game_init() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOWHEIGHT, WINDOWHEIGHT, SDL_WINDOW_RESIZABLE, &window,& renderer);
    SDL_SetWindowTitle(window, "贪吃蛇");
    gameover = 0;
    
    snake = snake_init();
    return 0;
}
// 游戏结束
int game_over() {
    gameover = 1;
    int l = get_snake_length();
    SDL_Log("游戏结束 得分: %d\n", l);
    char msg[20];
    char *s = msg;
    sprintf(s, "%d", l);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "游戏结束", s, window);
    game_reset();
    return 0;
}
// 游戏退出处理
static int game_quit() {
    SDL_DetachThread(renderer_thread);
    SDL_DetachThread(snakeer_thread);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

int game_reset()
{
    // 地图重置
    for(int x = 0; x < MAPWINDOW; x ++) {
        for(int y = 0; y < MAPHEIGHT; y ++) {
            map[x][y] = MAP_TYPE_NULL;
        }
    }
    // 蛇重置
    snake = snake_init();
    // 生成食物
    create_food();
    // 切换标志
    gameover = 0;
    return 0;
}


int main(int argc, char **argv) {
    game_init();
    // 贪吃蛇运动线程
    snakeer_thread = SDL_CreateThread(snake_thread, "snake", NULL);
    create_food();
    SDL_Log("蛇长度%d\n", get_snake_length());
    // 启动一个渲染线程
    renderer_thread = SDL_CreateThread(renderThread, "render", NULL);
    // 事件处理 堵塞
    while(event.type != SDL_QUIT) {
        SDL_PollEvent(&event);
        if(event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym == SDLK_LEFT && snake_t != SNAKE_MOVE_LEFT && snake_t != SNAKE_MOVE_RIGHT && !gameover) {
                snake_t = SNAKE_MOVE_LEFT;
            }
            if(event.key.keysym.sym == SDLK_RIGHT && snake_t != SNAKE_MOVE_RIGHT && snake_t != SNAKE_MOVE_LEFT && !gameover) {
                snake_t = SNAKE_MOVE_RIGHT;
            }
            if(event.key.keysym.sym == SDLK_DOWN && snake_t != SNAKE_MOVE_DOWN && snake_t != SNAKE_MOVE_UP && !gameover) {
                snake_t = SNAKE_MOVE_DOWN;
            }
            if(event.key.keysym.sym == SDLK_UP && snake_t != SNAKE_MOVE_UP && snake_t != SNAKE_MOVE_DOWN && !gameover) {
                snake_t = SNAKE_MOVE_UP;
            }
        }
        
        SDL_Delay(2);
    }
    
    game_quit();
    return 0;
}

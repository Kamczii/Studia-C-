#define _USE_MATH_DEFINES

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <time.h>

#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480

#define UNICORN_HEIGHT  90
#define UNICORN_WIDTH   90

#define BACKGROUND_WIDTH 3000
#define BACKGROUND_HEIGHT 3000

#define SPEED_UP_WAIT 5000
#define MAX_SPEED 3
#define JUMP_HEIGHT 600
#define OBSTACLES_AND_PLATFORMS 14
#define LIVES 3


enum ObjectType {NONE, PLAYER, GROUND, OBSTACLE, HIT};

struct GameObject {
        SDL_Surface* surface;
        ObjectType type;
        SDL_Point cords;
};

// narysowanie napisu txt na powierzchni screen, zaczynając od punktu (x, y)
// charset to bitmapa 128x128 zawierająca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
        int px, py, c;
        SDL_Rect s, d;
        s.w = 8;
        s.h = 8;
        d.w = 8;
        d.h = 8;
        while(*text) {
                c = *text & 255;
                px = (c % 16) * 8;
                py = (c / 16) * 8;
                s.x = px;
                s.y = py;
                d.x = x;
                d.y = y;
                SDL_BlitSurface(charset, &s, screen, &d);
                x += 8;
                text++;
                };
        };


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt środka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
        SDL_Rect dest;
        dest.x = x - sprite->w / 2;
        dest.y = y - sprite->h / 2;
        dest.w = sprite->w;
        dest.h = sprite->h;
        SDL_BlitSurface(sprite, NULL, screen, &dest);
};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
        int bpp = surface->format->BytesPerPixel;
        Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
        *(Uint32 *)p = color;
        };


// rysowanie linii o długości l w pionie (gdy dx = 0, dy = 1) 
// bądź poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
        for(int i = 0; i < l; i++) {
                DrawPixel(screen, x, y, color);
                x += dx;
                y += dy;
                };
        };


// rysowanie prostokąta o długości boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
        int i;
        DrawLine(screen, x, y, k, 0, 1, outlineColor);
        DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
        DrawLine(screen, x, y, l, 1, 0, outlineColor);
        DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
        for(i = y + 1; i < y + k - 1; i++)
                DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
        };


//Sprawdza czy doszło do kolizji, jeśli tak zwraca typ obiektu
ObjectType checkCollision(GameObject player, GameObject obj2) {

        double distance_x = obj2.cords.x - player.cords.x;
        double distance_y = obj2.cords.y - player.cords.y;

        distance_x = fabs(distance_x);
        distance_y = fabs(distance_y);

        bool collision_x = distance_x <= (player.surface->w + obj2.surface->w) / 2;
        bool collision_y = distance_y <= (player.surface->h + obj2.surface->h) / 2;

        if (collision_x&&collision_y) {
                return obj2.type;
        }
        else
                return NONE;
}

//zwraca false jeśli niedozwolona kolizja
bool isCollisionAllowed(GameObject player, GameObject obj2) {

        ObjectType type = obj2.type;

        //Kolizja z przeszkodą
        if (type == OBSTACLE)
                return false;

        if (type == GROUND) {
                int leftO = obj2.cords.x - obj2.surface->w / 2;
                int bottomO = obj2.cords.y + obj2.surface->h / 2;
                int rightO = obj2.cords.x + obj2.surface->w / 2;
                int topO = obj2.cords.y - obj2.surface->h / 2;

                int leftP = player.cords.x - player.surface->w / 2;
                int bottomP = player.cords.y + player.surface->h / 2;
                int rightP = player.cords.x + player.surface->w / 2;
                int topP = player.cords.y - player.surface->h / 2;

                //sprawdza kolizję od dołu i lewej strony
                if (leftO <= rightP && bottomO >= topP)
                        return false;
                if (rightO >= leftP && bottomO >= topP)
                        return false;
        }

        return true;
}


//Rysuje wszystkie elementy świata gry
void DrawElements(SDL_Surface* screen, SDL_Surface* background, int background_x2, SDL_Rect& camera, int background_x1, GameObject& ground, GameObject& unicorn, GameObject*  obstacles, SDL_Surface* explosion,  int collided, SDL_Surface* heart, int lives)
{

        DrawSurface(screen, background, background_x2 - camera.x, BACKGROUND_HEIGHT / 2 - camera.y);
        DrawSurface(screen, background, background_x1 - camera.x, BACKGROUND_HEIGHT / 2 - camera.y);
        DrawSurface(screen, ground.surface, ground.cords.x - camera.x, ground.cords.y - camera.y);
        DrawSurface(screen, unicorn.surface, unicorn.cords.x - camera.x, unicorn.cords.y - camera.y);
        for (int i = 0;i < OBSTACLES_AND_PLATFORMS;i++) {
                DrawSurface(screen, obstacles[i].surface, obstacles[i].cords.x - camera.x , obstacles[i].cords.y - camera.y);
        }
        if (collided) {
                DrawSurface(screen, explosion, unicorn.cords.x - camera.x + UNICORN_WIDTH/2, unicorn.cords.y - camera.y - UNICORN_HEIGHT / 3);
        }
        for (int i = 0;i < lives;i++) {
                DrawSurface(screen, heart,heart->w*(i+1)+(i*10), 38 + heart->h);
        }
}

//Wyświetla statystyki
void DrawHUD(SDL_Surface* screen, int czerwony, int niebieski, char  text[128], double worldTime, double fps, double distance, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer)
{
        // tekst informacyjny / info text
        DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
        //            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
        sprintf(text, "Czas = %.1lf s  %.0lf klatek / s dystans %.0f", worldTime, fps, distance);
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
        //            "Esc - exit, \030 - faster, \031 - slower"
        sprintf(text, "Esc - wyjscie, n - nowa gra");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        //              SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
}
;

// main
#ifdef __cplusplus
extern "C"
#endif


//Zwraca typ jeśli jednorożec zderzył się z czymś z czym nie powinien
ObjectType collidedWithObstacle(const GameObject& unicorn, GameObject*  obstacles)
{
        for (int i = 0;i < OBSTACLES_AND_PLATFORMS;i++) {
                if (checkCollision(unicorn, obstacles[i]) != NONE) {
                        if (!isCollisionAllowed(unicorn, obstacles[i]))
                                return obstacles[i].type;
                }
        }
        return NONE;
}


//Zapętla tło i generuje platformy każda z nich zawiera jedną przeszkodę
void loopBackgroundAndObstacles(double distance, int& background_x2, bool& x1_first, int& background_x1, GameObject** obstacles, GameObject* ground)
{
        if (distance> background_x2 && x1_first) {
                background_x1 += BACKGROUND_WIDTH * 2;
                x1_first = false;
        }
        else if (distance > background_x1 && !x1_first) {
                background_x2 += BACKGROUND_WIDTH * 2;
                x1_first = true;
        }

        for (int i = 0;i < OBSTACLES_AND_PLATFORMS / 2;i++) {
                if (distance > ((*obstacles)[i * 2].cords.x + SCREEN_WIDTH)) {
                        (*obstacles)[i * 2].cords.x = distance + BACKGROUND_WIDTH + (i % OBSTACLES_AND_PLATFORMS) * SCREEN_WIDTH / 4 * 3;
                        (*obstacles)[i * 2].cords.y = ground->cords.y - 3 * ((rand() % (i + 1)) + 1) * (ground->surface->h + (*obstacles)[i * 2].surface->h);
                        (*obstacles)[1 + (i * 2)].cords.x = distance + BACKGROUND_WIDTH + (i % OBSTACLES_AND_PLATFORMS) * SCREEN_WIDTH / 4 * 3 + (*obstacles)[i * 2].surface->w / 3;
                        (*obstacles)[1 + (i * 2)].cords.y = (*obstacles)[i * 2].cords.y - ((*obstacles)[i * 2].surface->h + (*obstacles)[1 + i * 2].surface->h) / 2;
                }
        }
}


//ląduje na ziemi
void landOnGround(int& isFalling, int& isJumping, int& isDashing, int& firstJump, int& secondJump)
{
        isFalling = 0;
        isJumping = 0;
        firstJump = 0;
        secondJump = 0;
        isDashing = 0;
}


//kolizja
void collision(double& collisionTimer, int& collided, int& lives) {
        if (!collided) {
                lives--;
        }
        collisionTimer = SDL_GetTicks() + 400.0;
        collided = 1;
}


//obsługuje grawitację skoku
void jumpAndFall(int& isJumping, int& firstJump, double& unicornJump, int& isFalling, int& secondJump, int isDashing, double delta, GameObject& unicorn, GameObject* obstacles, double& distance, double unicornSpeed, GameObject& ground , double& collisionTimer, int& collided, int& lives)
{
        //wzbija się
        if (isJumping && !firstJump)
        {
                firstJump = 1;
                unicornJump = 0;
                isFalling = 0;
                
        }
        else if (isJumping && isFalling && firstJump && !secondJump) { //wzbija się drugi raz
                secondJump = 1;
                unicornJump = 0;
                isFalling = 0;
        }
        else if (secondJump && isFalling) { //spada po drugim skoku
                isJumping = 0;
        }


        if (isJumping && unicornJump < JUMP_HEIGHT && !isDashing) { //skacze
                double jumped = delta * SCREEN_HEIGHT * 4;
                unicornJump += jumped;
                unicorn.cords.y -= jumped;
                for (int i = 0;i < OBSTACLES_AND_PLATFORMS;i++) {
                        if (checkCollision(unicorn, obstacles[i]) == GROUND) {
                                isFalling = 1;
                                isJumping = 0;
                                isDashing = 0;
                                collision(collisionTimer, collided, lives);
                                unicorn.cords.y = obstacles[i].cords.y + obstacles[i].surface->h/2 + unicorn.surface->h / 2+  1;
                        }
                }
        }
        else if (isJumping && !isDashing) { //osiągnał maks. wysokość
                isJumping = 0;
                isFalling = 1;
        }
        else if (isDashing && (isFalling || isJumping)) { //zryw, możliwy podczas opadania lub skoku
                distance += unicornSpeed * delta * SCREEN_HEIGHT / 3 * 2;
                secondJump = 0;
                for (int i = 0;i < OBSTACLES_AND_PLATFORMS;i++) {
                        if (checkCollision(unicorn, obstacles[i]) == GROUND) {
                                isFalling = 1;
                                isJumping = 0;
                                isDashing = 0;
                                collision(collisionTimer, collided, lives);
                                unicorn.cords.y = obstacles[i].cords.y + obstacles[i].surface->h / 2 + unicorn.surface->h / 2 + 1;
                        }
                }
        }
        else if (isFalling) { //spada
                double fallen = delta * SCREEN_HEIGHT * 4;
                unicornJump -= fallen;
                unicorn.cords.y += fallen;
                if (checkCollision(unicorn, ground) == GROUND && isFalling) {
                        unicorn.cords.y = ground.cords.y - (ground.surface->h + unicorn.surface->h) / 2 + 1;
                        landOnGround(isFalling, isJumping, isDashing, firstJump, secondJump);
                }
                for (int i = 0;i < OBSTACLES_AND_PLATFORMS;i++) {
                        if (checkCollision(unicorn, obstacles[i]) == GROUND && isFalling) {
                                unicorn.cords.y = obstacles[i].cords.y - (obstacles[i].surface->h + unicorn.surface->h) / 2 + 1;
                                landOnGround(isFalling, isJumping, isDashing, firstJump, secondJump);
                        }
                }
        }
        else { //zwykły ruch
                unicornJump = 0;
                if (!isJumping && !isFalling) {
                        bool falling = 1;
                        if (checkCollision(unicorn, ground) == GROUND) {
                                falling = 0;
                        }
                        for (int i = 0;i < OBSTACLES_AND_PLATFORMS/2;i++) {
                                if (checkCollision(unicorn, obstacles[i*2]) == GROUND) {
                                        falling = 0;
                                }
                        }
                        if (falling)
                                isFalling = 1;
                }
        }
}

//zwalnia pamięć
void freeMemory(SDL_Surface* charset, SDL_Surface* screen, SDL_Surface* background, SDL_Surface* platformSurface, SDL_Surface* obstacleSurface, SDL_Surface* explosionSurface, SDL_Surface* heartSurface, GameObject& ground, GameObject& unicorn, GameObject* obstacles, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer)
{
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_FreeSurface(background);
        SDL_FreeSurface(platformSurface);
        SDL_FreeSurface(obstacleSurface);
        SDL_FreeSurface(explosionSurface);
        SDL_FreeSurface(heartSurface);
        SDL_FreeSurface(ground.surface);
        SDL_FreeSurface(unicorn.surface);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();

        free(obstacles);
}

int main(int argc, char **argv) {
        int t1, t2, isJumping, isDoubleJumping, firstJump, secondJump, isFalling, isDashing, newGame, quit, frames, rc, background_x1, background_x2, collided, lives;
        double delta, worldTime, fpsTimer, fps, distance, resolutionDistance, unicornSpeed, unicornJump, jumpSpeed, additionalSpeed, speedIncreaseTimer, collisionTimer;
        SDL_Event event;
        SDL_Surface *screen = NULL, *charset = NULL;
        SDL_Surface *background = NULL, *platformSurface = NULL, *obstacleSurface = NULL, *explosionSurface = NULL, *heartSurface = NULL;
        GameObject unicorn, ground;
        SDL_Texture *scrtex;
        SDL_Window *window;
        SDL_Renderer *renderer;
        GameObject* obstacles = (GameObject*)malloc(sizeof(GameObject)* OBSTACLES_AND_PLATFORMS);
        bool defaultControl = true;
        SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_Keycode jump, dash;

        unicorn.surface = NULL;
        ground.surface = NULL;

        printf("wyjscie printfa trafia do tego okienka\n");
        printf("printf output goes here\n");

        if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
                printf("SDL_Init error: %s\n", SDL_GetError());
                return 1;
                }

        // tryb pełnoekranowy / fullscreen mode
        //rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
        //                                       &window, &renderer);
        rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                                         &window, &renderer);
        if(rc != 0) {
                SDL_Quit();
                printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
                return 1;
                };
        
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_SetWindowTitle(window, "Robot Unicorn Attack");


        screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
                                      0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

        scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   SCREEN_WIDTH, SCREEN_HEIGHT);


        // wyłączenie widoczności kursora myszy
        SDL_ShowCursor(SDL_DISABLE);


        charset = SDL_LoadBMP("./cs8x8.bmp");
        if(charset == NULL) {
                printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
                freeMemory(charset, screen, background, platformSurface, obstacleSurface, explosionSurface, heartSurface, ground, unicorn, obstacles, scrtex, window, renderer);
                return 1;
                };
        SDL_SetColorKey(charset, true, 0x000000);


        background = SDL_LoadBMP("./bg.bmp");
        if (background == NULL) {
                printf("SDL_LoadBMP(bg.bmp) error: %s\n", SDL_GetError());
                freeMemory(charset, screen, background, platformSurface, obstacleSurface, explosionSurface, heartSurface, ground, unicorn, obstacles, scrtex, window, renderer);
                return 1;
        };


        explosionSurface = SDL_LoadBMP("./explosion.bmp");
        if (explosionSurface == NULL) {
                printf("SDL_LoadBMP(explosion.bmp) error: %s\n", SDL_GetError());
                freeMemory(charset, screen, background, platformSurface, obstacleSurface, explosionSurface, heartSurface, ground, unicorn, obstacles, scrtex, window, renderer);
                return 1;
        };


        unicorn.type = PLAYER;
        unicorn.surface = SDL_LoadBMP("./unicorn.bmp");
        if(unicorn.surface == NULL) {
                printf("SDL_LoadBMP(unicorn.bmp) error: %s\n", SDL_GetError());
                freeMemory(charset, screen, background, platformSurface, obstacleSurface, explosionSurface, heartSurface, ground, unicorn, obstacles, scrtex, window, renderer);
                return 1;
        };


        ground.type = GROUND;
        ground.surface = SDL_LoadBMP("./ground.bmp");
        if (ground.surface == NULL) {
                printf("SDL_LoadBMP(unicorn.bmp) error: %s\n", SDL_GetError());
                freeMemory(charset, screen, background, platformSurface, obstacleSurface, explosionSurface, heartSurface, ground, unicorn, obstacles, scrtex, window, renderer);
                return 1;
        };

        platformSurface = SDL_LoadBMP("./platform.bmp");

        if (platformSurface == NULL) {
                printf("SDL_LoadBMP(platform.bmp) error: %s\n", SDL_GetError());
                freeMemory(charset, screen, background, platformSurface, obstacleSurface, explosionSurface, heartSurface, ground, unicorn, obstacles, scrtex, window, renderer);
                return 1;
        };


        obstacleSurface = SDL_LoadBMP("./obstacle.bmp");
        if (obstacleSurface == NULL) {
                printf("SDL_LoadBMP(obstacle.bmp) error: %s\n", SDL_GetError());
                freeMemory(charset, screen, background, platformSurface, obstacleSurface, explosionSurface, heartSurface, ground, unicorn, obstacles, scrtex, window, renderer);
                return 1;
        };


        heartSurface = SDL_LoadBMP("./heart.bmp");
        if (heartSurface == NULL) {
                printf("SDL_LoadBMP(heart.bmp) error: %s\n", SDL_GetError());
                freeMemory(charset, screen, background, platformSurface, obstacleSurface, explosionSurface, heartSurface, ground, unicorn, obstacles, scrtex, window, renderer);
                return 1;
        };


        for (int i = 0;i < OBSTACLES_AND_PLATFORMS/2;i++) {

                obstacles[i * 2].type = GROUND;
                obstacles[i * 2].surface = platformSurface;

                obstacles[1 + i * 2].type = OBSTACLE;
                obstacles[1 + i * 2].surface = obstacleSurface;
        }

        char text[128];
        int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
        int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
        int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
        int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);


        t1 = SDL_GetTicks();

        quit = 0;

        while (!quit) {

                lives = LIVES;

                frames = 0;
                fpsTimer = 0;
                fps = 0;
                newGame = 0;
                worldTime = 0;
                distance = 0;

                unicornJump = 0; //ile już przeskoczył jednorożec w danym skoku
                isJumping = 0; //czy skacze
                isFalling = 0; //czy spada
                isDashing = 0; //czy jest w trakcie zrywu
                firstJump = 0; //czy wykonał już pierwszy skok
                secondJump = 0; //czy wykonał już drugi skok
                background_x1 = 0; //pozycja tła pierwszego
                background_x2 = BACKGROUND_WIDTH; //pozycja tła drugiego

                unicorn.cords.x = UNICORN_WIDTH / 3;
                unicorn.cords.y = BACKGROUND_HEIGHT - SCREEN_HEIGHT / 2;


                ground.cords.x = SCREEN_WIDTH / 2;
                ground.cords.y = BACKGROUND_HEIGHT;


                srand(time(NULL));

                for (int i = 0;i < OBSTACLES_AND_PLATFORMS/2;i++) {

                        obstacles[i * 2].cords.x = SCREEN_WIDTH * 2 + (i % OBSTACLES_AND_PLATFORMS) * SCREEN_WIDTH;
                        obstacles[i*2].cords.y = ground.cords.y - 3 * ((rand() % (i + 1))+1) * (ground.surface->h + obstacles[i * 2].surface->h);

                        obstacles[1 + i * 2].cords.x = obstacles[i * 2].cords.x + (obstacles)[i * 2].surface->w / 3;
                        obstacles[1+ i * 2].cords.y = obstacles[i * 2].cords.y - (obstacles[i * 2].surface->h + obstacles[1 + i * 2].surface->h) / 2;
                }

                bool x1_first = true; //czy tło nr. jest jest aktualnie wyświetlanym tłem

                additionalSpeed = 2; //dodatkowa prędkość
                unicornSpeed = 2;

                speedIncreaseTimer = 0; //licznik, który odpowiada za przyśpieszenie jednorożca po upływie czassu

                collisionTimer = 0; //licznik, który służy do wyświetlania wybuchu po kolizji
                collided = 0; //kolizja

                while (!newGame && !quit && lives) {
                        t2 = SDL_GetTicks();

                        // w tym momencie t2-t1 to czas w milisekundach,
                        // jaki uplynał od ostatniego narysowania ekranu
                        // delta to ten sam czas w sekundach
                        // here t2-t1 is the time in milliseconds since
                        // the last screen was drawn
                        // delta is the same time in seconds
                        delta = (t2 - t1) * 0.001;
                        t1 = t2;

                        worldTime += delta;

                        distance += unicornSpeed * delta * SCREEN_HEIGHT / 3;

                        if (speedIncreaseTimer < t2 - 1.0 && additionalSpeed < MAX_SPEED) {
                                speedIncreaseTimer = SDL_GetTicks() + SPEED_UP_WAIT;
                                additionalSpeed*=1.3;
                                unicornSpeed = additionalSpeed;
                        }

                        if (collisionTimer < t2 - 1.0) {
                                collisionTimer = 0;
                                collided = 0;
                        }

                        SDL_FillRect(screen, NULL, zielony);

                        loopBackgroundAndObstacles(distance, background_x2, x1_first, background_x1, &obstacles, &ground);
                        
                        jumpAndFall(isJumping, firstJump, unicornJump, isFalling, secondJump, isDashing, delta, unicorn, obstacles, distance, unicornSpeed, ground, collisionTimer, collided, lives);

                        
                        //sprawdza czy zaliczyliśmy kolizję z przeszkodami
                        for (int i = 0;i < OBSTACLES_AND_PLATFORMS;i++) {
                                if (checkCollision(unicorn, obstacles[i]) == OBSTACLE)
                                        collision(collisionTimer, collided, lives);
                        }

                        //pozycja jednorożca to przebyty dystans
                        unicorn.cords.x = distance;

                        ground.cords.x = unicorn.cords.x + SCREEN_WIDTH / 4;
                        camera.x = unicorn.cords.x - SCREEN_WIDTH / 6;

                        if (unicorn.cords.y < BACKGROUND_HEIGHT / 4 - 20) { //jednorożec osiąga górę mapy
                                unicorn.cords.y = BACKGROUND_HEIGHT / 4 - 20;
                        }
                        camera.y = unicorn.cords.y - SCREEN_HEIGHT + ground.surface->h + unicorn.surface->h / 2;


                        DrawElements(screen, background, background_x2, camera, background_x1, ground, unicorn,  obstacles, explosionSurface, collided, heartSurface, lives);


                        fpsTimer += delta;
                        if (fpsTimer > 0.5) {
                                fps = frames * 2;
                                frames = 0;
                                fpsTimer -= 0.5;
                        };


                        DrawHUD(screen, czerwony, niebieski, text, worldTime, fps, distance, charset, scrtex, renderer);

                        // obsługa zdarzeń (o ile jakieś zaszły) / handling of events (if there were any)
                        while (SDL_PollEvent(&event)) {
                                        
                                        if(defaultControl)
                                        {
                                                dash = SDLK_RIGHT;
                                                jump = SDLK_UP;
                                        }
                                        else {
                                                dash = SDLK_x;
                                                jump = SDLK_z;
                                        }

                                        switch (event.type) {
                                        case SDL_KEYDOWN:
                                                if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
                                                else if (event.key.keysym.sym == SDLK_LEFT) unicornSpeed = additionalSpeed * 0.5;
                                                else if (event.key.keysym.sym == jump) isJumping = 1;
                                                else if (event.key.keysym.sym == dash) isDashing = 1;
                                                else if (event.key.keysym.sym == SDLK_d) defaultControl = !defaultControl;
                                                else if (event.key.keysym.sym == SDLK_n) newGame = 1;
                                                break;
                                        case SDL_KEYUP:
                                                unicornSpeed = additionalSpeed;
                                                isJumping = 0;
                                                isDashing = 0;
                                                break;
                                        case SDL_QUIT:
                                                quit = 1;
                                                break;
                                        };
                        };
                        frames++;
                };

        }
        

        freeMemory(charset, screen, background, platformSurface, obstacleSurface, explosionSurface, heartSurface, ground, unicorn, obstacles, scrtex, window, renderer);
        

        _CrtDumpMemoryLeaks();
        return 0;
        }

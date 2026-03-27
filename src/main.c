#include <math.h>
#include <stdbool.h>
#include "raylib.h"
#define ROWS 6
#define COLUMNS 8
#define CELL_SIZE 10


static struct{
    int ray_count;
    float fov, angle;
    Vector2 position;
} player;
static const bool map[ROWS][COLUMNS] = {
    {true, true, true, true, true, true, true, true},
    {true, false, false, false, true, false, false, true},
    {true, false, true, false, true, false, true, true},
    {true, false, true, false, false, false, false, true},
    {true, false, false, false, false, false, false, true},
    {true, true, true, true, true, true, true, true}
};
static int screen_width, screen_height;


static inline float radians(float degrees){
    return degrees * PI / 180.0f;
}


static inline bool check_collision_point(float x, float y){
    int i = y / CELL_SIZE, j = x / CELL_SIZE;

    return i>=0 && i<ROWS && j>=0 && j<COLUMNS && map[i][j];
}


static float ray_cast(float x, float y, float angle){
    float r = radians(angle);
    float sin_value = sinf(r);
    float cos_value = cosf(r);
    float tan_value = tanf(r);
    float epsilon = 1e-3;

    float d1 = INFINITY;
    if(fabsf(sin_value) > epsilon){
        float yn = -(y - floor(y/CELL_SIZE)*CELL_SIZE);
        float ys = -CELL_SIZE;
        if(sin_value > 0.0f){
            yn += CELL_SIZE;
            ys *= -1.0f;
        }
        float xn = yn / tan_value;
        float xs = ys / tan_value;
        float yy = y + yn;
        float xx = x + xn;
        for(int i=0; i<=ROWS; ++i){
            if(check_collision_point(xx, yy - 1.0f)) break;
            if(check_collision_point(xx, yy)) break;

            yy += ys, xx += xs;
        }

        d1 = hypotf(xx - x, yy - y);
    }

    float d2 = INFINITY;
    if(fabsf(cos_value) > epsilon){
        float xn = -(x - floor(x/CELL_SIZE)*CELL_SIZE);
        float xs = -CELL_SIZE;
        if(cos_value > 0.0f){
            xn += CELL_SIZE;
            xs *= -1.0f;
        }
        float yn = xn * tan_value;
        float ys = xs * tan_value;
        float xx = x + xn;
        float yy = y + yn;
        for(int i=0; i<=COLUMNS; ++i){
            if(check_collision_point(xx - 1.0f, yy)) break;
            if(check_collision_point(xx, yy)) break;

            xx += xs, yy += ys;
        }

        d2 = hypotf(xx - x, yy - y);
    }

    return (d1 < d2 ? d1 : d2);
}


static void draw_world(){
    int half_height = roundf(screen_height / 2.0f);
    DrawRectangle(0, 0, screen_width, half_height, (Color) {27, 40, 69, 255});
    DrawRectangle(0, half_height, screen_width, half_height, (Color) {58, 43, 35, 255});

    float step = player.fov / (player.ray_count+1);
    float half_fov = player.fov / 2.0f;
    Rectangle rectangle = {.width = screen_width / player.ray_count};
    for(float i=step; i<player.fov; i+=step){
        float distance = ray_cast(player.position.x, player.position.y, player.angle + i - half_fov);
        distance *= cosf(radians(i - half_fov));

        float height = screen_height / distance;
        height = fminf(height, screen_height);

        rectangle.y = screen_height/2.0f - height/2.0f;
        rectangle.height = height;

        DrawRectangleRec(rectangle, (Color) {214, 178, 104, 255});

        rectangle.x += rectangle.width;
    }
}


static void draw_minimap(float scale){
    int size = CELL_SIZE * scale;
    DrawRectangle(0, 0, COLUMNS * size, ROWS * size, (Color) {18, 24, 35, 255});
    for(int i=0; i<ROWS; ++i){
        for(int j=0; j<COLUMNS; ++j){
            if(map[i][j]){
                DrawRectangle(j * size, i * size, size, size, (Color) {84, 97, 118, 255});
                DrawRectangleLines(j * size, i * size, size, size, (Color) {224, 145, 90, 255});
            }
        }
    }

    float px = player.position.x * scale;
    float py = player.position.y * scale;
    float radius = size / 8.0f;
    DrawCircle(px, py, radius, (Color) {92, 212, 201, 255});

    float step = player.fov / (player.ray_count+1);
    float half_fov = player.fov / 2.0f;
    for(float i=step; i<player.fov; i+=step){
        float angle_degree = player.angle + i - half_fov;
        float angle_radian = radians(angle_degree);
        float distance = ray_cast(player.position.x, player.position.y, angle_degree) * scale;
        DrawLine(
            px, py,
            px + cosf(angle_radian)*distance,
            py + sinf(angle_radian)*distance,
            (Color) {255, 239, 199, 120}
        );
    }
}


int main(){
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(1280, 720, "window");
    SetTargetFPS(60);

    screen_width = GetScreenWidth();
    screen_height = GetScreenHeight();

    for(int i=0; i<ROWS; ++i){
        for(int j=0; j<COLUMNS; ++j){
            if(!map[i][j]){
                player.position.x = (j+0.5f) * CELL_SIZE;
                player.position.y = (i+0.5f) * CELL_SIZE;
            }
        }
    }

    player.ray_count = screen_width;
    player.fov = 90.0f, player.angle = -90.0f;

    while(!WindowShouldClose()){
        float delta_time = GetFrameTime();
        float rotation_speed = 128.0f * delta_time, movement_speed = 8.0f * delta_time;

        if(IsKeyDown(KEY_A)) player.angle -= rotation_speed;
        if(IsKeyDown(KEY_D)) player.angle += rotation_speed;

        float nx = player.position.x, ny = player.position.y;
        if(IsKeyDown(KEY_W)){
            nx += cosf(radians(player.angle)) * movement_speed;
            ny += sinf(radians(player.angle)) * movement_speed;
        }
        if(IsKeyDown(KEY_S)){
            nx -= cosf(radians(player.angle)) * movement_speed;
            ny -= sinf(radians(player.angle)) * movement_speed;
        }

        if(!check_collision_point(nx, ny)) player.position = (Vector2) {nx, ny};

        BeginDrawing();
        ClearBackground((Color) {8, 12, 20, 255});

        draw_world();
        draw_minimap(5.0f);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
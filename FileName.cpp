#include <iostream>
#include <cmath>
#include "SDL.h"
using namespace std;

#define PI 3.141592

struct vector3 {
    float x;
    float y;
    float z;
    float a = 1;
}typedef vector3;

struct matrix_3x3 {
    float x_x, x_y, x_z;
    float y_x, y_y, y_z;
    float z_x, z_y, z_z;
}typedef matrix_3x3;

// 외적
vector3 cross_product(vector3 u, vector3 v) {
    return { u.y * v.z - v.y * u.z, u.z * v.x - v.z * u.x, u.x * v.y - v.x * u.y, 1 };
}

// 내적
float dot_product(vector3 p1, vector3 p2) {
    return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
}

// 벡터의 크기
float vector_size(vector3 p) {
    return sqrtf(dot_product(p, p));
}

// 벡터의 크기를 1로 변환한 벡터 반환
vector3 vector_size_to_1(vector3 p) {
    return { p.x / vector_size(p), p.y / vector_size(p), p.z / vector_size(p), 1 };
}

vector3 rotate(vector3 v, matrix_3x3 m) {
    return { m.x_x * v.x + m.x_y * v.y + m.x_z * v.z,
             m.y_x * v.x + m.y_y * v.y + m.y_z * v.z,
             m.z_x * v.x + m.z_y * v.y + m.z_z * v.z,
             1 };
}

// 역행렬
matrix_3x3 inverse_matrix(matrix_3x3 m) {
    return { m.x_x, m.y_x, m.z_x,
    m.x_y, m.y_y, m.z_y,
    m.x_z, m.y_z, m.z_z, };
}

vector3 camera_position = { 0, 0, 0 }; int camera_rotation_x = 0, camera_rotation_y = 0, camera_rotation_z = 0;
float FOV = 60.f;
float d = 1 / tan((FOV * PI / 180) / 2);

vector3 obj1_position = { 0, 0, -600 }; int obj1_rotation_x = 0, obj1_rotation_y = 0, obj1_rotation_z = 0;
vector3 obj2_position = { 300, 100, -600 };

void fill_triangle(SDL_Renderer* renderer, vector3 v1, vector3 v2, vector3 v3);

void draw_triangle(SDL_Renderer* renderer, vector3 points[], int points_size, int edges[3], matrix_3x3 obj_rotation, matrix_3x3 camera_rotation, vector3 position) {
    vector3 point_1 = { points[edges[0]].x, points[edges[0]].y, points[edges[0]].z };
    vector3 point_2 = { points[edges[1]].x, points[edges[1]].y, points[edges[1]].z };
    vector3 point_3 = { points[edges[2]].x, points[edges[2]].y, points[edges[2]].z };
    
    point_1 = rotate(point_1, obj_rotation);
    point_2 = rotate(point_2, obj_rotation);
    point_3 = rotate(point_3, obj_rotation);

    point_1.x += position.x - camera_position.x; point_1.y += position.y - camera_position.y;
    point_2.x += position.x - camera_position.x; point_2.y += position.y - camera_position.y;
    point_3.x += position.x - camera_position.x; point_3.y += position.y - camera_position.y;
    point_1.z += position.z - camera_position.z;
    point_2.z += position.z - camera_position.z;
    point_3.z += position.z - camera_position.z;

    point_1 = rotate(point_1, camera_rotation);
    point_2 = rotate(point_2, camera_rotation);
    point_3 = rotate(point_3, camera_rotation);

    vector3 v = { point_1.x, point_1.y, -(point_1.z) };
    vector3 n_1 = { (d * v.x * 640) / (v.z * 1.777), (d * v.y * 360) / (v.z), 1 };

    v = { point_2.x, point_2.y, -(point_2.z) };
    vector3 n_2 = { (d * v.x * 640) / (v.z * 1.777), (d * v.y * 360) / (v.z), 1 };

    v = { point_3.x, point_3.y, -(point_3.z) };
    vector3 n_3 = { (d * v.x * 640) / (v.z * 1.777), (d * v.y * 360) / (v.z), 1 };

    vector3 n_1_n_2 = { n_2.x - n_1.x, n_2.y - n_1.y, n_2.z - n_1.z };
    vector3 n_1_n_3 = { n_3.x - n_1.x, n_3.y - n_1.y, n_3.z - n_1.z };

    if ((point_1.z <= 0 && point_2.z <= 0 && point_3.z <= 0) && (dot_product(cross_product(n_1_n_2, n_1_n_3), { 0, 0, -1 }) < 0)) {
        n_1.x += 640, n_1.y += 360;
        n_2.x += 640, n_2.y += 360;
        n_3.x += 640, n_3.y += 360;

        fill_triangle(renderer, n_1, n_2, n_3 );
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        SDL_RenderDrawLine(renderer, n_1.x, n_1.y, n_2.x, n_2.y);
        SDL_RenderDrawLine(renderer, n_2.x, n_2.y, n_3.x, n_3.y);
        SDL_RenderDrawLine(renderer, n_1.x, n_1.y, n_3.x, n_3.y);
    }
}


void fill_triangle(SDL_Renderer* renderer, vector3 v1, vector3 v2, vector3 v3) {
    vector3 point[3] = { v1, v2, v3 };
    vector3 u = { point[0].x - point[1].x, point[0].y - point[1].y, 1 };
    vector3 v = { point[2].x - point[1].x, point[2].y - point[1].y, 1 };

    float min_x = min(point[0].x, point[1].x);
    min_x = min(min_x, point[2].x);

    float max_x = max(point[0].x, point[1].x);
    max_x = max(max_x, point[2].x);

    float min_y = min(point[0].y, point[1].y);
    min_y = min(min_y, point[2].y);

    float max_y = max(point[0].y, point[1].y);
    max_y = max(max_y, point[2].y);

    float denominator = powf(dot_product(u, v), 2) - (dot_product(u, u) * dot_product(v, v));

    if (denominator != 0) {
        for (float i = min_x; i <= max_x; i += 1) {
            if (i < 0) {
                i = 0;
            }
            if (1280 < i || max_x < i) {
                break;
            }
            for (float j = min_y; j <= max_y; j += 1) {
                if (j < 0) {
                    j = 0;
                }
                if (720 < j || max_y < j) {
                    break;
                }
                vector3 w = { i - point[1].x, j - point[1].y, 1 };

                float t = (dot_product(w, u) * dot_product(u, v) - dot_product(w, v) * dot_product(u, u)) / denominator;
                float s = (dot_product(w, v) * dot_product(u, v) - dot_product(w, u) * dot_product(v, v)) / denominator;
                float one_minus_ts = 1 - t - s;
                if (-0.f <= t && t <= 1.f && -0.f <= s && s <= 1.f && -0.f <= one_minus_ts && one_minus_ts <= 1.f && 0 <= i && i <= 1280 && 0 <= j && j <= 720) {
                    SDL_Rect capturerect = { (one_minus_ts * 255.f) + (s * 255.f) + (t * 255.f), (one_minus_ts * 255.f) + (s * 255.f) + (t * 255.f), 1, 1 };
                    SDL_SetRenderDrawColor(renderer, (one_minus_ts * 255.f), (s * 255.f), (t * 255.f), 255);
                    SDL_RenderDrawPoint(renderer, i, j);
                }
            }
        }
    }
}


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING); // SDL 초기화
    SDL_Window* window = SDL_CreateWindow("title_", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN); // 윈도우 생성
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); // 렌더러 생성
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // 렌더 모드 설정

    float n = 50;

    // 정점
    vector3 points[] = {
        {-n, -n, -n},
        { n, -n, -n},
        { n,  n, -n},
        {-n,  n, -n},
        {-n, -n,  n},
        { n, -n,  n},
        { n,  n,  n},
        {-n,  n,  n}
    };
    // 정점 사이즈
    int points_size = 8;

    // 삼각형 인덱스
    int edges[][3] = {
        {0, 2, 1}, // 앞면
        {0, 3, 2},
        {1, 6, 5}, // 오른쪽면
        {1, 2, 6},
        {5, 7, 4}, // 뒷면
        {5, 6, 7},
        {4, 3, 0}, // 왼쪽면
        {4, 7, 3},
        {3, 6, 2}, // 위쪽면
        {3, 7, 6},
        {4, 1, 5}, // 아랫면
        {4, 0, 1}
    };
    // 인덱스 사이즈
    int edges_size = 12;

    bool running = true;
    SDL_Event event;
    while (running) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // 검은색
        SDL_RenderClear(renderer); // 화면 클리어

        while (SDL_PollEvent(&event)) { // 이벤트 감지
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.scancode) {
                    // 카메라 x축 회전
                case SDL_SCANCODE_W:
                    camera_rotation_y -= 5;
                    break;
                case SDL_SCANCODE_S:
                    camera_rotation_y += 5;
                    break;
                    // 카메라 y축 회전
                case SDL_SCANCODE_A:
                    camera_rotation_z += 5;
                    break;
                case SDL_SCANCODE_D:
                    camera_rotation_z -= 5;
                    break;

                    // 오브젝트 z축 회전
                case SDL_SCANCODE_Z:
                    obj1_rotation_x += 10;
                    break;
                case SDL_SCANCODE_X:
                    obj1_rotation_x -= 10;
                    break;
                    // 오브젝트 x축 회전
                case SDL_SCANCODE_C:
                    obj1_rotation_y += 10;
                    break;
                case SDL_SCANCODE_V:
                    obj1_rotation_y -= 10;
                    break;
                    // 오브젝트 y축 회전
                case SDL_SCANCODE_B:
                    obj1_rotation_z += 10;
                    break;
                case SDL_SCANCODE_N:
                    obj1_rotation_z -= 10;
                    break;

                case SDL_SCANCODE_UP:
                    camera_position.z -= 10;
                    break;
                case SDL_SCANCODE_DOWN:
                    camera_position.z += 10;
                    break;
                case SDL_SCANCODE_LEFT:
                    camera_position.x -= 10;
                    break;
                case SDL_SCANCODE_RIGHT:
                    camera_position.x += 10;
                    break;
                case SDL_SCANCODE_O:
                    camera_position.y += 10;
                    break;
                case SDL_SCANCODE_P:
                    camera_position.y -= 10;
                    break;
                }
            }
        }
        obj1_rotation_x %= 360;
        obj1_rotation_y %= 360;
        obj1_rotation_z %= 360;

        float obj1_a = (float)obj1_rotation_x * PI / 180;
        float obj1_b = (float)obj1_rotation_y * PI / 180;
        float obj1_r = (float)obj1_rotation_z * PI / 180;

        float obj1_sa = sin(obj1_a), obj1_ca = cos(obj1_a);
        float obj1_sb = sin(obj1_b), obj1_cb = cos(obj1_b);
        float obj1_sr = sin(obj1_r), obj1_cr = cos(obj1_r);

        matrix_3x3 obj1_rotation_matrix = {
            obj1_cr * obj1_ca - obj1_sr * obj1_sb * obj1_sa, -obj1_cb * obj1_sa, obj1_ca * obj1_sr + obj1_cr * obj1_sb * obj1_sa,
            obj1_cr * obj1_sa + obj1_sr * obj1_sb * obj1_ca, obj1_cb * obj1_ca, obj1_sr * obj1_sa - obj1_cr * obj1_sb * obj1_ca,
            -obj1_cb * obj1_sr, obj1_sb, obj1_cr * obj1_cb
        };

        camera_rotation_x %= 360;
        camera_rotation_y %= 360;
        camera_rotation_z %= 360;

        float camera_a = (float)camera_rotation_x * PI / 180;
        float camera_b = (float)camera_rotation_y * PI / 180;
        float camera_r = (float)camera_rotation_z * PI / 180;

        float camera_sa = sin(camera_a), camera_ca = cos(camera_a);
        float camera_sb = sin(camera_b), camera_cb = cos(camera_b);
        float camera_sr = sin(camera_r), camera_cr = cos(camera_r);

        matrix_3x3 camera_rotation_matrix = {
           camera_cr * camera_ca - camera_sr * camera_sb * camera_sa, -camera_cb * camera_sa, camera_ca * camera_sr + camera_cr * camera_sb * camera_sa,
           camera_cr * camera_sa + camera_sr * camera_sb * camera_ca, camera_cb * camera_ca, camera_sr * camera_sa - camera_cr * camera_sb * camera_ca,
            -camera_cb * camera_sr, camera_sb, camera_cr * camera_cb
        };
        camera_rotation_matrix = inverse_matrix(camera_rotation_matrix);

        d = 1 / tan((FOV * PI / 180) / 2);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < edges_size; i++) {
            draw_triangle(renderer, points, points_size, edges[i], obj1_rotation_matrix, camera_rotation_matrix, obj1_position);
        }
        for (int i = 0; i < edges_size; i++) {
            //draw_triangle(renderer, points, points_size, edges[i], obj1_rotation_matrix, camera_rotation_matrix, obj2_position);
        }
        SDL_RenderPresent(renderer); // 렌더
    }
    SDL_Quit();
    return 0;
}
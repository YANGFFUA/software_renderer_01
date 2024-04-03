#include <iostream>
#include <cmath>
#include "SDL.h"
#include "SDL_image.h"
using namespace std;

#define PI 3.141592

struct vector3 {
    float x;
    float y;
    float z;
    float a = 1;
}typedef vector3;

struct UV {
    float x;
    float y;
}typedef UV;

struct vertex {
    vector3 coordinate;
    UV uv;
}typedef vertex;

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

float near_z = 5.5f;
float far_z = 5000.f;

void fill_triangle(SDL_Renderer* renderer, SDL_Texture* texture, vertex v1, vertex v2, vertex v3);

void draw_triangle(SDL_Renderer* renderer, SDL_Texture* texture, vertex points[], int points_size, int edges[3], matrix_3x3 obj_rotation, matrix_3x3 camera_rotation, vector3 position) {
    vertex point_1 = points[edges[0]];
    vertex point_2 = points[edges[1]];
    vertex point_3 = points[edges[2]];

    point_1.coordinate = rotate(point_1.coordinate, obj_rotation);
    point_2.coordinate = rotate(point_2.coordinate, obj_rotation);
    point_3.coordinate = rotate(point_3.coordinate, obj_rotation);

    point_1.coordinate.x += position.x - camera_position.x; point_1.coordinate.y += position.y - camera_position.y;
    point_2.coordinate.x += position.x - camera_position.x; point_2.coordinate.y += position.y - camera_position.y;
    point_3.coordinate.x += position.x - camera_position.x; point_3.coordinate.y += position.y - camera_position.y;
    point_1.coordinate.z += position.z - camera_position.z;
    point_2.coordinate.z += position.z - camera_position.z;
    point_3.coordinate.z += position.z - camera_position.z;

    point_1.coordinate = rotate(point_1.coordinate, camera_rotation);
    point_2.coordinate = rotate(point_2.coordinate, camera_rotation);
    point_3.coordinate = rotate(point_3.coordinate, camera_rotation);

    vertex v = { {point_1.coordinate.x, point_1.coordinate.y, -(point_1.coordinate.z) }, point_1.uv};
    vertex n_1 = { {(d * v.coordinate.x * 640) / (v.coordinate.z * 1.777), (d * v.coordinate.y * 360) / (v.coordinate.z), (-v.coordinate.z * (near_z + far_z) / (near_z - far_z) + 2 * (near_z * far_z) / (near_z - far_z)), (v.coordinate.z)}, {v.uv} };

    v = { {point_2.coordinate.x, point_2.coordinate.y, -(point_2.coordinate.z) }, point_2.uv };
    vertex n_2 = { {(d * v.coordinate.x * 640) / (v.coordinate.z * 1.777), (d * v.coordinate.y * 360) / (v.coordinate.z), (-v.coordinate.z * (near_z + far_z) / (near_z - far_z) + 2 * (near_z * far_z) / (near_z - far_z)), (v.coordinate.z)}, {v.uv} };

    v = { {point_3.coordinate.x, point_3.coordinate.y, -(point_3.coordinate.z) }, point_3.uv };
    vertex n_3 = { {(d * v.coordinate.x * 640) / (v.coordinate.z * 1.777), (d * v.coordinate.y * 360) / (v.coordinate.z), (-v.coordinate.z * (near_z + far_z) / (near_z - far_z) + 2 * (near_z * far_z) / (near_z - far_z)), (v.coordinate.z)}, {v.uv} };

    vector3 n_1_n_2 = { n_2.coordinate.x - n_1.coordinate.x, n_2.coordinate.y - n_1.coordinate.y, n_2.coordinate.z - n_1.coordinate.z };
    vector3 n_1_n_3 = { n_3.coordinate.x - n_1.coordinate.x, n_3.coordinate.y - n_1.coordinate.y, n_3.coordinate.z - n_1.coordinate.z };

    if ((point_1.coordinate.z <= 0 && point_2.coordinate.z <= 0 && point_3.coordinate.z <= 0) && (dot_product(cross_product(n_1_n_2, n_1_n_3), { 0, 0, -1 }) < 0)) {
        n_1.coordinate.x += 640, n_1.coordinate.y += 360;
        n_2.coordinate.x += 640, n_2.coordinate.y += 360;
        n_3.coordinate.x += 640, n_3.coordinate.y += 360;

        fill_triangle(renderer, texture, n_1, n_2, n_3);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }
}

float depth_buffer[1280][720];

void fill_triangle(SDL_Renderer* renderer, SDL_Texture* texture, vertex v1, vertex v2, vertex v3) {
    vertex point[3] = { v1, v2, v3 };
    vector3 u = { point[0].coordinate.x - point[1].coordinate.x, point[0].coordinate.y - point[1].coordinate.y, 1 };
    vector3 v = { point[2].coordinate.x - point[1].coordinate.x, point[2].coordinate.y - point[1].coordinate.y, 1 };

    float min_x = min(point[0].coordinate.x, point[1].coordinate.x);
    min_x = min(min_x, point[2].coordinate.x);

    float max_x = max(point[0].coordinate.x, point[1].coordinate.x);
    max_x = max(max_x, point[2].coordinate.x);

    float min_y = min(point[0].coordinate.y, point[1].coordinate.y);
    min_y = min(min_y, point[2].coordinate.y);

    float max_y = max(point[0].coordinate.y, point[1].coordinate.y);
    max_y = max(max_y, point[2].coordinate.y);

    float denominator = powf(dot_product(u, v), 2) - (dot_product(u, u) * dot_product(v, v));

    if (denominator != 0) {
        for (int i = min_x; i <= max_x; i += 1) {
            if (i < 0) {
                i = 0;
            }
            if (1280 < i || max_x < i) {
                break;
            }
            for (int j = min_y; j <= max_y; j += 1) {
                if (j < 0) {
                    j = 0;
                }
                if (720 < j || max_y < j) {
                    break;
                }
                vector3 w = { i - point[1].coordinate.x, j - point[1].coordinate.y, 1 };

                float t = (dot_product(w, u) * dot_product(u, v) - dot_product(w, v) * dot_product(u, u)) / denominator;
                float s = (dot_product(w, v) * dot_product(u, v) - dot_product(w, u) * dot_product(v, v)) / denominator;
                float one_minus_ts = 1 - t - s;
                if (-0.f <= t && t <= 1.f && -0.f <= s && s <= 1.f && -0.f <= one_minus_ts && one_minus_ts <= 1.f && 0 <= i && i <= 1280 && 0 <= j && j <= 720) {
                    float new_depth = one_minus_ts * point[1].coordinate.z + s * point[0].coordinate.z + t * point[2].coordinate.z;
                    float pre_depth = depth_buffer[i][j];

                    if (new_depth < pre_depth || pre_depth == 0) {
                        depth_buffer[i][j] = new_depth;
                    }
                    else {
                        continue;
                    }

                    float invz0 = 1 / point[0].coordinate.a;
                    float invz1 = 1 / point[1].coordinate.a;
                    float invz2 = 1 / point[2].coordinate.a;
                    float invz_ = 1 / ((one_minus_ts * invz1) + (s * invz0) + (t * invz2));
                    SDL_Rect capturerect = { (one_minus_ts * point[1].uv.x * invz_ * invz1) + (s * point[0].uv.x * invz_ * invz0) + (t * point[2].uv.x * invz_ * invz2), (one_minus_ts * point[1].uv.y * invz_ * invz1) + (s * point[0].uv.y * invz_ * invz0) + (t * point[2].uv.y * invz_ * invz2), 1, 1 };
                    SDL_Rect screenrect = { i, j, 1, 1 };
                    SDL_RenderCopy(renderer, texture, &capturerect, &screenrect);
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

    SDL_Texture* texture = IMG_LoadTexture(renderer, "CKMan.png");

    float n = 50;

    // 정점
    vertex points[] = {
        // 뒷머리
        {{-n, -n, -n }, { 0.125 * 3 * 64, (1 - 0.875) * 64 }},
        {{ n, -n, -n }, { 0.125 * 3 * 64, (1 - 0.875) * 64 }},
        {{ n,  n, -n }, { 0.25 * 2 * 64, (1 - 0.75) * 64 }},
        {{-n,  n, -n }, { 0.25 * 2 * 64, (1 - 0.75) * 64 }},

        // 앞면
        {{-n, -n,  n }, { 0.125 * 64, (1 - 0.875) * 64 }},
        {{ n, -n,  n }, { 0.25 * 64, (1 - 0.875) * 64 }},
        {{ n,  n,  n }, { 0.25 * 64, (1 - 0.75) * 64 }},
        {{-n,  n,  n }, { 0.125 * 64, (1 - 0.75) * 64 }},

        // 윗머리
        {{-n, -n, -n }, { 0.125 * 64, 0 }},
        {{ n, -n, -n }, { 0.125 * 64, 0 }},
        {{ n, -n, n }, { 0.25 * 64, (1 - 0.875) * 64 }},
        {{-n, -n, n }, { 0.25 * 64, (1 - 0.875) * 64 }},

        // 좌측
        {{ -n, -n, -n }, { 0, (1 - 0.875) * 64 }}, // 1
        {{ -n, -n, n }, { 0.125 * 64, (1 - 0.875) * 64 }}, // 2
        {{ -n, n, n }, { 0.125 * 64, (1 - 0.75) * 64 }}, // 3
        {{ -n, n, -n }, { 0, (1 - 0.75) * 64 }}, // 4

        // 우측
        {{ n, -n, n }, { 0.25 * 64, (1 - 0.875) * 64 }}, // 1
        { { n, -n, -n }, { 0.125 * 3 * 64, (1 - 0.875) * 64 } }, // 2
        {{ n, n, -n }, { 0.125 * 3 * 64, (1 - 0.75) * 64 }}, // 3
        {{ n, n, n }, { 0.25 * 64, (1 - 0.75) * 64 }}, // 4

        {{ n, n, n }, { 0.125 * 3 * 64, 0 }},
        {{ -n, n, n }, { 0.125 * 2 * 64, 0 }},
        {{ -n, n, -n }, { 0.125 * 2 * 64, (1 - 0.875) * 64 }},
        {{ n, n, -n }, { 0.125 * 3 * 64, (1 - 0.875) * 64 }}
    };
    // 정점 사이즈
    int points_size = 24;

    // 삼각형 인덱스
    int edges[][3] = {
        {0, 3, 1},
        {1, 3, 2},

        {5, 7, 4},
        {5, 6, 7},

        {8, 9, 11},
        {9, 10, 11},

        {12, 13, 15},
        {13, 14, 15},

        {16, 17, 19},
        {17, 18, 19},

        {20, 23, 21},
        {21, 23, 22}
    };
    // 인덱스 사이즈
    int edges_size = 12;

    bool running = true;
    SDL_Event event;
    while (running) {
        for (int i = 0; i < 1280; i++) {
            for (int j = 0; j < 720; j++) {
                depth_buffer[i][j] = 0;
            }
        }

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
            draw_triangle(renderer, texture, points, points_size, edges[i], obj1_rotation_matrix, camera_rotation_matrix, obj1_position);
        }
        for (int i = 0; i < edges_size; i++) {
            draw_triangle(renderer, texture, points, points_size, edges[i], obj1_rotation_matrix, camera_rotation_matrix, obj2_position);
        }
        SDL_RenderPresent(renderer); // 렌더
    }
    SDL_Quit();
    return 0;
}
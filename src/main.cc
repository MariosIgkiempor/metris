#include <SDL2/SDL.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <pstl/glue_algorithm_defs.h>

#include "SDL_keyboard.h"
#include "SDL_keycode.h"
#include "SDL_render.h"
#include "SDL_scancode.h"
#include "SDL_timer.h"
#include "core.h"

using Colour = Vector4<f32>;
Colour make_colour(f32 r, f32 g, f32 b, f32 a) {
    return make_vector4(r, g, b, a);
}

#define SDL_COLOUR(colour)                                                     \
  (int)(colour.x * 255.0f), (int)(colour.y * 255.0f),                          \
      (int)(colour.z * 255.0f), (int)(colour.w * 255.0f)

using Coordinate = Vector2<i32>;

void draw_rect_filled(SDL_Renderer *renderer, Vector2<int> position,
                      Vector2<int> size, Colour colour) {
    SDL_SetRenderDrawColor(renderer, SDL_COLOUR(colour));

    SDL_Rect rect;
    rect.x = position.x;
    rect.y = position.y;
    rect.w = size.x;
    rect.h = size.y;

    SDL_RenderFillRect(renderer, &rect);
}

void draw_text(SDL_Renderer *renderer, TTF_Font *font, Vector2<int> position,
               const char *text, u8 r = 255, u8 g = 255, u8 b = 255,
               u8 _a = 255) {
    SDL_Color sdl_colour = {r, g, b};
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, sdl_colour);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect rect;
    rect.x = position.x;
    rect.y = position.y;
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
}

struct Tetromino {
    Coordinate coordinate = {};
    Vec<Coordinate> pieces = {};

    u32 last_tick = 0;
};

struct LockedIn {
    Coordinate coordinate = {};
    Colour colour = {};

    bool is_clearing = false;
    f32  clear_t = 0.0f;

    bool is_dropping = false;
    f32  drop_t = 0.0f;
};

bool running = true;
i32 grid_width = 8;
i32 grid_height = 8;
i32 tile_width = 60;
i32 tile_height = 60;

auto now = SDL_GetPerformanceCounter();
auto last = now;
f32 delta_time = 0.0f;
f32 current_frame_time = 0.0f;

u64 total_frames = 0;
u64 total_ticks = 0;
f32 default_frame_time = 1.0f;
f32 speed_up_factor = 4.0f;
f32 frame_time = default_frame_time;

f32 clear_animation_time = frame_time * 0.5f;
f32 drop_animation_time = clear_animation_time * 0.5f;

enum class GameState {
  playing,
  game_over,
};

GameState game_state = GameState::playing;

bool is_in_bounds(Coordinate coordinate) {
    return coordinate.x >= 0 && coordinate.x < grid_width && coordinate.y >= 0 &&
            coordinate.y < grid_height;
}

void next_tetromino(Tetromino &tetromino) {
    tetromino.coordinate = make_vector2(3, 0);
    tetromino.last_tick = 0;
    tetromino.pieces.clear();

    switch (rand() % 7) {
    case 0: {
        tetromino.pieces.push_back(make_vector2(0, 0));
        tetromino.pieces.push_back(make_vector2(1, 0));
        tetromino.pieces.push_back(make_vector2(2, 0));
        tetromino.pieces.push_back(make_vector2(3, 0));
    } break;

    case 1: {
        tetromino.pieces.push_back(make_vector2(0, 0));
        tetromino.pieces.push_back(make_vector2(1, 0));
        tetromino.pieces.push_back(make_vector2(2, 0));
        tetromino.pieces.push_back(make_vector2(1, 1));
    } break;

    case 2: {
        tetromino.pieces.push_back(make_vector2(0, 0));
        tetromino.pieces.push_back(make_vector2(1, 0));
        tetromino.pieces.push_back(make_vector2(2, 0));
        tetromino.pieces.push_back(make_vector2(0, 1));
    } break;

    case 3: {
        tetromino.pieces.push_back(make_vector2(0, 0));
        tetromino.pieces.push_back(make_vector2(1, 0));
        tetromino.pieces.push_back(make_vector2(1, 1));
        tetromino.pieces.push_back(make_vector2(2, 1));
    } break;

    case 4: {
        tetromino.pieces.push_back(make_vector2(0, 1));
        tetromino.pieces.push_back(make_vector2(1, 1));
        tetromino.pieces.push_back(make_vector2(1, 0));
        tetromino.pieces.push_back(make_vector2(2, 0));
    } break;

    case 5: {
        tetromino.pieces.push_back(make_vector2(0, 0));
        tetromino.pieces.push_back(make_vector2(1, 0));
        tetromino.pieces.push_back(make_vector2(1, 1));
        tetromino.pieces.push_back(make_vector2(2, 0));
    } break;

    case 6: {
        tetromino.pieces.push_back(make_vector2(0, 0));
        tetromino.pieces.push_back(make_vector2(1, 0));
        tetromino.pieces.push_back(make_vector2(1, 1));
        tetromino.pieces.push_back(make_vector2(2, 1));
    } break;

    default: {
        printf("Unknown tetromino type\n");
        exit(1);
    } break;
    }
}

bool tetromino_fits(Tetromino &tetromino, Coordinate target,
                    Vec<LockedIn> &locked_in) {
    auto tetromino_target = vector2_add(tetromino.coordinate, target);
    for (auto &piece : tetromino.pieces) {
        auto piece_coordinate = vector2_add(tetromino_target, piece);

        if (!is_in_bounds(piece_coordinate)) {
        return false;
        }

        for (auto &locked : locked_in) {
            if (vector2_equal(locked.coordinate, piece_coordinate)) {
                return false;
            }
        }

        // Could break out early if !can_drop
    }

    return true;
}

bool rotate_tetromino(Tetromino &tetromino, Vec<LockedIn> &locked_in) {
    auto old_pieces = tetromino.pieces;
    for (auto &piece : tetromino.pieces) {
        auto old_x = piece.x;
        piece.x = piece.y;
        piece.y = -old_x;
    }

    if (!tetromino_fits(tetromino, make_vector2(0, 0), locked_in)) {
        tetromino.pieces = old_pieces;
        return false;
    }

    return true;
}

u32 try_to_move_tetromino(Tetromino &tetromino, Vec<LockedIn> &locked_in) {
    auto drop_offset = make_vector2(0, 1);
    auto can_drop = tetromino_fits(tetromino, drop_offset, locked_in);
    u32 score = 0;

    if ((float)(SDL_GetTicks() - tetromino.last_tick) > frame_time * 1000.0f) {
        if (!can_drop) {
            for (auto &piece : tetromino.pieces) {
                LockedIn locked;
                locked.coordinate = vector2_add(tetromino.coordinate, piece);
                locked.colour = make_colour(0.2f, 0.1f, 0.3f, 1.0f);
                locked_in.push_back(locked);
                score += 1; // +1 score for every piece locked in.
            }

            next_tetromino(tetromino);
        } else {
            tetromino.coordinate = vector2_add(tetromino.coordinate, drop_offset);
            score += 1; // +1 score for every time the tetromino moves down.
        }

        tetromino.last_tick = SDL_GetTicks();

        // Check if you can clear any lines
        auto lines_cleared_so_far = 0;
        if (!locked_in.empty()) {
            for (int y = grid_height - 1; y >= 0; --y) {
                bool can_clear_line = true;
                for (int x = 0; x < grid_width; ++x) {
                    auto piece_exists = locked_in.end() != std::find_if(locked_in.begin(), locked_in.end(), [&](LockedIn &locked) { return vector2_equal(locked.coordinate, make_vector2(x, y)); });
                    if (!piece_exists) can_clear_line = false;
                }

                if (can_clear_line) {
                    for (auto &locked : locked_in) {
                        if (locked.coordinate.y == y && !locked.is_clearing) {
                            locked.is_clearing = true;
                            locked.clear_t = 0.0f;
                        }
                    }

                    lines_cleared_so_far += 1;
                    score += grid_width * 10 * lines_cleared_so_far;
                }
            }
        }

        // Check if the game is over
        for (auto &locked : locked_in) {
            if (locked.coordinate.y == 0) {
                game_state = GameState::game_over;
            }
        }
    }

    return score;
}

int main(int argc, char *argv[]) {
    // Init SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    auto window_width = grid_width * tile_width;
    auto window_height = grid_height * tile_height;

    SDL_Window *window = SDL_CreateWindow("SDL2Test", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, window_width,
                                          window_height, 0);
    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    auto font = TTF_OpenFont("assets/fonts/font.ttf", 24);
    auto font_colour = SDL_Color{255, 255, 255};
    auto font_surface = TTF_RenderText_Solid(font, "Hello World", font_colour);
    auto font_texture = SDL_CreateTextureFromSurface(renderer, font_surface);

    // Init game state
    Tetromino tetromino = {};
    Vec<LockedIn> locked_in = {};
    u32 score = 0;

    next_tetromino(tetromino);

    // Game loop
    while (running) {
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: {
                    running = false;
                } break;

                case SDLK_s: {
                    frame_time = default_frame_time / speed_up_factor;
                } break;

                case SDLK_a: {
                    if (game_state == GameState::playing) {
                        tetromino.coordinate.x -= 1;
                        if (!tetromino_fits(tetromino, make_vector2(0, 0),
                                            locked_in)) {
                            tetromino.coordinate.x += 1;
                        }
                    }
                } break;

                case SDLK_d: {
                    if (game_state == GameState::playing) {
                        tetromino.coordinate.x += 1;
                        if (!tetromino_fits(tetromino, make_vector2(0, 0),
                                            locked_in)) {
                            tetromino.coordinate.x -= 1;
                        }
                    }
                } break;

                case SDLK_SPACE: {
                    if (game_state == GameState::playing) {
                        auto did_rotate =
                            rotate_tetromino(tetromino, locked_in);
                    }
                } break;
                }
            }
            else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                case SDLK_s: {
                    frame_time = default_frame_time;
                } break;
                }
            }
        }

        last = now;
        now = SDL_GetPerformanceCounter();
        delta_time = (f32)((now - last) / (f32)SDL_GetPerformanceFrequency());
        current_frame_time += delta_time;

        // Try to move the tetromino
        score += try_to_move_tetromino(tetromino, locked_in);

        // Update the clear time
        // clear_t += delta_time;
        Vec<i32> lines_cleared = {};
        for (auto it = locked_in.begin(); it != locked_in.end();) {
            if (it->is_clearing) {
                if (it->clear_t < clear_animation_time) {
                    it->clear_t += delta_time;
                    ++it;
                } else {
                    if (lines_cleared.end() == std::find(lines_cleared.begin(), lines_cleared.end(), it->coordinate.y)) {
                        lines_cleared.push_back(it->coordinate.y);
                    }

                    it = locked_in.erase(it);
                }
            }
            else if (it->is_dropping) {
                if (it->drop_t < drop_animation_time) {
                    it->drop_t += delta_time;
                    ++it;
                }
                else {
                    it->is_dropping = false;
                    it->coordinate.y += 1;

                    // If there are any empty lines, push down lines above them
                    for (auto y = 0; y < grid_height; ++y) {
                        bool line_is_empty = true;
                        for (auto x = 0; x < grid_width; ++x) {
                            auto piece_exists = locked_in.end() != std::find_if(locked_in.begin(), locked_in.end(), [&](LockedIn &locked) {
                                return vector2_equal(locked.coordinate, make_vector2(x, y));
                            });
                            if (!piece_exists) line_is_empty = false;
                        }

                        if (line_is_empty) {
                            for (auto &locked : locked_in) {
                                if (locked.coordinate.y < y && !locked.is_dropping) {
                                    locked.is_dropping = true;
                                    locked.drop_t = 0.0f;
                                }
                            }
                        }
                    }

                    ++it;
                }
            }
            else {
                ++it;
            }
        }

        // Move the lines down
        for (auto &line : lines_cleared) {
            for (auto &locked : locked_in) {
                if (locked.coordinate.y < line && !locked.is_dropping) {
                    locked.is_dropping = true;
                    locked.drop_t = 0.0f;
                }
            }
        }


        // Draw
        i32 window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);

        auto tile_size = make_vector2(window_width / grid_width, window_height / grid_height);

        SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        if (game_state == GameState::playing) {
            for (i32 y = 0; y < grid_height; ++y) {
                for (i32 x = 0; x < grid_width; ++x) {
                    draw_rect_filled(renderer, make_vector2(x * tile_size.x, y * tile_size.y), tile_size, make_colour(0.1f, 0.1f, 0.1f, 1.0f));
                }
            }

            for (auto &piece : tetromino.pieces) {
                draw_rect_filled(
                    renderer,
                    make_vector2(
                        (tetromino.coordinate.x + piece.x) * tile_width,
                        (tetromino.coordinate.y + piece.y) * tile_height),
                    make_vector2(tile_width, tile_height),
                    make_colour(0.6f, 0.1f, 0.3f, 1.0f));
            }

            draw_rect_filled(
                renderer,
                make_vector2((tetromino.coordinate.x) * tile_width,
                             (tetromino.coordinate.y) * tile_height),
                make_vector2(10, 10), make_colour(0.0f, 1.0f, 1.0f, 1.0f));

            for (auto &locked : locked_in) {
                auto size_multiplier = 1.0f - (locked.clear_t / clear_animation_time);
                auto size = make_vector2((int)(tile_width), (int)(tile_height * size_multiplier));

                auto position = make_vector2(locked.coordinate.x * tile_width, locked.coordinate.y * tile_height);
                if (locked.is_dropping) {
                    position.y += (tile_height * locked.drop_t / drop_animation_time);
                }

                draw_rect_filled(renderer, position, size, locked.colour);

                draw_rect_filled(renderer, position, vector2_div(size, 10), make_colour(0.0f, 1.0f, 1.0f, 1.0f));
            }
        }

        auto score_string = std::to_string(score);
        draw_text(renderer, font, make_vector2(0, 0), score_string.c_str(), 255, 0, 0);

        auto fps_string = fmt::format("FPS: {}", (int)(1.0f / delta_time));
        draw_text(renderer, font, make_vector2(0, 20), fps_string.c_str(), 255, 0, 0);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();

    return 0;
}

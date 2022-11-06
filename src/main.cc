#include <SDL2/SDL.h>

#include "core.h"


using Colour = Vector4<f32>;
Colour make_colour(f32 r, f32 g, f32 b, f32 a) {
  return make_vector4(r, g, b, a);
}

#define SDL_COLOUR(colour)  (int)(colour.x*255.0f), (int)(colour.y*255.0f), (int)(colour.z*255.0f), (int)(colour.w*255.0f)


void
fill_rect(SDL_Renderer* renderer, Vector2<int> position, Vector2<int> size, Colour colour) {
  SDL_SetRenderDrawColor(renderer, SDL_COLOUR(colour));

  SDL_Rect rect;
  rect.x = position.x;
  rect.y = position.y;
  rect.w = size.x;
  rect.h = size.y;

  SDL_RenderFillRect(renderer, &rect);
}


struct Tetromino {
  Vector2<i32>      position = {};
  Vec<Vector2<i32>> pieces = {};
  u32 last_tick = 0;
};

struct LockedIn {
  Vector2<i32> position = {};
  Colour colour = {};
};

bool running = true;
u32  grid_width = 8;
u32  grid_height = 14;

u64  total_frames = 0;
u64  total_ticks = 0;


bool is_in_bounds(Vector2<i32> position) {
  return position.x >= 0 && position.x < grid_width &&
         position.y >= 0 && position.y < grid_height;
}

int
main(int argc, char *argv[])
{
  // Init SDL
  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_Window *window = SDL_CreateWindow("SDL2Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 480, 640, 0);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


  // Init game state
  Vec<Tetromino> tetrominos = {};
  Vec<LockedIn>  locked_in = {};

  {
    Tetromino tetromino;
    tetromino.position = make_vector2(0, 0);
    tetromino.pieces.push_back(make_vector2(0, 0));
    tetromino.pieces.push_back(make_vector2(1, 0));
    tetrominos.push_back(tetromino);
  }

  


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
        }
      }
    }


    for (auto& tetromino : tetrominos) {
      bool can_drop = true;
      for (auto piece : tetromino.pieces) {
        if (!is_in_bounds(vector2_add(tetromino.position, piece))) {
          can_drop = false;
          break;
        }

        for (auto locked : locked_in) {
          if (vector2_equal(locked.position, vector2_add(tetromino.position, piece))) {
            can_drop = false;
            break;
          }
        }
      }

      if (!can_drop) {
        for (auto piece : tetromino.pieces) {
          LockedIn locked;
          locked.position = vector2_add(tetromino.position, piece);
          locked.colour = make_colour(1.0f, 0.0f, 0.0f, 1.0f);
          locked_in.push_back(locked);
        }
      }
      else {
        tetromino.position.y += 1;
      }

      tetromino.last_tick = SDL_GetTicks();
    }



    // Draw
    int window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);

    int tile_width = window_width/grid_width;
    int tile_height = window_height/grid_height;

    SDL_SetRenderDrawColor(renderer, 28, 28, 28, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);


    for (i32 y = grid_height-1; y >= 0; --y) {
      for (i32 x = 0; x < grid_width; ++x) {
        fill_rect(renderer, make_vector2(x*tile_width, y*tile_height), make_vector2(tile_width, tile_height), make_colour(0.8f, 0.6f, 0.1f, 1.0f));
      }
    }

    for (auto& tetromino : tetrominos) {
      for (auto piece : tetromino.pieces) {
        fill_rect(renderer,
                  make_vector2((tetromino.position.x + piece.x)*tile_width, (tetromino.position.y + piece.y)*tile_height),
                  make_vector2(tile_width, tile_height),
                  make_colour(0.8f, 0.1f, 0.1f, 1.0f));
      }
    }

    for (auto& locked : locked_in) {
      fill_rect(renderer,
                make_vector2(locked.position.x*tile_width, locked.position.y*tile_height),
                make_vector2(tile_width, tile_height),
                locked.colour);
    }


    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

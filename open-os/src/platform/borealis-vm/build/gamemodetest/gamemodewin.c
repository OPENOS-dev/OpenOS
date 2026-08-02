// Copyright 2021 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static void window_message_pump(bool full_screen) {
  if (0 > SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "Failed to SDL_Init: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  const int kWindowWidth = 640;
  const int kWindowHeight = 480;
  Uint32 flags = full_screen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
  SDL_Window *win = SDL_CreateWindow("Game Mode", SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED, kWindowWidth,
                                     kWindowHeight, flags);
  if (NULL == win) {
    fprintf(stderr, "Failed to SDL_CreateWindow: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  bool prev_focused = false;
  while (1) {
    SDL_Event e;
    if (!SDL_WaitEvent(&e)) {
      fprintf(stderr, "Failed to SDL_WaitEvent: %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
    }
    switch (e.type) {
    case SDL_QUIT:
      return;

    case SDL_KEYDOWN:
      if (e.key.keysym.sym == SDLK_ESCAPE) {
        return;
      }
      break;

    case SDL_WINDOWEVENT:
      fprintf(stdout, "Window event received: %u\n", e.window.event);
      if (SDL_WINDOWEVENT_EXPOSED == e.window.event) {
        SDL_Surface *surface = SDL_GetWindowSurface(win);
        if (!surface) {
          fprintf(stderr, "Failed to SDL_GetWindowSurface: %s\n",
                  SDL_GetError());
          exit(EXIT_FAILURE);
        }
        // A pleasant shade of pink, ;-).
        Uint32 color = SDL_MapRGB(surface->format, 255, 0, 255);
        if (0 > SDL_FillRect(surface, NULL, color)) {
          fprintf(stderr, "Failed to SDL_FillRect: %s\n", SDL_GetError());
          exit(EXIT_FAILURE);
        }
        if (0 > SDL_UpdateWindowSurface(win)) {
          fprintf(stderr, "Failed to SDL_UpdateWindowSurface: %s\n",
                  SDL_GetError());
          exit(EXIT_FAILURE);
        }
      } else if (SDL_WINDOWEVENT_FOCUS_GAINED == e.window.event) {
        prev_focused = true;
      } else if (full_screen && prev_focused) {
        // If we started full-screen, make sure we never lose it.
        // NB: We can't just check SDL_GetWindowFlags because a window can be
        // minimized, but keep its SDL_WINDOW_FULLSCREEN flag.
        switch (e.window.event) {
        case SDL_WINDOWEVENT_FOCUS_LOST:
        case SDL_WINDOWEVENT_HIDDEN:
        case SDL_WINDOWEVENT_LEAVE:
        case SDL_WINDOWEVENT_MINIMIZED:
          fprintf(stderr, "Window exited full-screen\n");
          exit(EXIT_FAILURE);
        }
      }
      break;
    }
  }
}

static void print_usage(const char *exe) {
  printf("%s <full-screen>\n", exe);
  printf("  Create a window that can be full-screen.\n");
  printf("\n");
  printf("Arguments:\n");
  printf("  full-sreen: \"true\" | \"false\" - If true, the created window\n");
  printf("    is full-screen.\n");
  printf("\n");
}

int main(int argc, char *argv[]) {
  if (2 != argc) {
    print_usage(argv[0]);
    fprintf(stderr, "Expected 1 arg, got %d\n", argc - 1);
    return EXIT_FAILURE;
  }
  if (0 == strcmp("false", argv[1])) {
    window_message_pump(false);
  } else if (0 == strcmp("true", argv[1])) {
    window_message_pump(true);
  } else {
    print_usage(argv[0]);
    fprintf(stderr,
            "<full-screen> should be \"true\" or \"false\", got \"%s\"\n",
            argv[3]);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

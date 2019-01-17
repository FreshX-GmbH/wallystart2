#ifndef _WALLYSTART_H
#define _WALLYSTART_H

#include "wally.h"
#ifdef RASPBERRY
#include "bcm_host.h"
#endif

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font *showFont = NULL;
SDL_Event event;
bool quit = false;
bool niceing = false;
bool repaint = false;
bool eventLoop = false;
int rot = 0, h = 0, w = 0;
char *cmdStr = NULL;
char *startupScript = "/etc/wallyd.d";
void* globalSLG;
char *color;
int bindPort = 1109;
// If answer == NULL, the listener will echo back the input
// otherwise answer is sent
char *answer = "OK\r\n";
char *showText = NULL;
int showFontSize = 0;
int errno;

Uint32 SDL_CMD_EVENT;
Uint32 SDL_UPD_EVENT;
Uint32 SDL_ALLOC_EVENT;
Uint32 SDL_DESTROY_EVENT;
Uint32 SDL_LOADIMAGE_EVENT;

// Pascal style
struct texts *textFields[TEXT_SLOTS];
struct texture *textures[TEXTURE_SLOTS];

#endif

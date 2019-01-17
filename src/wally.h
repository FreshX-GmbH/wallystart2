#ifndef _WALLY_H
#define _WALLY_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <signal.h>
#include "slog.h"
#include "autoversion.h"
#ifdef RASPBERRY
#include "bcm_host.h"
#endif

#define TEXT_SLOTS 255
#define TEXTURE_SLOTS 3
#define VERSION "0.4"
#define BASE "."
#define FONT "/etc/wallyd.d/fonts/Cairo-Regular.ttf"
#define START WALLYD_CONFDIR"/wallystart.conf"

int EventFilter(void *userdata, SDL_Event *event);
bool initGFX(void);
bool initThreadsAndHandlers(void *p);
void cleanupGFX();
void renderTexts(void);
bool setupText(int id, int x, int y, int size, char *color, long timeout, char *str);
TTF_Font *loadFont(char *file, int size);
void update(int id);
SDL_Texture* loadImage(char *name);
void closeSDL();
SDL_Texture* renderLog(char *strTmp,int *w, int *h);
bool processCommand(char *buf);
char *repl_str(const char *str, const char *from, const char *to);
void hexToColor(char * colStr, SDL_Color *c);
void sig_handler(int);
SDL_Texture* renderText(char *, int, char *, int *, int *);
int getNumOrPercentEx(char *str, int relativeTo, int *value, int base);
bool fadeImage(SDL_Texture *text, bool reverse, long delay);
SDL_Texture *fadeOver(SDL_Texture *to, SDL_Texture *from, SDL_Texture *temp, int step);
void clearText(int id);
void copyTexture(int from, int to);
void destroyTexture(int id);
void resetTexture(int id);

char *repl_str(const char *str, const char *from, const char *to);
int sgetline(int fd, char ** out);

// Threads
void *processScript(void*);
void *logListener(void *);
void *faderThread(void *);
void *timerThread(void *);

typedef struct texture {
  int x;
  int y;
  int w;
  int h;
  int alpha;
  SDL_Color color;
  SDL_Texture *tex;
  bool active;
  int fadein;
  int fadeout;
  int fadeover;
  int fadesrc;
  int fadeloop;
  bool dirty;
  // struct timespec fadedelay;
  long duration;
  uint32_t fadeStart;
} texture;

typedef struct texts {
  int x;
  int y;
  int w;
  int h;
  int size;
  int alpha;
  SDL_Color color;
  SDL_Texture *tex;
  char *str;
  long timeout;
  bool active;
  bool destroy;
  bool dirty;
  uint32_t created;
} texts;

extern bool dirty;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern TTF_Font *showFont;
extern bool quit;
extern bool repaint;
extern int rot, h, w;
extern char *cmdStr;
extern char *startupScript;
extern char *color;
extern char *answer;
extern char *showText;
extern int showFontSize;
extern struct texts *textFields[TEXT_SLOTS];
extern struct texture *textures[TEXTURE_SLOTS];
extern int errno;
extern int bindPort;

// extern Uint32 SDL_CMD_EVENT;
extern Uint32 SDL_UPD_EVENT;
extern Uint32 SDL_ALLOC_EVENT;
extern Uint32 SDL_DESTROY_EVENT;
extern Uint32 SDL_LOADIMAGE_EVENT;

extern pthread_t startup_thr;
#endif

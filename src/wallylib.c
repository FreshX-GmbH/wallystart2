#include "wally.h"

pthread_t log_thr;
pthread_t startup_thr;
pthread_t timer_thr;
pthread_t fader_thr;

void PrintEvent(const SDL_Event *event)
{
    if (event->type == SDL_WINDOWEVENT)
    {
        switch (event->window.event)
        {
        case SDL_WINDOWEVENT_SHOWN:
            slog(TRACE, LOG_CORE, "Window %d shown", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            slog(TRACE, LOG_CORE, "Window %d hidden", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            slog(TRACE, LOG_CORE, "Window %d exposed", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MOVED:
            slog(TRACE, LOG_CORE, "Window %d moved to %d,%d",
                 event->window.windowID, event->window.data1,
                 event->window.data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            slog(TRACE, LOG_CORE, "Window %d resized to %dx%d",
                 event->window.windowID, event->window.data1,
                 event->window.data2);
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            slog(TRACE, LOG_CORE, "Window %d size changed to %dx%d",
                 event->window.windowID, event->window.data1,
                 event->window.data2);
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            slog(TRACE, LOG_CORE, "Window %d minimized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            slog(TRACE, LOG_CORE, "Window %d maximized", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            slog(TRACE, LOG_CORE, "Window %d restored", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            slog(TRACE, LOG_CORE, "Mouse entered window %d",
                 event->window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
            slog(TRACE, LOG_CORE, "Mouse left window %d", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            slog(TRACE, LOG_CORE, "Window %d gained keyboard focus",
                 event->window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            slog(TRACE, LOG_CORE, "Window %d lost keyboard focus",
                 event->window.windowID);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            slog(TRACE, LOG_CORE, "Window %d closed", event->window.windowID);
            break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
        case SDL_WINDOWEVENT_TAKE_FOCUS:
            slog(TRACE, LOG_CORE, "Window %d is offered a focus", event->window.windowID);
            break;
        case SDL_WINDOWEVENT_HIT_TEST:
            slog(TRACE, LOG_CORE, "Window %d has a special hit test", event->window.windowID);
            break;
#endif
        default:
            slog(TRACE, LOG_CORE, "Window %d got unknown event %d",
                 event->window.windowID, event->window.event);
            break;
        }
    }
}

int EventFilter(void *userdata, SDL_Event *event)
{
    // quit on ESC
    if (event->type == SDL_KEYDOWN)
    {
        if (event->key.keysym.sym == SDLK_ESCAPE)
        {
            slog(TRACE, LOG_CORE, "ESC pressed. quiting program.");
            quit = true;
            return true;
        }
        else
        {
            return false;
        }
    }
    if (event->type == SDL_WINDOWEVENT)
    {
        if (event->window.event == SDL_WINDOWEVENT_RESIZED)
        {
            w = event->window.data1;
            h = event->window.data2;
            SDL_GL_GetDrawableSize(window, &w, &h);
            slog(DEBUG, LOG_CORE, "Window resized to %dx%d", w, h);
            return true;
        }
        if (event->window.event == SDL_WINDOWEVENT_EXPOSED)
        {
            slog(DEBUG, LOG_CORE, "Window exposed. Redraw needed.");
            repaint = true;
            return true;
        }
        PrintEvent(event);
        return false;
    }
    if (event->type == SDL_MOUSEBUTTONDOWN ||
        event->type == SDL_APP_TERMINATING ||
        event->type == SDL_QUIT)
    {
        slog(TRACE, LOG_CORE, "Mouse pressed. quiting program.");
        quit = true;
    }
    // ignore some events
    if (event->type == SDL_MOUSEMOTION)
    {
        return false;
    }
    return true;
}

bool dumpSDLInfo()
{
    int i;
    printf("\nCheck SDL Window enabled flags:\n");
    int flags = SDL_GetWindowFlags(window);
    printf("    SDL_WINDOW_FULLSCREEN    [%c]\n", (flags & SDL_WINDOW_FULLSCREEN) ? 'X' : ' ');
    printf("    SDL_WINDOW_OPENGL        [%c]\n", (flags & SDL_WINDOW_OPENGL) ? 'X' : ' ');
    printf("    SDL_WINDOW_SHOWN         [%c]\n", (flags & SDL_WINDOW_SHOWN) ? 'X' : ' ');
    printf("    SDL_WINDOW_HIDDEN        [%c]\n", (flags & SDL_WINDOW_HIDDEN) ? 'X' : ' ');
    printf("    SDL_WINDOW_BORDERLESS    [%c]\n", (flags & SDL_WINDOW_BORDERLESS) ? 'X' : ' ');
    printf("    SDL_WINDOW_RESIZABLE     [%c]\n", (flags & SDL_WINDOW_RESIZABLE) ? 'X' : ' ');
    printf("    SDL_WINDOW_MINIMIZED     [%c]\n", (flags & SDL_WINDOW_MINIMIZED) ? 'X' : ' ');
    printf("    SDL_WINDOW_MAXIMIZED     [%c]\n", (flags & SDL_WINDOW_MAXIMIZED) ? 'X' : ' ');
    printf("    SDL_WINDOW_INPUT_GRABBED [%c]\n", (flags & SDL_WINDOW_INPUT_GRABBED) ? 'X' : ' ');
    printf("    SDL_WINDOW_INPUT_FOCUS   [%c]\n", (flags & SDL_WINDOW_INPUT_FOCUS) ? 'X' : ' ');
    printf("    SDL_WINDOW_MOUSE_FOCUS   [%c]\n", (flags & SDL_WINDOW_MOUSE_FOCUS) ? 'X' : ' ');
    printf("    SDL_WINDOW_FOREIGN       [%c]\n", (flags & SDL_WINDOW_FOREIGN) ? 'X' : ' ');

    // Allocate a renderer info struct
    SDL_RendererInfo *rend_info = (SDL_RendererInfo *)malloc(sizeof(SDL_RendererInfo));
    if (!rend_info)
    {
        slog(WARN, LOG_CORE, "Couldn't allocate memory for the renderer info data structure\n");
    }
    // Print the list of the available renderers
    printf("\nAvailable 2D rendering drivers:\n");
    for (i = 0; i < SDL_GetNumRenderDrivers(); i++)
    {
        if (SDL_GetRenderDriverInfo(i, rend_info) < 0)
        {
            slog(WARN, LOG_CORE, "Couldn't get SDL 2D render driver information: %s\n", SDL_GetError());
        }
        printf("%2d: %s\n", i, rend_info->name);
        printf("    SDL_RENDERER_SOFTWARE     [%c]\n", (rend_info->flags & SDL_RENDERER_SOFTWARE) ? 'X' : ' ');
        printf("    SDL_RENDERER_ACCELERATED  [%c]\n", (rend_info->flags & SDL_RENDERER_ACCELERATED) ? 'X' : ' ');
        printf("    SDL_RENDERER_PRESENTVSYNC [%c]\n", (rend_info->flags & SDL_RENDERER_PRESENTVSYNC) ? 'X' : ' ');
    }

    // Print the name of the current rendering driver
    if (SDL_GetRendererInfo(renderer, rend_info) < 0)
    {
        slog(WARN, LOG_CORE, "Couldn't get SDL 2D rendering driver information: %s\n", SDL_GetError());
    }
    printf("Rendering driver in use: %s\n", rend_info->name);
    printf("    SDL_RENDERER_SOFTWARE     [%c]\n", (rend_info->flags & SDL_RENDERER_SOFTWARE) ? 'X' : ' ');
    printf("    SDL_RENDERER_ACCELERATED  [%c]\n", (rend_info->flags & SDL_RENDERER_ACCELERATED) ? 'X' : ' ');
    printf("    SDL_RENDERER_PRESENTVSYNC [%c]\n", (rend_info->flags & SDL_RENDERER_PRESENTVSYNC) ? 'X' : ' ');
    return true;
}

bool dumpModes()
{
    SDL_DisplayMode mode;
    SDL_Rect r;
    int j, i, display_count;
    Uint32 f;
    if ((display_count = SDL_GetNumVideoDisplays()) < 1)
    {
        slog(WARN, LOG_CORE, "VideoDisplay count = 0");
        return false;
    }
    slog(INFO, LOG_CORE, "VideoDisplays: %i", display_count);

    for (j = 0; j < display_count; j++)
    {
        SDL_GetDisplayBounds(j, &r);
        slog(DEBUG, LOG_CORE, "Display %d boundaries : %d x %d", j, r.w, r.h);
        // Store size of first display
        if (j == 0)
        {
            w = r.w;
            h = r.h;
        }
        for (i = 0; i < SDL_GetNumDisplayModes(j); i++)
        {
            SDL_GetDisplayMode(j, i, &mode);
            f = mode.format;
            slog(DEBUG, LOG_CORE, "Display %d / Mode %d : %d x %d x %d bpp (%s) @ %d Hz", j, i, mode.w, mode.h, SDL_BITSPERPIXEL(f), SDL_GetPixelFormatName(f), mode.refresh_rate);
        }
    }
    return true;
}

bool loadSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        slog(ERROR, LOG_CORE, "SDL could not initialize! SDL Error: %s", IMG_GetError());
        return false;
    }
    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    int initted = IMG_Init(flags);
    if ((initted & flags) != flags) {
        slog(ERROR, LOG_CORE, "SDL_image could not initialize PNG and JPG! SDL_image Error: %s", IMG_GetError());
        return false;
    }
    if (TTF_Init() == -1) {
        slog(ERROR, LOG_CORE, "SDL_TTF could not initialize! SDL_ttf Error: %s", TTF_GetError());
        return false;
    }
    SDL_ShowCursor(0);

#ifdef DARWIN
    slog(DEBUG, LOG_CORE, "Starting in Darwin window mode.");
    window = SDL_CreateWindow("wallyd",
                 SDL_WINDOWPOS_CENTERED,
                 SDL_WINDOWPOS_CENTERED, 
                 0, 
                 0, 
                 SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_ALLOW_HIGHDPI);
#else
    window = SDL_CreateWindow("wallyd", 
                 SDL_WINDOWPOS_CENTERED, 
                 SDL_WINDOWPOS_CENTERED, 
                 1920, 
                 1080, 
                 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
#endif

    SDL_ShowCursor(0);
    //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED| SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (renderer == NULL) {
        slog(ERROR, LOG_CORE, "Hardware accelerated renderer could not initialize : %s", IMG_GetError());
        slog(WARN, LOG_CORE, "Falling back to software renderer.");
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
    }

    if (renderer == NULL) {
        slog(ERROR, LOG_CORE, "Renderer could not initialize : %s", SDL_GetError());
        return false;
    }
    SDL_SetWindowInputFocus(window);

    SDL_GetRendererOutputSize(renderer, &w, &h);

    // Temp texture
    textures[2]->tex = SDL_CreateTexture(renderer, 
        SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_TARGET, 
        w, 
        h);

    SDL_SetTextureBlendMode(textures[2]->tex, SDL_BLENDMODE_BLEND);

    return true;
}

SDL_Texture *fadeOver(SDL_Texture *src, SDL_Texture *dest, SDL_Texture *temp, int step)
{
    SDL_SetTextureBlendMode(src, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(dest, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, temp);
    SDL_RenderCopyEx(renderer, dest, NULL, NULL, rot, NULL, SDL_FLIP_NONE);
    SDL_SetTextureAlphaMod(src, step);
    SDL_RenderCopyEx(renderer, src, NULL, NULL, rot, NULL, SDL_FLIP_NONE);
    SDL_SetRenderTarget(renderer, NULL);

    return temp;
}

// bool fadeImage(SDL_Texture *text, bool reverse, long delay)
// {
//     struct timespec t = {0, delay};
//     int v = 0;
//     int i = 0;
//     for (i = 0; i < 255; i += 2) {
//         if (reverse) {
//             v = 255 - i;
//         } else {
//             v = i;
//         }
//         SDL_SetTextureColorMod(text, v, v, v);
//         update(text);
//         nanosleep(&t, NULL);
//     }
//     return true;
// }

// bool fadeImageEx(SDL_Texture *text, bool reverse, long delay)
// {
//     struct timespec t = {0, delay};
//     int v = 0;
//     int i = 0;
//     for (i = 0; i < 255; i += 2) {
//         if (reverse) {
//             v = 255 - i;
//         } else {
//             v = i;
//         }
//         SDL_SetTextureColorMod(text, v, v, v);
//         update(text);
//         nanosleep(&t, NULL);
//     }
//     return true;
// }


SDL_Texture *loadImage(char *name)
{
    // SDL_Surface *image = NULL;
    SDL_Texture *text = NULL;

    // if (mode2d == true) {
    //     image = IMG_Load(name);
    //     if (image == NULL) {
    //         slog(ERROR, LOG_CORE, "Unable to load image %s! SDL Error: %s", name, SDL_GetError());
    //         return false;
    //     }
    //     SDL_Surface *optimizedSurface = SDL_ConvertSurface(image, screenSurface->format, 0);

    //     if (SDL_BlitScaled(optimizedSurface, NULL, screenSurface, NULL)) {
    //         slog(ERROR, LOG_CORE, "Unable to blit image %s! SDL Error: %s", name, SDL_GetError());
    //         return false;
    //     }
    //     SDL_UpdateWindowSurface(window);
    // } else {
    text = IMG_LoadTexture(renderer, name);
    if (text == NULL) {
        slog(ERROR, LOG_CORE, "Error loading image : %s", IMG_GetError());
        return false;
    }
    SDL_SetTextureBlendMode(text, SDL_BLENDMODE_BLEND);
    // }
    return text;
}

TTF_Font *loadFont(char *file, int size)
{
    TTF_Font *font = TTF_OpenFont(file, size);
    if (font == NULL) {
        slog(ERROR, LOG_CORE, "Failed to load font : %s ", TTF_GetError());
        return NULL;
    } else {
        return font;
    }
}

void closeSDL()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    window = NULL;
    renderer = NULL;
    SDL_Quit();
}

bool initGFX(void)
{
    int i;
#ifdef RASPBERRY
    bcm_host_init();
    slog(INFO, LOG_TEXTURE, "Initializing broadcom hardware");
#endif
#ifndef DARWIN
    slog(DEBUG, LOG_TEXTURE, "Enable SDL2 verbose logging");
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#endif

    int bytes = sizeof(struct texts);
    for (i = 0; i < TEXT_SLOTS; i++)
    {
        textFields[i] = malloc(bytes);
        memset(textFields[i], 0, bytes);
        textFields[i]->active = false;
        textFields[i]->destroy = false;
    }
    bytes = sizeof(struct texture);
    for (i = 0; i < TEXTURE_SLOTS; i++) {
        textures[i] = malloc(bytes);
        memset(textures[i], 0, bytes);
        textures[i]->active = false;
        textFields[i]->destroy = false;
    }
    slog(DEBUG, LOG_CORE, "Initialized %d bytes", bytes * TEXT_SLOTS);

    dumpModes();

    if (!loadSDL()) {
        slog(ERROR, LOG_CORE, "Failed to initialize SDL. Exit");
        return false;
    }

    dumpSDLInfo();

    slog(INFO, LOG_CORE, "Screen size : %dx%d", w, h);

    SDL_UPD_EVENT = SDL_RegisterEvents(1);
    slog(DEBUG, LOG_CORE, "SDL_UPD_EVENT : %d", SDL_UPD_EVENT);
    SDL_ALLOC_EVENT = SDL_RegisterEvents(1);
    slog(DEBUG, LOG_CORE, "SDL_ALLOC_EVENT : %d", SDL_ALLOC_EVENT);
    SDL_DESTROY_EVENT = SDL_RegisterEvents(1);
    slog(DEBUG, LOG_CORE, "SDL_DESTROY_EVENT : %d", SDL_DESTROY_EVENT);
//    SDL_CMD_EVENT = SDL_RegisterEvents(1);
//    slog(DEBUG, LOG_CORE, "SDL_CMD_EVENT : %d", SDL_CMD_EVENT);
    SDL_LOADIMAGE_EVENT = SDL_RegisterEvents(1);
    slog(DEBUG, LOG_CORE, "SDL_LOADIMAGE_EVENT : %d", SDL_LOADIMAGE_EVENT);

    return true;
}

void cleanupGFX() {
    int i;
    for (i = 0; i < TEXTURE_SLOTS; i++) {
        if(textures[i]->tex) {
            destroyTexture(i);
        }
        free(textures[i]);
    } 
    for (i = 0; i < TEXT_SLOTS; i++)
    {
        free(textFields[i]);
    }

    closeSDL();
}

void renderTexts(void)
{
    int count = 0;
    int i;
    SDL_Rect r = {0, 0, 0, 0};
    for (i = 0; i < TEXT_SLOTS; i++)
    {
        if (textFields[i]->destroy){
            clearText(i);
            continue;
        }
        if (textFields[i]->active)
        {
            count++;
            SDL_Texture *tex = textFields[i]->tex;
            r.x = textFields[i]->x;
            r.y = textFields[i]->y;
            r.w = textFields[i]->w;
            r.h = textFields[i]->h;
            if (!rot) {
                SDL_RenderCopy(renderer, tex, NULL, &r);
            } else {
                SDL_RenderCopyEx(renderer, tex, NULL, &r, rot, NULL, SDL_FLIP_NONE);
            }
        }
    }
}

void copyTexture(int from, int to) {
    texture *f = textures[from];
    texture *t = textures[to];
    memcpy(t, f, sizeof(struct texture));
}

// Reset texture structure to defaults
void resetTexture(int id) {
    SDL_Texture *t = textures[id]->tex;
    memset(textures[id], 0, sizeof(struct texture));
    textures[id]->active = false;
    textures[id]->tex = t;
}

void destroyTexture(int id) {
    texture *t = textures[id];
    if (t->tex)
    {
        slog(DEBUG, LOG_CORE, "Freeing old tex.");
        SDL_DestroyTexture(t->tex);
        t->active = false;
        dirty = true;
        // update(-1);
    }
}

void clearText(int id) {
    texts *t = textFields[id];
    if (t->tex)
    {
        slog(DEBUG, LOG_CORE, "Freeing old text slot.");
        SDL_DestroyTexture(t->tex);
        t->tex = NULL;
        if(t->str) {
            free(t->str);
            t->str = 0;
        }
        t->active = false;
        dirty = true;
        // Update sollte hier nicht stattfinden (global)
        // update(0);
    }
}

bool setupText(int id, int x, int y, int size, char *color, long timeout, char *str)
{
    SDL_Surface *surf;
    texts *t = textFields[id];

    t->x = x;
    t->y = y;

    slog(DEBUG, LOG_CORE, "Setting up text %d(%d,%d, sz %d, color %s) : %s for %d seconds", id, x, y, size, color, str, timeout);
    if (t->tex)
    {
        slog(DEBUG, LOG_CORE, "Freeing old text slot.");
        SDL_DestroyTexture(t->tex);
        t->tex = NULL;
        if(t->str) {
            free(t->str);
            t->str = NULL;
        }
    }

    t->active = true;
    t->dirty = true;
    t->destroy = false;
    t->str = str;
    t->timeout = timeout;
    t->created = SDL_GetTicks();

    hexToColor(color, &t->color);

    if (!showFont || (showFont && showFontSize != size))
    {
        if (showFont)
            TTF_CloseFont(showFont);
        slog(DEBUG, LOG_CORE, "Loading font %s in size %d", BASE "" FONT, size);
        showFont = loadFont(BASE "" FONT, size);
        if (!showFont)
            return false;
        showFontSize = size;
    }

    surf = TTF_RenderUTF8_Blended(showFont, str, t->color);
    t->tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_SetTextureBlendMode(t->tex, SDL_BLENDMODE_BLEND);

    if (!t->tex)
        return false;
    TTF_SizeUTF8(showFont, str, &t->w, &t->h);
    SDL_FreeSurface(surf);

    return true;
}

void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        quit = true;
        exit(1);
    }
}

bool initThreadsAndHandlers(void *start){
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        slog(ERROR, LOG_CORE, "Could not catch signal.");
        return false;
    }

    // this is waiting to be joined by startup_thr after bind
    if (pthread_create(&log_thr, NULL, &logListener, NULL) != 0) {
        slog(ERROR, LOG_CORE, "Failed to create listener thread!");
        return false;
    }
    // detach startup script reader
    if (pthread_create(&startup_thr, NULL, &processScript, start) != 0) {
        slog(ERROR, LOG_CORE, "Failed to create startup scipt thread!");
        return false;
    }
    // start a fader thread
    if (pthread_create(&fader_thr, NULL, &faderThread, NULL) != 0) {
        slog(ERROR, LOG_CORE, "Failed to create fader thread!");
        return false;
    }
    // start a timer thread
    //if (pthread_create(&timer_thr, NULL, &timerThread, NULL) != 0) {
    //    slog(ERROR, LOG_CORE, "Failed to create timer thread!");
    //    return false;
    //}

    return true;
}

void hexToColor(char *colStr, SDL_Color *c)
{
    int color = strtol(colStr, NULL, 16);
    c->r = (color >> 16) & 0xff;
    c->g = (color >> 8) & 0xff;
    c->b = color & 0xff;
    c->a = 0;
}

// puts the number converted of string *str into *value
// returns false if str is not at num or a percentage
// relativeTo is the base value to calculate the percentage of
// (i.e. 80% of 500 is 400) or the MAX value, if the given value
// is negative, its substracted from the MAX
// Note : the last char of the String must be % if percentage

int getNumOrPercentEx(char *str, int relativeTo, int *value, int base)
{
    int x = 0;
    errno = 0;
    int err = 0;
    if (!str)
    {
        slog(INFO, LOG_UTIL, "getNumOrPercent() : string invalid");
        return false;
    }
    unsigned long len = strlen(str);
    if (str[len - 1] == '%')
    {
        str[len - 1] = '\0';
        if (str)
            x = (int)strtol(str, NULL, 10);
        //else errno = 1;
        str[len - 1] = '%';
        if (errno)
        {
            slog(WARN, LOG_UTIL, "strtol(%s) conversion error %d", str, err);
            return false;
        }
        *value = relativeTo * x / 100;
        slog(TRACE, LOG_UTIL, "it's percent : %s = %d", str, *value);
        return true;
    }
    if (str)
        x = (int)strtol(str, NULL, base);
    if (errno)
    {
        slog(WARN, LOG_UTIL, "strtol(%s) conversion error %d", str, errno);
        return false;
    }
    if (x < 0)
    {
        *value = relativeTo + x;
    }
    else
    {
        *value = x;
    }
    return true;
}

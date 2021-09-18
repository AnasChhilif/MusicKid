#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_log.h>
#include <id3v2lib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*launches the stream and plays the mp3 file*/
void playa(char name[]){
    Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
    Mix_Music *music = Mix_LoadMUS(name);
    Mix_PlayMusic(music, 1);
    while (!SDL_QuitRequested()) {
        SDL_Delay(250);
    }
    Mix_FreeMusic(music);
}

void TextDisplay(char *text, SDL_Renderer *renderer){
    TTF_Font* font = TTF_OpenFont("assets/font.ttf", 1700);
    SDL_Color color = {255, 255, 255};
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    SDL_Rect Message_rect;
    Message_rect.x = 100;
    Message_rect.y = 100;
    Message_rect.w = 200;
    Message_rect.h = 100;
    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
    SDL_RenderPresent(renderer);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
}

void PrintData(ID3v2_frame_text_content* data, char *buffer){
    if((data->encoding == 0) || (data->encoding == 3)){
        strncpy(buffer, data->data, data->size*(sizeof(char)));
        strcat(buffer, "\0");
        return;
    }
    else{
        int u;
        for (int i = 0; i<data->size/2; i++){
            buffer[i] = data->data[2*i];
            u = i;
        }
        buffer[u+1] = '\0';
        return;
    }
}


int main(int argc, char **argv) {
    /*initial variables for initializing SDL, and for grabbing the track name*/
    TTF_Init();
    int result = 0;
    int flags = MIX_INIT_MP3;
    char ender[] = ".mp3";
    char track[100];
    SDL_Renderer *renderer = NULL;
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

/*Initializing SDL and quitting in case of error*/
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0) {
        printf("Failed to init SDL\n");
        exit(1);
    }

    if (flags != (result = Mix_Init(flags))) {
        printf("Could not initialize mixer (result: %d).\n", result);
        printf("Mix_Init: %s\n", Mix_GetError());
        exit(1);
    }
/*creating window to display graphical thingies */
    SDL_Window *window;
        window = SDL_CreateWindow(
        "MusicMan",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        920,                               // width, in pixels
        480,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );
/*again testing for any errors and quitting in case there are*/
    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }
/*opening up renderer and drawing the main background image onto the window*/
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer){
        printf("Error creating renderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Surface *surface = IMG_Load("assets/titled.png");
    if (!window){
        printf("error creating surface\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer,surface);
    SDL_FreeSurface(surface);

/*testing if texture gets created and adjusting accordingly*/
    if(!tex){
        printf("error creating texture : %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
/*rendering the image into the window*/
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, tex, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);


    /*Take the name as a string file then append the .mp3 extension to it*/

    //fgets(track,30,stdin);
    //track[strlen(track)-1] = '\0';
    //strcat(track,ender);
    TextDisplay("test", renderer);
    SDL_Delay(3000);
    //playa(track);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Log("test");
    SDL_Quit();
    return 0;
}


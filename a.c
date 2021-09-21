#include <stdio.h>
#include <iconv.h>
#include <sys/param.h>
#include <id3v2lib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rwops.h>

struct info{
    char title[100];
    char artist[100];
    char year[100];
    char album[100];
};

/*launches the stream and plays the mp3 file*/
void playa(char *name){
    Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
    Mix_Music *music = Mix_LoadMUS(name);
    Mix_PlayMusic(music, 1);
    while (!SDL_QuitRequested()) {
        SDL_Delay(250);
    }
    Mix_FreeMusic(music);
}

void PrintData(ID3v2_frame_text_content* data, char *buffer){
    //checking the encoding of the data to see how we treat it
    //see id3.org docs for how text data is encoded, or just id3 wikipedia in the id3v2 section
    if(data->encoding=0){
        strncpy(buffer, data->data, MAX(99,data->size));
        strcat(buffer,"\0");
    }
    else if(data->encoding=1){
        iconv_t cd = iconv_open("ASCII", "UCS-2");
        size_t in = data->size;
        size_t out = 99;
        iconv(cd, &data->data, &in, &buffer, &out);
    }
    else if(data->encoding=2){
        iconv_t cd = iconv_open("ASCII", "UTF-16BE");
        size_t in = data->size;
        size_t out = 99;
        iconv(cd, &data->data, &in, &buffer, &out);
    }
    else if(data->encoding=3){
        iconv_t cd = iconv_open("ASCII", "UTF-8");
        size_t in = data->size;
        size_t out = 99;
        iconv(cd, &data->data, &in, &buffer, &out);
    }
        return;
}

void GetInfo(ID3v2_tag* tag, struct info *track){

    // Load the fields from the tag
    ID3v2_frame* FrTitle = tag_get_title(tag);
    // We need to parse the frame content to make readable
    ID3v2_frame_text_content* ConTitle = parse_text_frame_content(FrTitle);
    //Calling PrintData() so we could convert any type of encoding to the usual ascii/utf-8
    PrintData(ConTitle, track->title);
    printf("%x\n", track->title);

    //Rinse and repeat for all relevant text data we might need to display
    ID3v2_frame* FrArtist = tag_get_artist(tag);
    ID3v2_frame_text_content* ConArtist = parse_text_frame_content(FrArtist);
    PrintData(ConArtist, track->artist);
    printf("%x\n", track->artist);

    ID3v2_frame* FrYear = tag_get_year(tag);
    ID3v2_frame_text_content* ConYear = parse_text_frame_content(FrYear);
    PrintData(ConYear, track->year);
    printf("%x\n", track->year);

    ID3v2_frame* FrAlbum = tag_get_album(tag);
    ID3v2_frame_text_content* ConAlbum = parse_text_frame_content(FrAlbum);
    PrintData(ConAlbum, track->album);
    printf("%x\n", track->album);
    return;
}

void DisplayCover(ID3v2_tag* tag, SDL_Renderer* renderer){
    SDL_Rect cover_pos;
    cover_pos.w = 320;
    cover_pos.h = 320;
    cover_pos.x = 100;
    cover_pos.y = 100;

    ID3v2_frame* cover = tag_get_album_cover(tag);
    ID3v2_frame_apic_content* cover_content = parse_apic_frame_content(cover);
    SDL_RWops *rw = SDL_RWFromConstMem(cover_content->data, cover_content->picture_size);
    SDL_Surface *img = IMG_Load_RW(rw, 1);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, img);
    SDL_FreeSurface(img);
    SDL_RenderCopy(renderer, tex, NULL, &cover_pos);
    SDL_DestroyTexture(tex);
}

void TextDisplay(char *text, SDL_Renderer *renderer, SDL_Rect *titler){
    TTF_Font* font = TTF_OpenFont("assets/font.ttf", 64);
    SDL_Color color = {0, 0, 0};
    SDL_Surface* surfaceMessage = TTF_RenderText_Blended(font, text, color);
    if (surfaceMessage == NULL ){
	printf("Error creating surface : %s\n", SDL_GetError());
	exit(0);
    }

    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    if (Message == NULL ){
	printf("Error creating texture : %s\n", SDL_GetError());
	exit(0);
    }

    int err = SDL_RenderCopy(renderer, Message, NULL, titler);
    if (err <0){
	printf("Error copying to renderer : %s\n", SDL_GetError());
	exit(0);
    }
    SDL_RenderPresent(renderer);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
}


int main(int argc, char *argv[]){
    TTF_Init();
    SDL_Rect titler;
    titler.x = 100;
    titler.y = 150;
    titler.w = 700;
    titler.h = 100;
    struct info track;
    SDL_Surface* bgsurf = NULL;
    SDL_Texture* bgtex = NULL;

    if(argc < 2){
    	printf("Usage: %s <music_file>\n", argv[0]);
	exit(0);
    }
    ID3v2_tag* tag = load_tag(argv[1]); // Load the full tag from the file
    if(tag == NULL)
    {
	    printf("Error loading id3 tag\nCheck if file has id3\n");
	    exit(0);
    }

    GetInfo(tag, &track);
    printf("outside the function\n");
    printf("%x\n", track.title);
    printf("%x\n", track.artist);
    printf("%x\n", track.year);
    printf("%x\n", track.album);
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
    SDL_Window* win;
    SDL_Renderer *renderer = NULL;
    win = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        960,                               // width, in pixels
        540,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    bgsurf = IMG_Load("assets/background.png");
    bgtex = SDL_CreateTextureFromSurface(renderer, bgsurf);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, bgtex, NULL, NULL);

    //DisplayCover(tag, renderer);
    TextDisplay(track.title, renderer, &titler);
    //SDL_RenderPresent(renderer);
    playa(argv[1]);
    SDL_Delay(3000);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

#include <stdio.h>
#include <errno.h>
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

extern int errno;

struct text{
    char text[100];
    Uint16 text16[100];
    int encoding;
};

struct info{
    struct text *title;
    struct text *artist;
    struct text *album;
    struct text *year;
};

void Player(char *name){
//Initializing the mixer API and loading the mp3 file and playing it.
    Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
    Mix_Music *music = Mix_LoadMUS(name);
    Mix_PlayMusic(music, 1);
    while (!SDL_QuitRequested()) {
        SDL_Delay(250);
    }
    Mix_FreeMusic(music);
}

void FillData(ID3v2_frame_text_content* data, struct text *buffer){
    buffer->encoding = data->encoding; // copying the encoding in the id3 frame to the struct var.
    //checking the encoding of the data to see how we treat it
    //see id3.org docs for how text data is encoded, or just id3 wikipedia in the id3v2 section

    if(data->encoding == 0 || data->encoding == 3){
        //If it's encoded in ASCII or UTF-8, it's just copied into the text string.
        strncpy(buffer->text, data->data, MIN(data->size, 99));
        strcat(buffer->text,"\0");
    }
    else if(data->encoding == 1 || data->encoding == 2){
        //If it's UTF-16BE or UCS-2, it gets copied byte by byte to the variable, ignoring frequent
        //garbage values as well as zeros.
        int k = 0;
        Uint8 bufferr;
        for(int i = 0; i< MIN(data->size, 99); i++){
            bufferr = (unsigned char) data->data[i];
            if((bufferr < 254 || bufferr > 255) && bufferr != 0){
                buffer->text16[k] = bufferr;
                k++;
            }
        }

    }
    else{
        // if encoding isn't there, the variable is just filled with an empty value.
        strcpy(buffer->text, "");
        buffer->text16[0] = 0;
    }
    return;
}

void GetInfo(ID3v2_tag* tag, struct info *track){

    // Load the fields from the tag
    ID3v2_frame* FrTitle = tag_get_title(tag);
    // We need to parse the frame content to make readable
    ID3v2_frame_text_content* ConTitle = parse_text_frame_content(FrTitle);
    //Calling PrintData() so we could convert any type of encoding to the usual ascii/utf-8
    FillData(ConTitle, track->title);

    //Rinse and repeat for all relevant text data we might need to display
    ID3v2_frame* FrArtist = tag_get_artist(tag);
    ID3v2_frame_text_content* ConArtist = parse_text_frame_content(FrArtist);
    FillData(ConArtist, track->artist);

    ID3v2_frame* FrYear = tag_get_year(tag);
    ID3v2_frame_text_content* ConYear = parse_text_frame_content(FrYear);
    FillData(ConYear, track->year);

    ID3v2_frame* FrAlbum = tag_get_album(tag);
    ID3v2_frame_text_content* ConAlbum = parse_text_frame_content(FrAlbum);
    FillData(ConAlbum, track->album);
    return;
}

void DisplayCover(ID3v2_tag* tag, SDL_Renderer* renderer){
    // setting the dimensions and position of the album cover
    SDL_Rect cover_pos;
    cover_pos.w = 230;
    cover_pos.h = 230;
    cover_pos.x = 200;
    cover_pos.y = 70;

    //getting the info necessary from the mp3 file then loading and displaying the album cover
    ID3v2_frame* cover = tag_get_album_cover(tag);
    ID3v2_frame_apic_content* cover_content = parse_apic_frame_content(cover);
    SDL_RWops *rw = SDL_RWFromConstMem(cover_content->data, cover_content->picture_size);
    SDL_Surface *img = IMG_Load_RW(rw, 1);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, img);
    SDL_FreeSurface(img);
    SDL_RenderCopy(renderer, tex, NULL, &cover_pos);
    SDL_DestroyTexture(tex);
}

void AllocateStruct(struct info *buffer){
    //Allocating all necessary memory space for the info struct
    buffer->title = (struct text*) malloc(sizeof(struct text));
    buffer->artist = (struct text*) malloc(sizeof(struct text));
    buffer->album = (struct text*) malloc(sizeof(struct text));
    buffer->year = (struct text*) malloc(sizeof(struct text));
    return;
}

void FreeStruct(struct info *buffer){
    //Freeing all memory previously allocated for the info struct
    free(buffer->title);
    free(buffer->artist);
    free(buffer->album);
    free(buffer->year);
}
void TextDisplay(struct text *text, SDL_Renderer *renderer, SDL_Rect *titler){
    //Displaying whatever text we want based on whatever dimensions we give
    TTF_Font* font = TTF_OpenFont("assets/font.ttf", 64); // Loading up the font
    SDL_Color color = {255, 255, 255}; // setting up the color of the text

    //Declaring the surface variable
    SDL_Surface* surfaceMessage = NULL;

    //Rendering the text based on the encoding of the data.
    if(text->encoding == 0 || text->encoding == 3){
        surfaceMessage = TTF_RenderText_Blended(font, text->text, color);
    }

    if(text->encoding == 1 || text->encoding == 2){
        surfaceMessage = TTF_RenderUNICODE_Blended(font, text->text16, color);
    }

    //Testing if the text is rendered succesfully.
    if (surfaceMessage == NULL ){
	printf("Error creating surface : %s\n", SDL_GetError());
	exit(0);
    }

    //Creating the texture of the text to be displayed
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    if (Message == NULL ){
	printf("Error creating texture : %s\n", SDL_GetError());
	exit(0);
    }

    //Copying the Message data to the renderer and storing the functions output for bug testing.
    int err = SDL_RenderCopy(renderer, Message, NULL, titler);
    if (err <0){
	printf("Error copying to renderer : %s\n", SDL_GetError());
	exit(0);
    }

    //Presenting the Result and freeing all unused memory space
    SDL_RenderPresent(renderer);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
}


int main(int argc, char *argv[]){
    //Initializing the TTF library and declaring the rect for dimensions and positions to display
    //text (will be changed once I include responsive capabilities to the program).
    TTF_Init();
    SDL_Rect titler;
    titler.x = 150;
    titler.y = 325;
    titler.w = 200;
    titler.h = 50;
    SDL_Rect namer;
    namer.x = 800;
    namer.y = 200;
    namer.w = 75;
    namer.h = 30;

    //Declaring the info struct and allocating memory for it using the function declared before
    struct info track;
    AllocateStruct(&track);

    //Declaring the surface and structure variables
    SDL_Surface* bgsurf = NULL;
    SDL_Texture* bgtex = NULL;

    //Testing for errors
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

    //Gathering all needed data from the mp3 file and filling the track variable with said data.
    GetInfo(tag, &track);
    //Initializing video and audio, creating a window, and loading the background image..
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
    SDL_Window* win;
    SDL_Renderer *renderer = NULL;

    win = SDL_CreateWindow(
        "MusicKid",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        960,                               // width, in pixels
        540,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    bgsurf = IMG_Load("assets/titled.png");
    bgtex = SDL_CreateTextureFromSurface(renderer, bgsurf);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, bgtex, NULL, NULL);

    //Displaying the title and artist name and the album cover, then playing the music selected
    //by the user
    DisplayCover(tag, renderer);
    TextDisplay(track.title, renderer, &titler);
    TextDisplay(track.artist, renderer, &namer);
    SDL_RenderPresent(renderer);
    Player(argv[1]);
    //Freeing all unused memory before finishing the execution
    FreeStruct(&track);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

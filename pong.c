#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include <stdio.h>

/* constants */
/* screen dimensions */
static const int SCREEN_WIDTH = 1080;
static const int SCREEN_HEIGHT = 720;

/* global variables */
static unsigned int lScore, rScore;
static char scoreText[16];

static SDL_Window* gWindow;		/* the window we will be rendering to */
static SDL_Renderer* gRenderer;	/* the window renderer */
static TTF_Font* gFont;		/* global font */
			
/* type definitions */
typedef struct Entity Entity;
typedef struct LTexture LTexture;

/* function declarations */
static int init();			/* initializes libraries */
static int loadMedia();		/* loads textures, fonts, &c. */
static void moveEntity(Entity *en);	/* moves entity on rendered screen */
static void updateScore();		/* changes the score string to be rendered */
static void close();			/* frees memory from SDL libraries */

/* Texture functions */
static int LTexture_loadFromRenderedText(LTexture *tex, char* textureText, SDL_Color textColor); /* load texture from text using SDL_Font */
static void LTexture_free(LTexture *tex); /* free resources */
static void LTexture_render(LTexture *tex, int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip); /* render texture */

/* An Entity is anything on the screen that has its own physics */
typedef struct Entity
{
        SDL_Rect rect;
        int hasBounce;
        int xVel;
        int yVel;
};

/* Texture wrapper class */
/* source from lazyfoo.net */
typedef struct LTexture
{
	/* the actual hardware texture */
	SDL_Texture *mTexture;

	/* image dimensions */
	int mWidth;
	int mHeight;
};


/* function definitions */
int LTexture_loadFromRenderedText(LTexture *tex, char *textureText, SDL_Color textColor)
{
        /* get rid of preexisting texture */
        LTexture_free(tex);

        /* render text surface */
        SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText, textColor);
        if (textSurface == NULL) {
                fprintf(stderr, "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        }
        else {
                /* create texture from surface pixels */
                if ((tex->mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface)) == NULL) {
                        fprintf(stderr, "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
                }
                else {
                        /* get image dimensions */
                        tex->mWidth = textSurface->w;
                        tex->mHeight = textSurface->h;
                }

                /* get rid of old surface */
                SDL_FreeSurface(textSurface);
        }
        /* return success */
        return tex->mTexture != NULL;
}

void LTexture_free(LTexture *tex)
{
        /* free texture if it exists */
        if (tex->mTexture != NULL) {
                SDL_DestroyTexture(tex->mTexture);
                tex->mTexture = NULL;
                tex->mWidth = 0;
                tex->mHeight = 0;
        }
}

void LTexture_render(LTexture *tex, int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
        /* set rendering space and render to screen */
        SDL_Rect renderQuad = { x, y, tex->mWidth, tex->mHeight };

        /* set clip rendering dimensions */
        if (clip != NULL) {
                renderQuad.w = clip->w;
                renderQuad.h = clip->h;
        }

        /* render to screen */
        SDL_RenderCopyEx(gRenderer, tex->mTexture, clip, &renderQuad, angle, center, flip);
}

 static LTexture gScoreTexture; //the texture used to display the score





/* function definitions */
int init()
{
	/* initialize SDL */
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                fprintf(stderr, "SDL could not be initialized. SDL Error:%s\n", SDL_GetError());
                return 0;
        }
        if ((gWindow = SDL_CreateWindow("pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN)) == NULL) {
                fprintf(stderr, "Window could not be created. SDL Error:%s\n", SDL_GetError());
                return 0;
        }
        if ((gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == NULL) {
                fprintf(stderr, "Renderer could not be created. SDL Error:%s\n", SDL_GetError());
                return 0;
        }

        /* initialize SDL_image */
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
                fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                return 0;
        }

        /*initialize SDL_ttf */
        if (TTF_Init() == -1) {
                fprintf(stderr, "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
                return 0;
        }

	/* initialize scores */
        lScore = rScore = 0;

        return 1;
}

int loadMedia()
{
        /* laoding success flag */
        int success = 1;

        /* open the font */
        /* Copyright (c) 2011 by Sorkin Type Co (www.sorkintype.com), with Reserved Font Name "Basic". */
        if ((gFont = TTF_OpenFont("font/Basic-Regular.ttf", 64)) == NULL) { 
                fprintf(stderr, "Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
                success = 0;
        }

        return success;
}


void moveEntity(Entity *en)
{
        en->rect.x += en->xVel;
        if (en->rect.x > SCREEN_WIDTH || en->rect.x < 0 - en->rect.w) {
                en->rect.x -= en->xVel;
                if (en->hasBounce) {
                        en->xVel *= -1;
                }
        }

        en->rect.y += en->yVel;
        if (en->rect.y + en->rect.h > SCREEN_HEIGHT || en->rect.y < 0) {
                en->rect.y -= en->yVel;
                if (en->hasBounce) {
                        en->yVel *= -1;
                }
        }
}

void close()
{
        lScore = rScore = 0;

        LTexture_free(&gScoreTexture);

        TTF_CloseFont(gFont);
        gFont = NULL;

        SDL_DestroyRenderer(gRenderer);
        SDL_DestroyWindow(gWindow);
        gWindow = NULL;
        gRenderer = NULL;

        TTF_Quit();
        IMG_Quit();
        SDL_Quit();

}

void updateScore()
{
	if (sprintf(scoreText, "%u        %u", lScore, rScore) < 0)
		perror("score text failed to update");
}


/* main function */
int main()
{
        /* setup */
        int i;
        if ((i=init()) == 0) {
                perror("pong could not initialize\n");
                return -1;
        }
        if (loadMedia() == 0) {
                perror("failed to load media\n");
                return -1;
        }

	Entity puck = {
        	{ 530, 350, 20, 20 },
        	10,
        	10,
        	1
	};

	Entity lPaddle = {
        	{ 24, 270, 20, 90 },
        	0,
        	0,
        	0
	};

	Entity rPaddle = {
        	{ 1036, 270, 20, 90 },
        	0,
        	0,
        	0
	};

        SDL_Event e; /* event handling */
        SDL_Color textColor = { 0, 0, 0, 255 }; /* color for displayed score */
	
        /* main loop */
        int quit = 0;
        while (!quit) {
                /* move entities */
                moveEntity(&puck);
                moveEntity(&lPaddle);
                moveEntity(&rPaddle);
                /* key state array */
                const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
                /* handle events */
                while (SDL_PollEvent(&e) != 0) {
                        /* user requests quit */
                        if (e.type == SDL_QUIT) {
                                quit = 1;
                        }
                }

                /* keystate handling */
                if (currentKeyStates[SDL_SCANCODE_Q] && !currentKeyStates[SDL_SCANCODE_A])
                        lPaddle.yVel = -20;
                else if (currentKeyStates[SDL_SCANCODE_A])
                        lPaddle.yVel = 20;
                else
                        lPaddle.yVel = 0;

                if (currentKeyStates[SDL_SCANCODE_UP] && !currentKeyStates[SDL_SCANCODE_DOWN])
                        rPaddle.yVel = -20;
                else if (currentKeyStates[SDL_SCANCODE_DOWN])
                        rPaddle.yVel = 20;
                else
                        rPaddle.yVel = 0;

                /* puck collision with paddles */
                if (SDL_HasIntersection(&puck.rect, &lPaddle.rect)) {
                        puck.rect.x = lPaddle.rect.x + lPaddle.rect.w;
                        puck.xVel *= -1;
                }
                if (SDL_HasIntersection(&puck.rect, &rPaddle.rect)) {
                        puck.rect.x = rPaddle.rect.x - puck.rect.w;
                        puck.xVel *= -1;
                }

                /* score */
                if (puck.rect.x > rPaddle.rect.x + rPaddle.rect.w) {
                        lScore++;
                        puck.rect.x = 530;
                        puck.rect.y = 350;
                        printf("Left: %u, Right: %u\n", lScore, rScore);
                }
                if (puck.rect.x + puck.rect.w < lPaddle.rect.x) {
                        rScore++;
                        puck.rect.x = 530;
                        puck.rect.y = 350;
                        printf("Left: %u, Right: %u\n", lScore, rScore);
                }

                /* render score on screen */
		updateScore();
                if (!LTexture_loadFromRenderedText(&gScoreTexture, scoreText, textColor)) {
                        printf("Unable to render score texture.\n");
                }

                if (rScore >= 10) {
                        printf("Right Wins!\n");
                        quit = 1;
                }
                if (lScore >= 10) {
                        printf("Left Wins!\n");
                        quit = 1;
                }

                /* clear screen */
                if (SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF) < 0)
			perror(SDL_GetError());
                if (SDL_RenderClear(gRenderer) < 0)
			perror(SDL_GetError());

                /* render puck */
                if (SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF) < 0)
			perror(SDL_GetError());
                if (SDL_RenderFillRect(gRenderer, &(puck.rect)) < 0)
			perror(SDL_GetError());

                /* render left paddle */
                if (SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF) < 0)
			perror(SDL_GetError());
                if (SDL_RenderFillRect(gRenderer, &(lPaddle.rect)) < 0)
			perror(SDL_GetError());

                /* render right paddle */
                if (SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF) < 0)
			perror(SDL_GetError());
                if (SDL_RenderFillRect(gRenderer, &(rPaddle.rect)) < 0)
			perror(SDL_GetError());

                /* render score */
                LTexture_render(&gScoreTexture, (SCREEN_WIDTH - gScoreTexture.mWidth) / 2, 24, NULL, 0.0, NULL, SDL_FLIP_NONE);

                /* update screen */
                SDL_RenderPresent(gRenderer);
        }


        close();

        return 0;
}


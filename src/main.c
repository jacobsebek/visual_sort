/*TODO: error hanfling*/

#include "SDL.h"
#include "SDL_image.h"

#include <time.h>
#include <string.h>
#include <stdbool.h>

#include <stdio.h>
#include <math.h>

#include "sorts.h"

uint WIDTH = 1000, HEIGHT = 500;
bool quit = 0, // The program should be terminated without any exceptions
	 pause = 0, // The program should repeatedly draw the current frame, with the exception of quit
	 reset = 0, // The program should quit all recursive functions and loops and get back to the entry
	 norec = 1; // The program should NOt RECursively call functions (generate and sortcheck for example)

bool mode = 1; // ASCENDING (1) or DESCENDING (0);
bool rainbow = 0, 
	 style = 0,
	 extreme = 0; //Rendering flags : 1 or 0 (HSV gradient option); 0 - 3D style, 1 - flat style; no visual effects for maximum performance

int seed = 0; // If not zero, the values will get initialized with the same seed every time

static int press_arrow = 0; // If positive, indicates that the right triangle should be "pressed", if negative, same for the left one
static uint delay = 10;

SDL_Renderer* ren;
SDL_Window* win;

int* vals = NULL;
uint valc = 200; // DO NOT CHANGE

const char *names[] =  {"No. 1 : Selection Sort",
						"No. 2 : Bubble Sort",
						"No. 3 : Quick-Sort",
						"No. 4 : Coctail Shaker Sort",
						"No. 5 : Heap Sort",
						"No. 6 : Merge Sort",
						"No. 7 : Insertion Sort",
						"No. 8 : Introsort (std::sort) ",
						"No. 9 : Shellsort (original)",
						"No. 10 : LSD Radix sort (Base 10)"
						};

const char* credits = "Made by github.com:jacobsebek with SDL and C under the MIT license -- Fonts - Sans, Monogram";
				
void const(*sorts[])() = {	sort_selection,
							sort_bubble, 
							sort_quick,
							sort_coctail, 
							sort_heap, 
							sort_merge,
							sort_insertion,
							sort_intro,
							sort_shell,
							sort_radix};
int sort = 0;

static SDL_Color priv_HSVtoSDL_Color(double, double, double);

static SDL_Texture* priv_load_tex(const char*);
static SDL_Surface* priv_load_surf(const char*);
static SDL_Surface* priv_load_surf(const char*);
static SDL_Texture* priv_draw_text(const char*, SDL_Color);

void step();
int drawpdate(column_range* marked, uint count);

int init_sdl();
void init_vals();

static SDL_Color priv_HSVtoSDL_Color(double hue, double s, double v)
{
	if (s > 1 || v > 1 || s < 0 || v < 0) return (SDL_Color){0,0,0};

	hue = fmod(hue, 360);
	hue /= 60.0;
	register double c = v*s;
	register double x = c * (1 - fabs(fmod(hue, 2.0)-1));
	register double m = v-c;

	register struct fSDL_Color {
		float r,g,b;	
	} fcol = {0,0,0};

	switch ((uint)hue) {
		case 0 : fcol = (struct fSDL_Color){c,x,0}; break;
		case 1 : fcol = (struct fSDL_Color){x,c,0}; break;
		case 2 : fcol = (struct fSDL_Color){0,c,x}; break;
		case 3 : fcol = (struct fSDL_Color){0,x,c}; break;
		case 4 : fcol = (struct fSDL_Color){x,0,c}; break;
		case 5 : fcol = (struct fSDL_Color){c,0,x}; break;
	}

	register SDL_Color col;
	col.r = (uint) ((fcol.r+m)*255.0);
	col.g = (uint) ((fcol.g+m)*255.0);
	col.b = (uint) ((fcol.b+m)*255.0);

	return col;
}

static inline int clamp(register int val, register int bot, register int top)
{
	return val = (val > top ? top : (val < bot ? bot : val));
}

static SDL_Texture* priv_load_tex(const char* name)
{
	SDL_Surface *surf = IMG_Load(name);
	SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);

	SDL_FreeSurface(surf);
	surf = NULL;

	if (tex == NULL)
		quit = 1;

	return tex;
}

static SDL_Surface* priv_load_surf(const char* name)
{
	SDL_Surface *surf = IMG_Load(name);

	if (surf == NULL)
		quit = 1;

	return surf;
}

static SDL_Texture* priv_draw_text(const char* str, SDL_Color col) // Draws text on a texture in specified color with transparent background
{
	static uint charcount = 68; // Number of characters on the font image
	static uint charwidth;
	static SDL_Color lastcol = {0,0,0}; // Used to determine if the font needs to be repainted with a new color

	static SDL_Surface* font = NULL;
	static SDL_Texture *fontex = NULL;

	if (font == NULL) {
		font = priv_load_surf("./res/monogram.png");
		if (font == NULL) {
			quit = 1;
			return NULL;
		}

		charwidth = round(font->w/(double)charcount);
	}
	if (strlen(str) == 0) {
		SDL_FreeSurface(font);
		font = NULL;

		SDL_DestroyTexture(fontex);
		fontex = NULL;

		return NULL;
	}

	//Color the font
	if (lastcol.r != col.r || 
		lastcol.g != col.g || 
		lastcol.b != col.b ||
		fontex == NULL) {

		SDL_LockSurface(font);
		
		register Uint32 *intpixels = (Uint32*)font->pixels;
		for (register uint i = 0; i < (font->w*font->h); i++) {
			Uint8 a, r, g, b;
			SDL_GetRGBA(intpixels[i], font->format, &r, &g, &b, &a);
			intpixels[i] = SDL_MapRGBA(font->format, col.r, col.g, col.b, a);
		}

		SDL_UnlockSurface(font);

		if (fontex != NULL)
			SDL_DestroyTexture(fontex);
		fontex = SDL_CreateTextureFromSurface(ren, font);
	}

	SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, strlen(str)*charwidth, font->h);

	if (fontex == NULL || tex == NULL) {
		quit = 1;
		return NULL;
	}

	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(ren, tex);

	SDL_SetRenderDrawColor(ren,255,255,255,0);
	SDL_RenderClear(ren);
	
	for (register uint i = 0; *str; *str++, i++) {
		register int index;

		if (*str >= 'a' && *str <= 'z') index = *str-'a'+11;
		else if (*str >= 'A' && *str <= 'Z') index = *str-'A'+11+26;
		else if (*str >= '0' && *str <= '9') index = *str-'0'+1;
		else switch (*str) {
			case '.' : 
			case ',' : index = 63; break;
			case ':' : index = 64; break;
			case '-' : index = 65; break;
			case '(' : index = 66; break;
			case ')' : index = 67; break;
			default :  index = 0;  break;
		}

		SDL_RenderCopy(ren, fontex, &(SDL_Rect){index*charwidth, 0, charwidth, font->h}, &(SDL_Rect){i*charwidth, 0, charwidth, font->h});
	}

	SDL_SetRenderTarget(ren, NULL);

	lastcol = col;

	return tex;
}

static inline void priv_gradient_vertical(SDL_Rect* dst, SDL_Color col1, SDL_Color col2, uint sample_height) // Can draw vertical gradients with flipped y size (upwards)
{
	register bool flipped = (dst->h < 0);
	register double grad;
	register int y;
	for (register int i = 0; i < (flipped ? -dst->h : dst->h); i+=sample_height) {
		grad = (i == 0) ? 0 : ((double)i/((flipped ? -dst->h : dst->h)+1));
		SDL_SetRenderDrawColor(ren, (col1.r+(col2.r-col1.r)*grad),
									(col1.g+(col2.g-col1.g)*grad),
									(col1.b+(col2.b-col1.b)*grad),
									(col1.a+(col2.a-col1.a)*grad));

		y = dst->y+(flipped ? -i-sample_height : i);
		SDL_RenderFillRect(ren, &(SDL_Rect){dst->x, y, dst->w, sample_height});
	}

}

void step()
{
	static bool canpress = 1;
	static SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch(e.type) {
			case SDL_QUIT :
				quit = 1; // Flag for the main loop to end the program
			break;
			case SDL_KEYDOWN :
				if (!canpress) break;
				canpress = 0;

				switch (e.key.keysym.sym) {
					case SDLK_LEFT :
						sort = (sort-1 < 0 ? arrlen(sorts)-1 : sort-1);
						press_arrow = -SDL_GetTicks();
						reset = 1;
					break;
					case SDLK_RIGHT :
						sort = (sort+1 >= arrlen(sorts) ? 0 : sort+1);
						press_arrow = SDL_GetTicks();
						reset = 1;
					break;
					case SDLK_SPACE : // Play / Pause 
						pause = !pause;
					break;
					case SDLK_g : // Generate
						if (norec) break;

						pause = 0;

						norec = 1;
						init_vals();
						norec = 0;

						reset = 1;
					break;
					case SDLK_m : // Generate
						mode = !mode;
						reset = 1;
					break;
				}

			break;
			case SDL_KEYUP :
				canpress = 1;
			break;
		}
	}
}

int drawpdate(column_range* marked, uint count)
{
	//SDL_GetWindowSize(win, &WIDTH, &HEIGHT); // If window is resized

	static const uint bounds_h = 100,//Horizontal bounds
					  bounds_top = 40,
					  bounds_bot = 150; //Top bound	`

	register uint colw = (uint) ( (float)(WIDTH-bounds_h*2)/valc );
	if (colw == 0) colw = 1;

	register uint colh;
	SDL_Rect col, gradient;
	register SDL_Color color;
	register int i;
	register uint j;
	register uint max;

	register const uint coltexw = colw*valc,
						coltexh = HEIGHT-bounds_top-bounds_bot;

	static SDL_Texture *coltex = NULL;
	if (coltex == NULL) {
		coltex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, coltexw, coltexh);
		SDL_SetTextureBlendMode(coltex, SDL_BLENDMODE_BLEND);

		if (coltex == NULL) {
			quit = 1;
			return STEP_ABORT;
		}
	}

	static SDL_Texture *guides[4] = { NULL };
	if (guides[0] == NULL) {
		guides[0] = priv_load_tex("./res/pause.png");
		guides[1] = priv_load_tex("./res/order.png");
		guides[2] = priv_load_tex("./res/select.png");
		guides[3] = priv_load_tex("./res/generate.png");
	}

	static SDL_Texture *nametex[arrlen(sorts)] = { NULL };
	if (nametex[0] == NULL) {
		for (i = 0; i < arrlen(nametex); i++)
			nametex[i] = priv_draw_text(names[i], (SDL_Color){255,255,255});
	}

	static SDL_Texture *creditex = NULL;
	if (creditex == NULL)
		creditex = priv_draw_text(credits, (SDL_Color){150,150,150});

	static SDL_Texture *triangle = NULL;
	if (triangle == NULL)
		triangle = priv_load_tex("./res/triangle.png");

	do {
		if (step(), quit) {

			SDL_DestroyTexture(coltex);
			coltex = NULL;

			SDL_DestroyTexture(triangle);
			triangle = NULL;

			SDL_DestroyTexture(creditex);
			creditex = NULL;

			for (i = 0; i < arrlen(guides); i++) {
				SDL_DestroyTexture(guides[i]);
				guides[i] = NULL;
			}

			for (i = 0; i < arrlen(nametex); i++) {
				SDL_DestroyTexture(nametex[i]);
				nametex[i] = NULL;
			}
			priv_draw_text("", (SDL_Color){0,0,0}); // Empty string to clear the font cache
			
			return STEP_ABORT;
		}

		SDL_SetRenderDrawColor(ren, 0,0,0,0);
		SDL_RenderClear(ren);

		if (!extreme) {
			priv_gradient_vertical(&(SDL_Rect){bounds_h, HEIGHT-bounds_bot, WIDTH-bounds_h*2, -150}, 
									(SDL_Color){255,255,255,255},
									(SDL_Color){0,0,0,0}, 5);
		}

		SDL_SetRenderTarget(ren, coltex);

		SDL_SetRenderDrawColor(ren, 0,0,0,0);
		SDL_RenderClear(ren);

		for (i = 0; i < valc; i++) {
			if (vals[i] <= 0) continue;

			colh = (uint) ( vals[i]*((float)(coltexh)/valc) );
			
			col = (SDL_Rect){ i*(colw) , coltexh-colh, colw, colh};
			
			if (rainbow)
				color = priv_HSVtoSDL_Color((360.0/valc)*vals[i], 1.0, 1.0);
			else {
				color = (SDL_Color){ 255, 255, 255};
				for (j = 0; j < count; j++)
					if (i >= marked[j].l && i <= marked[j].r) {
						color = marked[j].color;
						break;
					}
			}


			SDL_SetRenderDrawColor(ren, color.r,
										color.g,
										color.b, 
										255);
			
			SDL_RenderFillRect(ren, &col);

			if (!extreme) {
				priv_gradient_vertical(&(SDL_Rect){i*(colw), coltexh-colh, colw, -10},
										(SDL_Color){color.r, color.g, color.b,255},
										(SDL_Color){0,0,0,0}, 1);

				priv_gradient_vertical(&(SDL_Rect){i*(colw), coltexh-colh+colw, colw-(style ? 0 : colw/4.0), 200},
										(SDL_Color){0,0,0,255},
										(SDL_Color){0,0,0,0}, 5);
			}

		}
		SDL_SetRenderTarget(ren, NULL);

		SDL_RenderCopy(ren, coltex, NULL, &(SDL_Rect){bounds_h, bounds_top, WIDTH-bounds_h*2, HEIGHT-bounds_bot-bounds_top});

		SDL_SetRenderDrawColor(ren, 255,255,255,255);
		SDL_RenderFillRect(ren, &(SDL_Rect){bounds_h, HEIGHT-bounds_bot, WIDTH-bounds_h*2, 3});

		// GUIDES

		for (i = 0; i < arrlen(guides); i++)
			SDL_RenderCopy(ren, guides[i], NULL, &(SDL_Rect){WIDTH/2-((150+2)/2*arrlen(guides))+i*(150+20), HEIGHT-60, 150, 40});

		// ARROWS

		if (press_arrow != 0 && 
		   SDL_GetTicks()-abs(press_arrow) >= 100) press_arrow = 0;

		register int triangle_scale = 40/(press_arrow < 0 ? 2 : 1);
		SDL_RenderCopy(ren, triangle, NULL, &(SDL_Rect){bounds_h, HEIGHT-bounds_bot+40-triangle_scale/2, triangle_scale, triangle_scale});

		triangle_scale = 40/(press_arrow > 0 ? 2 : 1);
		SDL_RenderCopyEx(ren, triangle, NULL, &(SDL_Rect){WIDTH-bounds_h-triangle_scale, HEIGHT-bounds_bot+40-triangle_scale/2, triangle_scale, triangle_scale}, 0, NULL, SDL_FLIP_HORIZONTAL);

		// SORT NAME
		SDL_RenderCopy(ren, nametex[sort], NULL, &(SDL_Rect){WIDTH/2-strlen(names[sort])*5*4/2, HEIGHT-bounds_bot+60-75/2, strlen(names[sort])*5*4, 8*4});
		// CREDITS
		SDL_RenderCopy(ren, creditex, NULL, &(SDL_Rect){5, 5, (uint)(strlen(credits)*5*1.5), (uint)(8*1.5)});

		SDL_RenderPresent(ren);

		if (delay)
			SDL_Delay(delay);
	} while (pause);
	if (reset) return STEP_ABORT; // quit is kinda unnecessary

	return STEP_CONTINUE;
}

int init_sdl()
{

	//------------------SDL------------------
	if (SDL_Init(SDL_INIT_VIDEO) != 0){
		printf("SDL_Init Error: %s", SDL_GetError());
		return 1;
	}

	win = SDL_CreateWindow("Sorting algorithm visualisation (SDL2)", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	if (win == NULL){
		printf("SDL_CreateWindow Error: %s", SDL_GetError());
 		SDL_Quit();
		return 1;
	}

	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (ren == NULL){
		SDL_DestroyWindow(win);
		printf("SDL_CreateRenderer Error: %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

	return 0;
}

void init_vals()
{
	if (vals == NULL) {
		vals = calloc(valc, sizeof(uint));

		if (vals == NULL) {
			quit = 1;
			return;
		}
	}
	
	/*int j = 11, k = 12;
	for (int i = valc-1; i >= 0; i--) {
		vals[i] = (i < valc/2) ? (j-=2) : (k-=2);
	}*/

	if (seed != 0)
		srand(seed);

	int src[valc];
	for (uint i = 0; i < valc; i++) src[i] = i+1;

	int sum = 0;
	for (uint i = 0; i < valc; i++) {
		drawpdate(NULL, 0);
		if (quit) return; // Cannot check for abort because has to ignore resets

	    uint index;
	    while (src[index=(rand()%valc)] == 0);
	    vals[i] = src[index];
	    src[index] = 0;

		sum +=vals[i];
	}
	printf("generation complete, checksum : %d\n", sum);

	/*for (uint i = 0; i < valc; i++)
		vals[i]=(rand()%valc);*/

	/*for (uint i = 0; i < valc; i++) printf("%d, ", vals[i]);
	putchar('\n');*/

	pause = 1;
}

int main(int argc, char** argv)
{
	if ((argc-1) % 2 != 0) {
		printf("Argument count must be odd! (Every option shall have the parameter after it)\n");
		return 1;
	}
	for (int i = 1; i < argc;) {
		if (argv[i][0] == '-') {

			switch (argv[i][1]) {
				case 'c' : valc = atoi(argv[i+1]); break; // c - Count
				case 'a' : sort = atoi(argv[i+1]); break; // a - algorithm
				case 'g' : rainbow = atoi(argv[i+1]); break; // g - Gradient
				case 'm' : mode = atoi(argv[i+1]); break; // m - Mode
				case 's' : style = atoi(argv[i+1]); break; // s - style
				case 'w' : WIDTH = atoi(argv[i+1]); HEIGHT = WIDTH/2; break; // w - window
				case 'd' : delay = atoi(argv[i+1]); break; // d - delay
				case 'x' : extreme = atoi(argv[i+1]); break; // x - eXtreme
				case 'r' : seed = atoi(argv[i+1]); break; // r - Random
				default: printf("Invalid command line option : %s\n", argv[i]);
			}
			i+=2;
		} else return 1;
	}

	{
		int INIT;
		if ( INIT = init_sdl() ) return INIT;
	}

	init_vals();

	while (!quit) {
		reset = 0;
		if (drawpdate(NULL, 0) == STEP_ABORT) continue;	// Do not continue until there is abort

		long long start = SDL_GetTicks();

		printf("starting %s.\n", names[sort]);
		sorts[sort]();

		if (!reset) {
			printf("This sort took %.3f seconds.\n", (SDL_GetTicks() - start)/1000.0);
			sort_check();
			pause = 1;
		}

	}

	free(vals);

	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	vals = NULL, win = NULL, ren = NULL;

	return 0;

}

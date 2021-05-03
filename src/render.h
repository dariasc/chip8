typedef unsigned char uint8;

void sdl_init();
void sdl_destroy();

void sdl_clear();
int sdl_draw(int x, int y, uint8 sprite[], uint8 size);
void sdl_paint();


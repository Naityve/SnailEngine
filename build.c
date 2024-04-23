#include <stdlib.h>

int main() {
    system("gcc -o Aspha.exe main.c ./src/*.c -I./raylib/src -L./raylib/src -lraylib -lm -lopengl32 -lgdi32 -lwinmm");
    return 0;
}

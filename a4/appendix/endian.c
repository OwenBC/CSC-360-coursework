#include <stdio.h>
#include <stdlib.h>

int main() {
    short ii = 127;
    int   i  = 127;

    FILE *f = fopen("somefile.bin", "w");
    if (f == NULL) { exit(1); }
    fwrite(&ii, sizeof(short), 1, f);
    fwrite(&i, sizeof(int), 1, f);
    fclose(f);
}

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <getopt.h>
#define PNG_DEBUG 3
#include <png.h>

struct Png{
    int width, height;
    png_byte color_type;
    png_byte bit_depth;

    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep *row_pointers;
};

void print_PNG_info(struct Png *image){
    printf("Ширина изображения: %d\n", image->width);
    printf("Высота изображения: %d\n", image->height);
    printf("Тип цвета: %u\n", image->color_type);
    printf("Глубина цвета: %u\n", image->bit_depth);
}

void print_info(){
//    printf("\033[1;34m__Text__:\033[0m\n");
//    "\n\033[4;31mВыберите команду:\033[0m\n"
//    "\n\033[1;31mВыберите команду:\033[0m\n"
    printf("Это программа с CLI для редактирования png-изображений <з\n");
    printf("Поддерживаются файлы с глубиной цвета RGBa\n");
    printf("Формат ввода: ./pngedit [filename.png] -[o]/--[option] [arguments] [filename.png]\n");


    printf("[имя файла] -q/--square - нарисовать квадрат\n");
    printf("    [x-координата] [y-координата] - левый верхний угол\n");
    printf("    [число] - длина стороны\n");
    printf("    [число] - толщина линий (в пикселях)\n");
    printf("    [R] [G] [B] [A] - числа, цвет заливки и линий\n");
    printf("    [число] - заливка (по умолчанию без заливки) (1 - заливка, 0 - нет)\n");

    printf("[имя файла] -w/--swap - поменять местами 4 куска области\n");
    printf("    [x-координата] [y-координата] - левый верхний угол\n");
    printf("    [x-координата] [y-координата] - правый нижний угол\n");
    printf("    [число] - способ (1 - по кругу, 2 - по диагонали)\n");

    printf("[имя файла] -f/--frequent - заменить самый часто встречающийся цвет на новый\n");
    printf("    [R] [G] [B] [A] - числа, новый цвет (RGBa)\n");

    printf("[имя файла] -n/--inversion - инвертировать цвет в заданной области\n");
    printf("    [x-координата] [y-координата] - левый верхний угол\n");
    printf("    [x-координата] [y-координата] - правый нижний угол\n");

    printf("-i - получить информацию об изображении\n");
    printf("-h - вызвать справку\n");

    printf("[имя нового файла] - файл для вывода\n");
}

void read_png_file(char *file_name, struct Png *image) {
    int x,y;
    char header[8];    // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    if (!fp){
        // Some error handling: file could not be opened
    }

    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)){
        // Some error handling: file is not recognized as a PNG
    }

    /* initialize stuff */
    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!image->png_ptr){
        // Some error handling: png_create_read_struct failed
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        // Some error handling: png_create_info_struct failed
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        // Some error handling: error during init_io
    }

    png_init_io(image->png_ptr, fp);
    png_set_sig_bytes(image->png_ptr, 8);

    png_read_info(image->png_ptr, image->info_ptr);

    image->width = png_get_image_width(image->png_ptr, image->info_ptr);
    image->height = png_get_image_height(image->png_ptr, image->info_ptr);
    image->color_type = png_get_color_type(image->png_ptr, image->info_ptr);
    image->bit_depth = png_get_bit_depth(image->png_ptr, image->info_ptr);

    image->number_of_passes = png_set_interlace_handling(image->png_ptr);
    png_read_update_info(image->png_ptr, image->info_ptr);

    /* read file */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        // Some error handling: error during read_image
    }

    image->row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * image->height);
    for (y = 0; y < image->height; y++)
        image->row_pointers[y] = (png_byte *) malloc(png_get_rowbytes(image->png_ptr, image->info_ptr));

    png_read_image(image->png_ptr, image->row_pointers);

    fclose(fp);
}

void write_png_file(char *file_name, struct Png *image) {
    int x,y;
    /* create file */
    FILE *fp = fopen(file_name, "wb");
    if (!fp){
        // Some error handling: file could not be opened
    }

    /* initialize stuff */
    image->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!image->png_ptr){
        // Some error handling: png_create_write_struct failed
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        // Some error handling: png_create_info_struct failed
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        // Some error handling: error during init_io
    }

    png_init_io(image->png_ptr, fp);


    /* write header */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        // Some error handling: error during writing header
    }

    png_set_IHDR(image->png_ptr, image->info_ptr, image->width, image->height,
                 image->bit_depth, image->color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(image->png_ptr, image->info_ptr);


    /* write bytes */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        // Some error handling: error during writing bytes
    }

    png_write_image(image->png_ptr, image->row_pointers);


    /* end write */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        // Some error handling: error during end of write
    }

    png_write_end(image->png_ptr, NULL);

    /* cleanup heap allocation */
    for (y = 0; y < image->height; y++)
        free(image->row_pointers[y]);
    free(image->row_pointers);

    fclose(fp);
}

void process_file(struct Png *image) {
    int x,y;
    if (png_get_color_type(image->png_ptr, image->info_ptr) == PNG_COLOR_TYPE_RGB){
        // Some error handling: input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA
    }

    if (png_get_color_type(image->png_ptr, image->info_ptr) != PNG_COLOR_TYPE_RGBA){
        // Some error handling: color_type of input file must be PNG_COLOR_TYPE_RGBA
    }

    for (y = 0; y < image->height; y++) {
        png_byte *row = image->row_pointers[y];
        for (x = 0; x < image->width; x++) {
            png_byte *ptr = &(row[x * 4]);
//            printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n",
//                   x, y, ptr[0], ptr[1], ptr[2], ptr[3]);

            /* set red value to 0 and green value to the blue one */
            ptr[0] = 0;
            ptr[1] = ptr[2];
        }
    }
}

//drawSquare();
void drawSquare(struct Png *image, int x, int y, int l, int t, int * color, int fill, int * colorF) {
    if (x < 0 || y < 0 || l < 0 || t < 0)  {
        printf("Недопустимые параметры:\n "
               "координаты, размер квадрата и ширина контура не могут "
               "иметь отрицательные значения\n");
        return;
    }
    if (x > image->width || y > image->height) {
        printf("Недопустимые параметры:\n "
               "координаты не могут лежать за пределами изображения\n");
        return;
    }
    if ((x + l) >= image->width || (y + l) >= image->height) {
        printf("Квадрат не может заходить за пределы изображения\n");
        return;
    }
    if (color[0] > 255 || color[0] < 0 || color[1] > 255 || color[1] < 0 || color[2] > 255 || color[2] < 0 || color[3] > 255 || color[3] < 0) {
        printf("Некорректно заданные диапазоны.\n");
        return;
    }
//    if (x < 0 || y < 0 || x + l > image->width || y + l > image->height) {
//        // Квадрат не влезает в изображение, ничего не делаем
//        return;
//    }
    // Вычисляем координаты углов квадрата
    int x1 = x;
    int y1 = y;
    int x2 = x + l - 1;
    int y2 = y + l - 1;

    if (fill) {
    // Заливаем квадрат выбранным цветом
        for (int i = x1 + 1; i <= x2 - 1; i++) {
            for (int j = y1 + 1; j <= y2 - 1; j++) {
                image->row_pointers[i][j * 4 + 0] = colorF[0];
                image->row_pointers[i][j * 4 + 1] = colorF[1];
                image->row_pointers[i][j * 4 + 2] = colorF[2];
                image->row_pointers[i][j * 4 + 3] = colorF[3];
            }
        }
    }

    for (int i = x1; i <= x2; i++) {
        for (int j = y1; j <= y2; j++) {
            if ((j >= y1 && j <= y1+t) || (j <= y2 && j >= y2-t)
            || (i >= x1 && i <= x1+t)  || (i <= x2 && i >= x2-t)) {

                image->row_pointers[i][j * 4 + 0] = color[0];
                image->row_pointers[i][j * 4 + 1] = color[1];
                image->row_pointers[i][j * 4 + 2] = color[2];
                image->row_pointers[i][j * 4 + 3] = color[3];
            }
        }
    }

}


//swapAreas();
//changeColor();
//invertColors();

int main(int argc, char **argv) {
//    if (argc != 3){
//        fprintf(stderr,"Usage: program_name <file_in> <file_out>\n");
//        return 0;
//    }
    struct Png image;

    char* output = argv[1];
    read_png_file(argv[1], &image);
//    process_file(&image);
//    write_png_file(argv[2], &image);
//    char *output = argv[2];
    struct option longOpts[] = {
            {"square", required_argument, NULL, 'q'},
            {"swap", required_argument, NULL, 'w'},
            {"often", required_argument, NULL, 'o'},
            {"inversion", required_argument, NULL, 'n'},
//
    //            {"start", required_argument, NULL, 's'},
    //            {"end", required_argument, NULL, 'e'},
    //            {"length", required_argument, NULL, 'l'},
    //            {"thickness", required_argument, NULL, 't'},
    //            {"color", required_argument, NULL, 'c'},
    //            {"fill", required_argument, NULL, 'f'},
    //            {"round", no_argument, NULL, 'r'},
    //            {"diagonal", no_argument, NULL, 'd'},
//
            {"info", no_argument, NULL, 'i'},
            {"help", no_argument, NULL, 'h'},
            {NULL, 0, NULL, 0},
    };
    int opt;
    int longIndex;
//    char *opts = "q:w:o:n:s:e:l:t:c:f:r:d:i:h:";
    char *opts = "q:w:o:n:ih";
    opt = getopt_long(argc, argv, opts, longOpts, &longIndex);

//
//    switch (opt) {
//        case 'q':
//            while ((opt = getopt_long(argc, argv, opts, longOpts, &longIndex)) != -1) {
//                switch (opt) {
//                    case
//                }
//            }
//            drawSquare();
//            write_png_file();
//            break;
//        case 'w':
//
//            swapAreas();
//            write_png_file();
//            break;
//        case 'f':
//            changeColor();
//            write_png_file();
//            break;
//        case 'n':
//            invertColors();
//            write_png_file();
//            break;
//        case 'i':
//            print_PNG_info();
//            break;
//        case 'h':
//            print_info();
//            break;
//    }

    int y, x, length, thickness, fill;
    int color[4];
    int color_fill[4];

        while (opt != -1) {
            switch (opt) {
                case 'q':
                    x = atoi(argv[3]);
                    y = atoi(argv[4]);
                    length = atoi(argv[5]);
                    thickness = atoi(argv[6]);
                    color[0] = atoi(argv[7]);
                    color[1] = atoi(argv[8]);
                    color[2] = atoi(argv[9]);
                    color[3] = atoi(argv[10]);
                    fill = atoi(argv[11]);
                    if (fill) {
                        color_fill[0] = atoi(argv[12]);
                        color_fill[1] = atoi(argv[13]);
                        color_fill[2] = atoi(argv[14]);
                        color_fill[3] = atoi(argv[15]);
                        drawSquare(&image, x, y, length, thickness, color, fill, color_fill);
                        write_png_file(argv[16], &image);
                    } else {
                        drawSquare(&image, x, y, length, thickness, color, fill, NULL);
                        write_png_file(argv[12], &image);
                    }

//                    color = {atoi(argv[7]), atoi(argv[8]), atoi(argv[9]), atoi(argv[10])};
//                    fill = atoi(argv[11]);
//                    if (fill) {
//
//                    }
//                    drawSquare(&image, x, y, length, thickness, color, fill, color_fill);
//                    drawSquare(&image, x, y, length, thickness, color[0], color[1], color[2], color[3], fill, color[0], color[1], color[2], color[3]);
//                    write_png_file(argv[16], &image);
//                    printf("%d %d %d %d %d %d %d %d %d\n", x, y, length, thickness, color[0], color[1], color[2], color[3], fill);
//                    printf("%d\n", atoi(argv[3]));
                    break;
                case 'w':
//                    swapAreas();
//                    write_png_file();
                    break;
                case 'o':
//                    changeColor();
//                    write_png_file();
                    break;
                case 'n':
//                    invertColors();
//                    write_png_file();
                    break;
                case 'i':
//                    print_PNG_info();
                    break;
                case 'h':
                    print_info();
                    break;
            }
            opt = getopt_long(argc, argv, opts, longOpts, &longIndex);
        }

    return 0;
}
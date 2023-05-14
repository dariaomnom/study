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
    printf("Поддерживаются файлы с глубиной цвета RGBa 8 бит\n");
    printf("Формат ввода: ./pngedit [filename.png] -[o]/--[option] [arguments] [filename.png]\n");


    printf("[имя файла] -q/--square - нарисовать квадрат\n");
    printf("    [x-координата] [y-координата] - левый верхний угол\n");
    printf("    [число] - длина стороны\n");
    printf("    [число] - толщина линий (в пикселях)\n");
    printf("    [R] [G] [B] [A] - числа от 0 до 255, цвет линий\n");
    printf("    [число] - заливка (по умолчанию без заливки) (1 - заливка, 0 - нет)\n");
    printf("    [R] [G] [B] [A] - числа от 0 до 255, цвет заливки\n");

    printf("[имя файла] -w/--swap - поменять местами 4 куска области\n");
    printf("    [x-координата] [y-координата] - левый верхний угол\n");
    printf("    [x-координата] [y-координата] - правый нижний угол\n");
    printf("    [число] - способ (1 - по кругу, 2 - по диагонали)\n");

    printf("[имя файла] -o/--often - заменить самый часто встречающийся цвет на новый\n");
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

    // from GPT
//    if (bit_depth == 16) {
//        png_set_strip_16(png_ptr);
//    }
//
//    if (color_type == PNG_COLOR_TYPE_PALETTE) {
//        png_set_palette_to_rgb(png_ptr);
//    }
//
//    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
//        png_set_expand_gray_1_2_4_to_8(png_ptr);
//    }
    // from GPT


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
void drawSquare(struct Png * image, int x, int y, int l, int t, int * color, int fill, int * colorF) {
    if (x < 0 || y < 0 || l < 0 || t < 0)  {
        printf("Введены некорретные данные: \n "
               "координаты, длина стороны квадрата и ширина линий "
               "не могут иметь отрицательные значения\n");
        return;
    }
    if (x < 0 ||  x >= image->width || y < 0 || y >= image->height) {
        printf("Введены некорретные данные: координаты должны "
               "находиться в пределах изображения и быть не меньше 0\n");
        return;
    }
    if ((x + l) >= image->width || (y + l) >= image->height) {
        printf("Введены некорретные данные: квадрат не может "
               "выходить за пределы изображения\n");
        return;
    }
    if (color[0] > 255 || color[0] < 0 || color[1] > 255 || color[1] < 0
    || color[2] > 255 || color[2] < 0 || color[3] > 255 || color[3] < 0) {
        printf("Введены некорретные данные: цвета должны лежать от 0 до 255\n");
        return;
    }
    // Вычисляем координаты углов квадрата
    int x1 = x;
    int y1 = y;
    int x2 = x + l - 1;
    int y2 = y + l - 1;

    int number_of_channels = 4;
    int bit_depth = image->bit_depth;
    int stride = number_of_channels * bit_depth / 8;

    for (int i = x1; i <= x2; i++) {
        for (int j = y1; j <= y2; j++) {
            for (int c = 0; c < 4; c++) {
                if ((j >= y1 && j <= y1+t) || (j <= y2 && j >= y2-t)
                    || (i >= x1 && i <= x1+t)  || (i <= x2 && i >= x2-t)) {
                    image->row_pointers[j][i * stride + c] = color[c];
                } else if (fill && (i >= x1+t && i <= x2-t) && (j >= y1+t && j <= y2-t)) {
                    image->row_pointers[j][i * stride + c] = colorF[c];
                }
            }
        }
    }

}


//swapAreas();
//changeColor();
void changeColor(struct Png * image, int * new_color) {
////    int cnt = 0;
////    fot (int i = 0; i < image->width; i++) {
////        fot (int j = 0; j < image->height; j++) {
////
////        }
////    }
////    int cnt_rgba[MAX_COLOR] = {0};
//    int size = 200;
////    printf("%s, %d\n", __func__, __LINE__);
//    int ** colors = calloc(size, sizeof(int*));
//
//    int cnt_rgba = 0;
//    int * max_cnt = calloc(size, sizeof(int));
//    int R = 0; int G = 0; int B = 0; int A = 0;
////    printf("%s, %d\n", __func__, __LINE__);
//    int number_of_channels = 4;
//    int bit_depth = image->bit_depth;
//    int stride = number_of_channels * bit_depth / 8;
////    printf("%s, %d\n", __func__, __LINE__);
//    for (int x = 0; x < image->width; x++) {
////        png_byte* row = row_pointers[j];
////        colors[i] = malloc(4 * sizeof(int));
//        for (int y = 0; y < image->height; y++) {
////            printf("X %d Y %d\n", x, y);
////            printf("%s, %d\n", __func__, __LINE__);
////            if (size <= cnt_rgba-1) {
////                size += 100;
////                colors = realloc(colors, size * sizeof(int*));
////                max_cnt = realloc(colors, size * sizeof(int));
////            }
////            printf("%s, %d\n", __func__, __LINE__);
//            R = image->row_pointers[y][x * stride + 0];
//            G = image->row_pointers[y][x * stride + 1];
//            B = image->row_pointers[y][x * stride + 2];
//            A = image->row_pointers[y][x * stride + 3];
////            printf("%s, %d\n", __func__, __LINE__);
////            printf("%d %d %d %d\n", R,G,B,A);
//            int flag_new = 1;
//            for (int i = 0; i < cnt_rgba; i++) {
//                if (cnt_rgba > size-1) {
//                    printf("SIZE %d CNT %d i %d\n", size,cnt_rgba, i);
//                    size += 100;
//                    colors = realloc(colors, size * sizeof(int*));
//                    printf("%s, %d\n", __func__, __LINE__);
//                    max_cnt = realloc(max_cnt, size * sizeof(int));
//                }
////                printf("%s, %d\n", __func__, __LINE__);
//                if (R == colors[i][0] && G == colors[i][1] && B == colors[i][2] && A == colors[i][3]) {
////                    printf("%d %d %d %d\n", colors[i][0], colors[i][1], colors[i][2], colors[i][3]);
//                    max_cnt[i] += 1;
////                    printf("%s, %d\n", __func__, __LINE__);
//                    flag_new = 0;
//                    break;
//                }
//            }
////            printf("%s, %d\n", __func__, __LINE__);
//            if (flag_new) {
////                printf("%s, %d\n", __func__, __LINE__);
//                colors[cnt_rgba] = malloc(4 * sizeof(int));
//                colors[cnt_rgba][0] = image->row_pointers[y][x * stride + 0];
//                colors[cnt_rgba][1] = image->row_pointers[y][x * stride + 1];
//                colors[cnt_rgba][2] = image->row_pointers[y][x * stride + 2];
//                colors[cnt_rgba][3] = image->row_pointers[y][x * stride + 3];
//                max_cnt[cnt_rgba] += 1;
//                printf("%s, %d\n", __func__, __LINE__);
//                cnt_rgba++;
//            }
////            printf("%s, %d\n", __func__, __LINE__);
//        }
//    }
//    printf("%s, %d CNT %d\n", __func__, __LINE__, cnt_rgba);
//    int max = 0; int number = 0;
//    for (int i = 0; i < cnt_rgba; i++) {
//        if (max_cnt[i] > max) {
//            max = max_cnt[i];
//            number = i;
//        }
//    }
//    int frequent[4];
//    for (int c = 0; c < 4; c++) {
//        frequent[c] = colors[number][c];
//    }
//
////    for (int x = 0; x < image->width; x++) {
////        for (int y = 0; y < image->height; y++) {
////            if
////        }
////    }
//    for (int x = 0; x < image->width; x++) {
////        png_bytep row_old = image->row_pointers[y];
//        for (int y = 0; y < image->height; y++) {
//            png_bytep row_old = image->row_pointers[y];
//            png_bytep ptr_old = &(row_old[x*4]);
//            if (ptr_old[0] == frequent[0] && ptr_old[1] == frequent[1]
//            && ptr_old[2] == frequent[2] && ptr_old[3] == frequent[3]) {
//                ptr_old[0] = new_color[0];
//                ptr_old[1] = new_color[1];
//                ptr_old[2] = new_color[2];
//                ptr_old[3] = new_color[3];
//            }
//        }
//    }
//    for (int i = 0; i < size; i++) {
//        free(colors[i]);
//    }
//    free(colors);
//    free(max_cnt);

//    printf("The most common color is #%06x, occurring %d times\n", max_color, max_count);

//    int *** colors = calloc(256 * sizeof(int**));
//    for (int i = 0; i < 256; i++) {
//        colors[i] = calloc(256, sizeof(int*));
//        for (int j = 0; j < 256; j++) {
//            colors[i][j] = calloc(256, sizeof(int));
//        }
//    }
    int number_of_channels = 4;
    int bit_depth = image->bit_depth;
    int stride = number_of_channels * bit_depth / 8;

    int *** colors = calloc(256, sizeof(int**));
    for (int i = 0; i < 256; i++) {
        colors[i] = calloc(256, sizeof(int**));
        for (int j = 0; j < 256; j++) {
            colors[i][j] = calloc(256, sizeof(int*));
        }
    }

    for (int y = 0; y < image->height; y++) {
        png_bytep row = image->row_pointers[y];
        for (int x = 0; x < image->width; x++) {
            png_bytep ptr = &(row[x * stride]);
            colors[ptr[0]][ptr[1]][ptr[2]]++;
        }
    }
    int max_freq = colors[0][0][0];
//    int max_r = 0, max_g = 0, max_b = 0;
    int max_colors[] = {0,0,0};
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 256; k++) {
                if (colors[i][j][k] > max_freq) {
                    max_colors[0] = i;
                    max_colors[1] = j;
                    max_colors[2] = k;
                    max_freq = colors[i][j][k];
                }
            }
        }
    }
    for (int j = 0; j < image->height; j++) {
        png_bytep row2 = image->row_pointers[j];
        for (int i = 0; i < image->width; i++) {
            png_bytep ptr2 = &(row2[i * stride]);
            if (ptr2[0] == max_colors[0] && ptr2[1] == max_colors[1] && ptr2[2] == max_colors[2]) {
                ptr2[0] = new_color[0];
                ptr2[1] = new_color[1];
                ptr2[2] = new_color[2];
                ptr2[3] = new_color[3];
            }
        }
    }
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            free(colors[i][j]);
        }
        free(colors[i]);
    }
    free(colors);
}

//invertColors();
void invertColors(struct Png * image, int x1, int y1, int x2, int y2) {
//    printf("x1 %d y1 %d x2 %d y2 %d\n", x1, y1, x2, y2);
    if (x1 < 0 ||  x1 >= image->width || y1 < 0 || y1 >= image->height
    || x2 < 0 ||  x2 >= image->width || y2 < 0 || y2 >= image->height) {
        printf("Введены некорретные данные: координаты должны "
               "находиться в пределах изображения и быть не меньше нуля\n");
        return;
    }
    if (x1 > x2 || y1 > y2) {
        printf("Введены некорретные данные: координаты верхнего левого угла "
               "должны быть меньше координат нижнего правого угла.\n");
        return;
    }
    int number_of_channels = 4;
    int bit_depth = image->bit_depth;
    int stride = number_of_channels * bit_depth / 8;
    for (int i = x1; i <= x2; i++) {
        for (int j = y1; j <= y2; j++) {
            image->row_pointers[j][i * stride + 0] = 255 - image->row_pointers[j][i * stride + 0];
            image->row_pointers[j][i * stride + 1] = 255 - image->row_pointers[j][i * stride + 1];
            image->row_pointers[j][i * stride + 2] = 255 - image->row_pointers[j][i * stride + 2];
            image->row_pointers[j][i * stride + 3] = image->row_pointers[j][i * stride + 3];
        }
    }
}

int main(int argc, char **argv) {
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
            {"info", no_argument, NULL, 'i'},
            {"help", no_argument, NULL, 'h'},
            {NULL, 0, NULL, 0},
    };
    int opt;
    int longIndex;
//    char *opts = "q:w:o:n:s:e:l:t:c:f:r:d:i:h:";
    char *opts = "q:w:o:n:ih";
    opt = getopt_long(argc, argv, opts, longOpts, &longIndex);

    int y, x, length, thickness, fill;
    int x1, y1, x2, y2;
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

                break;
            case 'w':
//                    swapAreas();
//                    write_png_file();
                break;
            case 'o':
                color[0] = atoi(argv[3]);
                color[1] = atoi(argv[4]);
                color[2] = atoi(argv[5]);
                color[3] = atoi(argv[6]);
                changeColor(&image, color);
                write_png_file(argv[7], &image);
//                    write_png_file();
                break;
            case 'n':
                x1 = atoi(argv[3]);
                y1 = atoi(argv[4]);
                x2 = atoi(argv[5]);
                y2 = atoi(argv[6]);
                invertColors(&image, x1, y1, x2, y2);
                write_png_file(argv[7], &image);
//                    write_png_file();
                break;
            case 'i':
                print_PNG_info(&image);
                break;
            case 'h':
                print_info();
                break;
        }
        opt = getopt_long(argc, argv, opts, longOpts, &longIndex);
    }

    return 0;
}
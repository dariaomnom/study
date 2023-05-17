#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

struct Png{
    int width, height;
    png_byte color_type;
    png_byte bit_depth;

    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers;
};

void print_PNG_info(struct Png *image){
    printf("Ширина изображения: %d\n", image->width);
    printf("Высота изображения: %d\n", image->height);
    printf("Тип цвета: %u\n", image->color_type);
    printf("Глубина цвета: %u\n", image->bit_depth);
}

void print_info(){
    printf("\nЭто программа с CLI для редактирования png-изображений <з \n"
           "Поддерживаются файлы с глубиной цвета 8 бит и каналами RGBa\n");
    printf("Формат ввода:\033[1;35m ./pngedit [infilename.png] -[o]/--[option] [arguments] [outfilename.png]\033[0m\n\n");

    printf("[имя файла] \033[1;35m-q/--square\033[0m - нарисовать квадрат\n");
    printf("    [x-координата] [y-координата] - левый верхний угол\n");
    printf("    [число] - длина стороны\n");
    printf("    [число] - толщина линий (в пикселях)\n");
    printf("    [R] [G] [B] [A] - числа от 0 до 255, цвет линий\n");
    printf("    [число] - заливка (по умолчанию без заливки) (1 - заливка, 0 - нет)\n");
    printf("    [R] [G] [B] [A] - числа от 0 до 255, цвет заливки\n\n");

    printf("[имя файла] \033[1;35m-w/--swap\033[0m - поменять местами 4 куска области\n");
    printf("    [x-координата] [y-координата] - левый верхний угол\n");
    printf("    [x-координата] [y-координата] - правый нижний угол\n");
    printf("    [число] - способ (circle - по кругу, diagonal - по диагонали)\n\n");

    printf("[имя файла] \033[1;35m-o/--often\033[0m - заменить самый часто встречающийся цвет на новый\n");
    printf("    [R] [G] [B] [A] - числа, новый цвет (RGBa)\n\n");

    printf("[имя файла] \033[1;35m-n/--inversion\033[0m - инвертировать цвет в заданной области\n");
    printf("    [x-координата] [y-координата] - левый верхний угол\n");
    printf("    [x-координата] [y-координата] - правый нижний угол\n\n");

    printf("[имя файла] \033[1;35m-i/--info\033[0m - получить информацию об изображении\n");
    printf("[имя файла] \033[1;35m-h/--help\033[0m - вызвать справку\n\n");
}

void read_png_file(char * file_name, struct Png * image) {
    char header[8];

    FILE *fp = fopen(file_name, "rb");
    if (!fp){
        printf("Ошибка: не удалось открыть файл для чтения. Введите название файла с расширением '.png'\n");
        exit(1);
    }

    fread(header, 1, 8, fp);
    if (png_sig_cmp((const unsigned char *)header, 0, 8)){
        printf("Ошибка: формат изображения не PNG.\n");
        exit(1);
    }

    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!image->png_ptr){
        printf("Ошибка: не удалось создать структуру изображения.\n");
        exit(1);
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("Ошибка: не удалось создать структуру с информацией об изображении.\n");
        exit(1);
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Ошибка инициализации.\n");
        exit(1);
    }

    png_init_io(image->png_ptr, fp);
    png_set_sig_bytes(image->png_ptr, 8);
    png_read_info(image->png_ptr, image->info_ptr);

    image->width = png_get_image_width(image->png_ptr, image->info_ptr);
    image->height = png_get_image_height(image->png_ptr, image->info_ptr);
    image->color_type = png_get_color_type(image->png_ptr, image->info_ptr);
    image->bit_depth = png_get_bit_depth(image->png_ptr, image->info_ptr);

    if (image->bit_depth != 8) {
        printf("Ошибка: поддерживается только 8-битная глубина цвета\n");
        exit(1);
    }

    png_read_update_info(image->png_ptr, image->info_ptr);
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Ошибка чтения изображения.\n");
        exit(1);
    }

    if (png_get_color_type(image->png_ptr, image->info_ptr) != PNG_COLOR_TYPE_RGB_ALPHA){
        printf("Поддерживается только тип цвета RGBa.\n");
        exit(1);
    }

    image->row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * image->height);
    for (int y = 0; y < image->height; y++)
        image->row_pointers[y] = (png_byte *) malloc(png_get_rowbytes(image->png_ptr, image->info_ptr));

    png_read_image(image->png_ptr, image->row_pointers);
    fclose(fp);
}

void write_png_file(char * file_name, struct Png * image) {
//    if (file_name[strlen(file_name)-4] != '.' && file_name[strlen(file_name)-3] != 'p'
//    && file_name[strlen(file_name)-2] != 'n' && file_name[strlen(file_name)-1] != 'g') {
//        printf("Ошибка: не передан аргумент для итогового изображения в расширении '.png'.\n");
//        exit(-1);
//    }
    if (strstr(file_name, ".png") != &(file_name[strlen(file_name)-4])) {
        printf("Ошибка: не передан аргумент для итогового изображения в расширении '.png'.\n");
        exit(1);
    }
    FILE *fp = fopen(file_name, "wb");
    if (!fp){
        printf("Ошибка при создании файла итогового изображения.'\n");
        exit(1);
    }

    image->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!image->png_ptr){
        printf("Ошибка при создании структуры итогового изображения.'\n");
        exit(1);
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("Ошибка при создании структуры с информацией об итоговом изображении.'\n");
        exit(1);
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Ошибка инициализации\n");
        exit(1);
    }
    png_init_io(image->png_ptr, fp);

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Ошибка записи заголовка итогового файла.\n");
        exit(1);
    }

    png_set_IHDR(image->png_ptr, image->info_ptr, image->width, image->height,
                 image->bit_depth, image->color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(image->png_ptr, image->info_ptr);

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Ошибка записи данных итогового файла.\n");
        exit(1);
    }
    png_write_image(image->png_ptr, image->row_pointers);

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Ошибка при окончании записи итогового файла.\n");
        exit(1);
    }
    png_write_end(image->png_ptr, NULL);

    for (int y = 0; y < image->height; y++)
        free(image->row_pointers[y]);
    free(image->row_pointers);

    fclose(fp);
}

//void process_file(struct Png *image) {
//    int x,y;
//    if (png_get_color_type(image->png_ptr, image->info_ptr) == PNG_COLOR_TYPE_RGB){
//        // Some error handling: input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA
//    }
//
//    if (png_get_color_type(image->png_ptr, image->info_ptr) != PNG_COLOR_TYPE_RGBA){
//        // Some error handling: color_type of input file must be PNG_COLOR_TYPE_RGBA
//    }
//
//    for (y = 0; y < image->height; y++) {
//        png_byte *row = image->row_pointers[y];
//        for (x = 0; x < image->width; x++) {
//            png_byte *ptr = &(row[x * 4]);
////            printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n",
////                   x, y, ptr[0], ptr[1], ptr[2], ptr[3]);
//
//            /* set red value to 0 and green value to the blue one */
//            ptr[0] = 0;
//            ptr[1] = ptr[2];
//        }
//    }
//}

void draw_square(struct Png * image, int x, int y, int l, int t, int * color, int fill, int * colorF) {
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
                    // рисование границ
                    image->row_pointers[j][i * stride + c] = color[c];
                } else if (fill && (i >= x1+t && i <= x2-t) && (j >= y1+t && j <= y2-t)) {
                    // заливка
                    image->row_pointers[j][i * stride + c] = colorF[c];
                }
            }
        }
    }

}

void change_frag(struct Png * image, int to_x1, int to_x2, int to_y1, int to_y2, int h, int w, png_bytep ** save) {
    int number_of_channels = 4;
    int bit_depth = image->bit_depth;
    int stride = number_of_channels * bit_depth / 8;

    if (!save) {
        for (int y = to_y1; y <= to_y2; y++) {
            png_byte *row_to = image->row_pointers[y];
            png_byte *row_from = image->row_pointers[y + h];
            for (int x = to_x1; x <= to_x2; x++) {
                png_byte *ptr_to = &(row_to[x * stride]);
                png_byte *ptr_from = &(row_from[(x + w) * stride]);
                ptr_to[0] = ptr_from[0];
                ptr_to[1] = ptr_from[1];
                ptr_to[2] = ptr_from[2];
                ptr_to[3] = ptr_from[3];
            }
        }
    } else {
        for (int y = to_y1, y_save = 0; y <= to_y2 && y_save <= h; y++, y_save++) {
            png_byte *row = image->row_pointers[y];
            for (int x = to_x1, x_save = 0; x <= to_x2 && x_save <= w; x++, x_save++) {
                png_byte *ptr2 = &(row[x * stride]);
                ptr2[0] = save[y_save][x_save][0];
                ptr2[1] = save[y_save][x_save][1];
                ptr2[2] = save[y_save][x_save][2];
                ptr2[3] = save[y_save][x_save][3];
            }
        }
    }


}

png_bytep ** save_area(struct Png * image, int x1, int x2, int y1, int y2) {
    int number_of_channels = 4;
    int bit_depth = image->bit_depth;
    int stride = number_of_channels * bit_depth / 8;

    png_bytep ** save = calloc(y2-y1+1, sizeof(png_bytep *));
    for (int i = 0; i < y2-y1+1; i++) {
        save[i] = (png_bytep*) calloc(x2-x1+1, sizeof(png_bytep));
        for (int j = 0; j < x2-x1+1; j++) {
            save[i][j] = (png_bytep) calloc(4, sizeof(png_byte));
        }
    }
    for (int y = y1, y_s = 0; y <= y2; y++, y_s++) {
        png_byte *row = image->row_pointers[y];
        for (int x = x1, x_s = 0; x <= x2; x++, x_s++) {
            png_byte *ptr = &(row[x * stride]);
            save[y_s][x_s][0] = ptr[0];
            save[y_s][x_s][1] = ptr[1];
            save[y_s][x_s][2] = ptr[2];
            save[y_s][x_s][3] = ptr[3];
        }
    }
    return save;
}

void swap_areas(struct Png * image, int x1, int y1, int x2, int y2, char * type) {
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

    int h = y2 - y1 + 1, w = x2 - x1 + 1;
    if (w % 2) { x2 -= 1; w--; }
    if (h % 2) { y2 -= 1; h--; }
    int h_area = h / 2, w_area = w / 2;

    int area_1_x1 = x1,    area_1_x2 = x1 + w_area-1,    area_1_y1 = y1,    area_1_y2 = y1 + h_area-1;
    int area_2_x1 = area_1_x2+1,    area_2_x2 = area_2_x1 + w_area-1,    area_2_y1 = y1,    area_2_y2 = y1 + h_area-1;
    int area_3_x1 = x1,    area_3_x2 = area_1_x2,    area_3_y1 = area_1_y2+1,    area_3_y2 = area_3_y1 + h_area-1;
    int area_4_x1 = area_3_x2+1,    area_4_x2 = area_4_x1 + w_area-1,    area_4_y1 = area_2_y2+1,    area_4_y2 = area_4_y1 + h_area-1;

    png_bytep ** save_pix_1 = save_area(image, area_1_x1, area_1_x2, area_1_y1, area_1_y2); // save area 1
    png_bytep ** save_pix_3 = save_area(image, area_3_x1, area_3_x2, area_3_y1, area_3_y2); // save area 3

    if (!strcasecmp(type, "circle")) {
        change_frag(image, area_1_x1, area_1_x2, area_1_y1, area_1_y2, h_area, 0, NULL); // area 3 to area 1
        change_frag(image, area_3_x1, area_3_x2, area_3_y1, area_3_y2, 0, w_area, NULL); // area 4 to area 3
        change_frag(image, area_4_x1, area_4_x2, area_4_y1, area_4_y2, 0-h_area, 0, NULL); // area 2 to area 4
        change_frag(image, area_2_x1, area_2_x2, area_2_y1, area_2_y2, h_area, w_area, save_pix_1); // area 1 (saved) to 2
    } else if (!strcasecmp(type, "diagonal")) {
        change_frag(image, area_1_x1, area_1_x2, area_1_y1, area_1_y2, h_area, w_area, NULL); // area 3 to area 1
        change_frag(image, area_3_x1, area_3_x2, area_3_y1, area_3_y2, 0-h_area, w_area, NULL); // area 3 to area 1
        change_frag(image, area_4_x1, area_4_x2, area_4_y1, area_4_y2, h_area, w_area, save_pix_1); // area 1 (saved) to 4
        change_frag(image, area_2_x1, area_2_x2, area_2_y1, area_2_y2, h_area, w_area, save_pix_3); // area 3 (saved) to 2
    }

}

void change_color(struct Png * image, int * new_color) {
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

    int freq = colors[0][0][0];
    int max_colors[] = {0,0,0};
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 256; k++) {
                if (colors[i][j][k] > freq) {
                    max_colors[0] = i;
                    max_colors[1] = j;
                    max_colors[2] = k;
                    freq = colors[i][j][k];
                }
            }
        }
    }

    for (int j = 0; j < image->height; j++) {
        png_bytep row = image->row_pointers[j];
        for (int i = 0; i < image->width; i++) {
            png_bytep ptr = &(row[i * stride]);
            if (ptr[0] == max_colors[0] && ptr[1] == max_colors[1] && ptr[2] == max_colors[2]) {
                ptr[0] = new_color[0];
                ptr[1] = new_color[1];
                ptr[2] = new_color[2];
                ptr[3] = new_color[3];
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

void invert_colors(struct Png * image, int x1, int y1, int x2, int y2) {
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
//int stride = 4;
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
    if(argc == 1){
        print_info();
        return 0;
    }
    struct Png image;
    char* output = argv[2];
    read_png_file(argv[1], &image);
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

    int y = 0, x = 0, length = 0, thickness = 0, fill = 0;
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    int color[] = {0,0,0,0};
    int color_fill[] = {0,0,0,0};
    char swap_type[10];

    while (opt != -1) {
        switch (opt) {
            case 'q':
                if(sscanf(optarg,"%d %d %d\n", &x, &y, &lineWidth)){
                    if(x1 < 0, y1 < 0){
                        printf("\nКоличество линий не может быть меньше нуля\n");
                        return 0;
                    }
                    if(W - lineWidth*x1 < x1+1 || H - lineWidth*y1 < y1+1){
                        printf("\nЗадайте меньшую толщину линии-разделителя\n");
                        return 0;
                    }
                    if(cutBMP(&picture, x1, y1, lineWidth, colour1, path2)){
                        printf("\nКартинка успешно разделена\n");
                    }
                    else{
                        return 0;
                    }
                }
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
                    draw_square(&image, x, y, length, thickness, color, fill, color_fill);
                    write_png_file(argv[16], &image);
                } else {
                    draw_square(&image, x, y, length, thickness, color, fill, NULL);
                    write_png_file(argv[12], &image);
                }
                break;
            case 'w':
                x1 = atoi(argv[3]);
                y1 = atoi(argv[4]);
                x2 = atoi(argv[5]);
                y2 = atoi(argv[6]);
                strcpy(swap_type, argv[7]);
                swap_areas(&image, x1, y1, x2, y2, swap_type);
                write_png_file(argv[8], &image);
                break;
            case 'o':
                color[0] = atoi(argv[3]);
                color[1] = atoi(argv[4]);
                color[2] = atoi(argv[5]);
                color[3] = atoi(argv[6]);
                change_color(&image, color);
                write_png_file(argv[7], &image);
                break;
            case 'n':
                x1 = atoi(argv[3]);
                y1 = atoi(argv[4]);
                x2 = atoi(argv[5]);
                y2 = atoi(argv[6]);
                invert_colors(&image, x1, y1, x2, y2);
                write_png_file(argv[7], &image);
                break;
            case 'i':
                print_PNG_info(&image);
                break;
            case 'h':
                print_info();
                break;
            default:
                printf("Команды не введены.\n");
        }
        opt = getopt_long(argc, argv, opts, longOpts, &longIndex);
    }








    if (argc < 3) {printf("Не введено название опции.\n"); return 0;}
    char *choice[10];
    strcpy(choice, argv[2]);

//    int p1[2] = {-1, -1}, p2[2] = {-1, -1}, p3[2] = {-1, -1};
//
//    int rad = -1, thick = -1, fill_flag = 0, pattern = -1;
//    Rgb color1 = {0, 0, 0}, color2 = {0, 0, 0};
//    char *wfile_name = argv[1];

    int y = 0; int x = 0; int length = 0; int thickness = 0;
    int fill = 0; int x1 = 0; int y1 = 0; int x2 = 0; int y2 = 0;
    int* color = {0,0,0,0}; int* color_fill = {0,0,0,0}; char* swap_type[10];

    opterr = 0;
    getAllKeys(argc, argv, &y, &x, &length, &thickness, &fill, &x1, &y1, &x2, &y2, &color, &color_fill, &swap_type);

    if (!strcmp(choice, "square")){
        if (fill) draw_square(&image, x, y, length, thickness, color, fill, colorF);
        else draw_square(&image, x, y, length, thickness, color, fill, NULL);
    }

    else if (!strcmp(choice, "swap")){
        swap_areas(&image, x1, y1, x2, y2, swap_type);
    }

    else if (!strcmp(choice, "often")){
        change_color(&image, color);
    }

    else if (!strcmp(choice, "inversion")){
        invert_colors(&image, x1, y1, x2, y2);
    }
    else if (!strcmp(choice, "info")){
        print_PNG_info();
    }
    else if (!strcmp(choice, "help")){
        print_info();
    }
    else printf("Неизвестное название опции.\n");


    return 0;
}

void getAllKeys(int argc, char *argv[],
                int* y, int* x = 0, int* length = 0, int* thickness = 0,
                int* fill = 0, int x1 = 0, int* y1 = 0, int* x2 = 0, int* y2 = 0,
                int** color, int** color_fill, char** swap_type){
    struct option long_opts[] = {
            {"square", required_argument, NULL, 'q'},
            {"swap", required_argument, NULL, 'w'},
            {"often", required_argument, NULL, 'o'},
            {"inversion", required_argument, NULL, 'n'},
            {"info", no_argument, NULL, 'i'},
            {"help", no_argument, NULL, 'h'},

            {"y", required_argument, NULL, 'y_square'},
            {"x", required_argument, NULL, 'x_square'},
            {"l", required_argument, NULL, 'length'},
            {"t", required_argument, NULL, 'thinkness'},
            {"f", required_argument, NULL, 'fill'},
            {"start", required_argument, NULL, 'x1_y1_area'},
            {"end", required_argument, NULL, 'x2_y2_area'},
//            {"x1", required_argument, NULL, 'x1_area'},
//            {"y1", required_argument, NULL, 'x2_area'},
//            {"x2", required_argument, NULL, 'y1_area'},
//            {"y2", required_argument, NULL, 'y2_area'},
            {"c", required_argument, NULL, 'color'},
            {"cf", required_argument, NULL, 'color_fill'},
            {"p", required_argument, NULL, 'type'},
            {NULL, 0, NULL, 0}
    };
    char *short_opts = "q:w:o:n:ihy:x:l:t:f:start:end:c:cf:p:";
    int opt;

    while ((opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1){
        switch (opt){
            case 'q':{
                int ind = optind - 1;

//                int arg_len = strlen(argv[ind]);
//
//                if (!isdigit(argv[ind][0])) break;
//                (*p1)[0] = atoi(argv[ind]);
//
//                int i = 0;
//                for (; argv[ind][i] != '.'; i++)
//                    if (i >= arg_len) break;
//                if (i == arg_len) break;
//
//                if (!isdigit(argv[ind][i + 1])) break;
//                (*p1)[1] = atoi(&argv[ind][i + 1]);

                break;
            }
            case 'e':{
                int ind = optind - 1;
                int arg_len = strlen(argv[ind]);

                if (!isdigit(argv[ind][0])) break;
                (*p2)[0] = atoi(argv[ind]);

                int i = 0;
                for (; argv[ind][i] != '.'; i++)
                    if (i >= arg_len) break;
                if (i == arg_len) break;

                if (!isdigit(argv[ind][i + 1])) break;
                (*p2)[1] = atoi(&argv[ind][i + 1]);

                break;
            }
            case 'z':{
                int ind = optind - 1;
                int arg_len = strlen(argv[ind]);

                if (!isdigit(argv[ind][0])) break;
                (*p3)[0] = atoi(argv[ind]);

                int i = 0;
                for (; argv[ind][i] != '.'; i++)
                    if (i >= arg_len) break;
                if (i == arg_len) break;

                if (!isdigit(argv[ind][i + 1])) break;
                (*p3)[1] = atoi(&argv[ind][i + 1]);

                break;
            }
            case 'r':{
                if (!isdigit(argv[optind - 1][0])) break;
                *rad = atoi(argv[optind - 1]);
                break;
            }
            case 't':{
                if (!isdigit(argv[optind - 1][0])) break;
                *thickness = atoi(argv[optind - 1]);
                break;
            }
            case 'f':{
                *fill_flag = 1;
                break;
            }
            case 'p':{
                if (!isdigit(argv[optind - 1][0])) break;
                *pattern = atoi(argv[optind - 1]);
                if (*pattern < 1 || *pattern > 3) *pattern = -1;
                break;
            }
            case 'c':{
                int ind = optind - 1;
                int arg_len = strlen(argv[ind]);

                if (!isdigit(argv[ind][0])) break;
                int r = atoi(argv[ind]);
                if (r >= 0 && r <= 255) color1->r = r;

                int i = 0;
                for (; argv[ind][i] != '.'; i++)
                    if (i >= arg_len) break;
                if (i == arg_len) break;
                i ++;

                if (!isdigit(argv[ind][i])) break;
                int g = atoi(&argv[ind][i]);
                if (g >= 0 && g <= 255) color1->g = g;

                for (; argv[ind][i] != '.'; i++)
                    if (i >= arg_len) break;
                if (i == arg_len) break;
                i ++;

                if (!isdigit(argv[ind][i])) break;
                int b = atoi(&argv[ind][i]);
                if (b >= 0 && b <= 255) color1->b = b;

                break;
            }
            case 'C':{
                int ind = optind - 1;
                int arg_len = strlen(argv[ind]);

                if (!isdigit(argv[ind][0])) break;
                int r = atoi(argv[ind]);
                if (r >= 0 && r <= 255) color2->r = r;

                int i = 0;
                for (; argv[ind][i] != '.'; i++)
                    if (i >= arg_len) break;
                if (i == arg_len) break;
                i ++;

                if (!isdigit(argv[ind][i])) break;
                int g = atoi(&argv[ind][i]);
                if (g >= 0 && g <= 255) color2->g = g;

                for (; argv[ind][i] != '.'; i++)
                    if (i >= arg_len) break;
                if (i == arg_len) break;
                i ++;

                if (!isdigit(argv[ind][i])) break;
                int b = atoi(&argv[ind][i]);
                if (b >= 0 && b <= 255) color2->b = b;

                break;
            }
            case 'o':{
                *wfile = argv[optind - 1];
            }
            default:
                break;
        }
    }
}
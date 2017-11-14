/************************************************************************************
 * @file encode.c
 * @author Erias Diego, Pisanello Antonio, Rmiza Hassine
 * @date 29 Oct 2017
 * @brief Encode a text into a ppm file.
 *
 * This program receive, via argument, the path of a text file, and 2 ppm images
 * (input and output). It also receive a number of threads. The program will then
 * encode the text changing if needed the lowest bit from the input image and
 * outputing the new image. It will encode in multi-threading.
 ***********************************************************************************/

#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <libgen.h>
#include <unistd.h>
#include "encode_lib.h"
#include "../libs/alloc.h"
#include "../libs/files.h"

const uint BYTES_HEADER_CHAR = sizeof(int) * 8;
const uint FIRST_PIXEL = 11;
const int NB_ARG = 4;

/***********************************************************
 * Store where the threads begins
 * @param initial_indice Pixel of the image
 * @param initial_pos_rgb R, G or B (0, 1 or 2) in the pixel
 ***********************************************************/
typedef struct limit_threads_st{
	int initial_indice;
	uint8_t initial_pos_rgb;
} limit_threads_t;

/***********************************************************
 * Store the arguments for the threads
 * @param limit see the struct limit_threads_t
 * @param text_cut pointer to the part of the text we encode
 * @param img a pointer to the image to write
 ***********************************************************/
typedef struct param_st{
	limit_threads_t limit;
	char *text_cut;
	img_t **img_out;
} param_t;

/***********************************************************
 * The maximum of chars that can fit in a picture
 * @param img a pointer to the image to read
 * @return the maximum of chars that can fit in a picture
 ***********************************************************/
uint max_char_encode(img_t *img){
	return floor((img->height * img->width - FIRST_PIXEL) * 
                 (float) sizeof(pixel_t) / (float) BITS_PER_CHAR);
}

/***********************************************************
 * To know the limits of a thread
 * @param min first char position of the cut text
 * @return where the threads begins (pixel position)
 ***********************************************************/
limit_threads_t get_limits(int min){
	limit_threads_t ret;
	
	ret.initial_indice = floor((float)(min * BITS_PER_CHAR) / sizeof(pixel_t)) + FIRST_PIXEL;
	ret.initial_pos_rgb = (min * BITS_PER_CHAR) % sizeof(pixel_t);

	return ret;
}

/***********************************************************
 * Threads doing the encoding
 * @param param see the struct param_t
 * @return return NULL if no problem encountered
 ***********************************************************/
void *thread(void *param){
    // Get arguments
    param_t *p = (param_t *)param;

    int initial_ind = p->limit.initial_indice;
    int initial_pos = p->limit.initial_pos_rgb;
    img_t **img_out = p->img_out;
    // Position the pointer to the first pixel (on R, G or B) we want to encode
	uint8_t *ptr = &(*img_out)->raw[initial_ind].r + initial_pos;

    // This loop go through each char of the cut text
    for (uint i = 0; i < strlen(p->text_cut); i++){
        // Current char
		char temp = p->text_cut[i];
        // For each char it must encode 7 lowest bits
		for(int8_t indice = BITS_PER_CHAR - 1; indice >= 0;indice--){
            // Initialise bit to zero and calculate current pow of 2
			uint8_t bit = 0;
			uint8_t temp_pow = (uint8_t) pow(2, indice);
            // If current char current char >= current pow..
            // ..bit to 1 and this pow of 2 is reduced to the char
			if(temp >= temp_pow){
				bit = 1;
				temp -= temp_pow;
			}
            // Encode the bit into the R, G or B
			*(ptr) = encode_int(*(ptr), bit);
            
            // Reposition the pixel to the next R, G or B
			ptr++;
		}
	}
    return NULL;
}

/***********************************************************
 * Display the program's syntaxe.
 * @param argv program's command line arguments
 ***********************************************************/
void usage(char **argv){
	fprintf(stderr, "\nNumber of arguments is invalid\n\n"\
        "usage: %s text_file input_image output_image thread_count\n"\
		"       where input_image and output_image are PPM files\n"\
		"       and thread_count the number of threads to use.\n", basename(argv[0]));
	exit(EXIT_FAILURE);
}

/****************************************************
 * Program entry point.
 * @param argc command line argument count
 * @param argv program's command line arguments
 ****************************************************/
int main(int argc, char **argv){	
    // Parse command line
	if(argc != NB_ARG + 1)
		usage(argv);
	char *filename=argv[1];
	char *input=argv[2];
	char *output=argv[3];
	int nb_threads = atoi(argv[4]);
    
	float interval;
	img_t *img;
	char  *text;
   	char  *text_cut;
    
    // Load the image, see the max char that it can contains..
	img = load_ppm(input);
	uint max_char = max_char_encode(img);
   	uint nb_char = fsize(filename);
    //.. and compare it to the number of chars in the text 
	if (nb_char > max_char){
		fprintf(stderr,"TEXT TOO LONG FOR THIS IMAGE\nExiting now...\n");
		exit(EXIT_FAILURE);
	}
    
    // Check if there is a correct number of threads
    if(nb_threads <= 0){
        fprintf(stderr,"NUMBERS OF THREADS MUST BE GREATER THAN ZERO\nExiting now...\n");
		exit(EXIT_FAILURE);
    }
    
    // Check if there is more threads thans char in the text and allocate memory
	if(nb_threads > (int)nb_char)
        nb_threads = nb_char;
	pthread_t *threads = my_malloc(nb_threads * sizeof(pthread_t));
	param_t   *threads_param = my_malloc(nb_threads * sizeof(param_t));
	char      *nb_char_header = my_calloc(BYTES_HEADER_CHAR + 1, sizeof(char));
	
    // Write in the first pixels the number of chars of the text
	int_to_bin_str(nb_char, nb_char_header, BYTES_HEADER_CHAR);
	write_nb_char_in_img(nb_char_header, &img);
    free(nb_char_header);
    
    // Convert the file text to a string
	file_to_str(filename, nb_char, &text);
    
    // Get the general (float) interval of each text cut
	interval = (float)nb_char / (float)nb_threads;
    
    // Threads launching loop
	for (int i = 0; i < nb_threads; i++){
        // Get the "real" interval and limits
		int min = round(interval * i) + 0;
		int char_in_interval = round(interval * (i + 1)) - min;
		text_cut = my_calloc(char_in_interval + 1, sizeof(char));
        
        // Cut the text
        memcpy(text_cut, text + min, char_in_interval);
        
        // Assign the threads arguments
        threads_param[i].limit = get_limits(min);
        threads_param[i].text_cut = text_cut;
        threads_param[i].img_out = &img;

        // Create thread and check for fail
        if (pthread_create(&threads[i], NULL, thread, &threads_param[i]) != 0){
            fprintf(stderr, "pthread_create failed!\n");
            exit(0);
        }
	}
    free(text);
    
    // Threads "waiting" loop
	for (int i = 0; i < nb_threads; i++){
        pthread_join(threads[i], NULL);
    }
    
    printf("%u threads were used\n", nb_threads);
    
    // Write image
    if(!write_ppm(output, img, PPM_BINARY)){
		fprintf(stderr, "ERROR CREATING THE OUTPUT FILE\nExiting now...\n");
        free_img(img);
		exit(EXIT_FAILURE);
    }
    
    free_img(img);
    free(text_cut);
    free(threads_param);
    free(threads);
	return EXIT_SUCCESS;
}

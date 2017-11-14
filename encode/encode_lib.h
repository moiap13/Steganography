/************************************************************************************
 * @file encode_lib.h
 * @author Erias Diego, Pisanello Antonio, Rmiza Hassine
 * @date 29 oct 2017
 * @brief Routines used by the main encode file
 ***********************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "../libs/ppm.h"

#define BITS_PER_CHAR 7

void decode_char(char a, char* b);
uint8_t encode_char(uint8_t rgb, char c);
uint8_t encode_int(uint8_t rgb, uint8_t b);
char *int_to_bin_str(int a, char *buffer, int buf_size);
void write_nb_char_in_img(char *nb_char, img_t **img_out);
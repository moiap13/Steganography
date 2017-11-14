/************************************************************************************
 * @file encode_lib.c
 * @author Erias Diego, Pisanello Antonio, Rmiza Hassine
 * @date 29 oct 2017
 * @brief Routines used by the main encode file
 ***********************************************************************************/
#include "encode_lib.h"

/***********************************************************
 * Encode into the RGB component one part of a char
 * @param rgb component to encode
 * @param char '0' or '1' to encode
 * @return the encoded component
 ***********************************************************/
uint8_t encode_char(uint8_t rgb, char c){
    if(!((c == '0')||(c == '1'))){
        fprintf(stderr,"C IS A \"BIT\" VALUE MUST BE '0' OR '1'\n"\
        "Exiting now...\n");
		exit(EXIT_FAILURE);
    }
	rgb >>= 1;
	rgb <<= 1;
	rgb += c - '0';

	return rgb;
}

/***********************************************************
 * Encode into the RGB component one part of a char
 * @param rgb component to encode
 * @param b 0 or 1 to encode
 * @return the encoded component
 ***********************************************************/
uint8_t encode_int(uint8_t rgb, uint8_t b){
    if(b > 1){
        fprintf(stderr,"B IS A BIT SO VALUE MUST BE 0 OR 1\n"\
        "Exiting now...\n");
		exit(EXIT_FAILURE);
    }
	rgb >>= 1;
	rgb <<= 1;
	rgb += b;

	return rgb;
}

/***********************************************************
 * Convert an int into a binary string
 * @param a int to convert
 * @param buffer string to convert
 * @param buf_size size of the string
 * @return the binary string
 ***********************************************************/
char *int_to_bin_str(int a, char *buffer, int buf_size){
    buffer += (buf_size - 1);
    for (int i = buf_size - 1; i >= 0; i--){
        *buffer-- = (a & 1) + '0';
        a >>= 1;
    }
    return buffer;
}

/***********************************************************
 * Write the number of chars that the image contains
 * @param nb_char a pointer to the string with the number of
 *                chars in binary
 * @param img a pointer to the image to write
 ***********************************************************/
void write_nb_char_in_img(char *nb_char, img_t **img_out){
	uint8_t *rgb = &(*img_out)->raw[0].r;

	for (uint8_t i = 0; i < strlen(nb_char); i++)
		*(rgb+i) = encode_char(*(rgb+i), nb_char[i]);
}
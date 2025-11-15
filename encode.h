#ifndef ENCODE_H
#define ENCODE_H

#include "types.h" // Contains user defined types
#include<stdio.h>

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _EncodeInfo
{
    /* Source Image info */
    char *src_image_fname;// storing .bmp file name
    FILE *fptr_src_image;// storing address of .bmp file, opening in r mode
    uint image_capacity;// storing file size 2359351
    uint bits_per_pixel; // optional
    char image_data[MAX_IMAGE_BUF_SIZE]; // optional

    /* Secret File Info */
    char *secret_fname;// storing secret file
    FILE *fptr_secret;// opening file in r mode
    char extn_secret_file[MAX_FILE_SUFFIX];// storing the .txt, .sh extension files with the size of 4
    char secret_data[MAX_SECRET_BUF_SIZE];//
    long size_secret_file; // storing size of the secret file 25

    /* Stego Image Info */
    char *stego_image_fname; // destination file name
    FILE *fptr_stego_image; // open file in w mode

} EncodeInfo;

/* Encoding function prototype */

/* Read and validate Encode args from argv */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);

/* Perform the encoding */
Status do_encoding(EncodeInfo *encInfo);

/* Get File pointers for i/p and o/p files */
Status open_files(EncodeInfo *encInfo);

/* check capacity */
Status check_capacity(EncodeInfo *encInfo);

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Get file size */
uint get_file_size(FILE *fptr);

/* Copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

/* Store Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/*Extend file size*/
Status encode_secret_extn_file_size(long file_size, EncodeInfo *encInfo); 

/* Encode secret file extenstion */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);

/* Encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo);

/* Encode function, which does the real encoding */
Status encode_int_to_image(int size, char *image_buffer); //32 bytes

/* Encode a byte into LSB of image data array */
Status encode_byte_to_lsb(char data, char *image_buffer); //8 bytes

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

#endif

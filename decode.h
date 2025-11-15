#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types
#include <stdio.h>


#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *op_image_fname;// storing .bmp file name
    FILE *fptr_op_image;// storing address of .bmp file, opening in r mode

    /* Secret File Info */
    char *out_fname;// output file file
    FILE *out_secret;// output file pointer
    char extn_secret_file[MAX_FILE_SUFFIX];// storing the .txt, .sh, .c extension
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file; // decoded secret file 

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Encode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the encoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* Copy bmp image header */
Status skip_bmp_header(FILE *fptr_op_image);

/* Store Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/*Extend file size*/
long decode_secret_extn_file_size(DecodeInfo *decInfo); 

/* Encode secret file extenstion */
Status decode_secret_file_extn(int extn_size, DecodeInfo *decInfo);

/* Encode secret file size */
long decode_secret_file_size(DecodeInfo *decInfo);

/* Encode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "common.h"
#include "types.h"

static char decode_byte_from_lsb(char *image_buffer); // Decode one byte from 8 image bytes
static int decode_int_from_lsb(char *image_buffer);   // Decode integer from 32 image bytes
static Status create_output_file_name(DecodeInfo *decInfo); // Create final output file name with extension

/* Read and validate decode arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Validate that input stego image is a .bmp file
    if (strstr(argv[2], ".bmp"))
    {
        decInfo->op_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    if (argv[3] != NULL)
    {
        decInfo->out_fname = argv[3];
    }
    else
    {
        decInfo->out_fname = malloc(50);
        strcpy(decInfo->out_fname, "default");
    }

    return e_success;
}

/* Open stego image file */
Status open_decode_files(DecodeInfo *decInfo)
{
    // Open the stego image in binary read mode
    decInfo->fptr_op_image = fopen(decInfo->op_image_fname, "rb");

    // Check if file opened successfully
    if (decInfo->fptr_op_image == NULL)
    {
        perror("fopen"); // Print error if file open fails
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->op_image_fname);
        return e_failure;
    }

    return e_success;
}

/* Skip the 54-byte BMP header */
Status skip_bmp_header(FILE *fptr_op_image)
{
    // Move the file pointer after 54-byte header
    fseek(fptr_op_image, 54, SEEK_SET);
    return e_success;
}

/* Decode one byte (8 bits) from 8 image bytes */
static char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;

    // Combine 8 LSBs into one byte
    for (int i = 0; i < 8; i++)
        data = (data << 1) | (image_buffer[i] & 1);

    return data;
}

/* Decode one integer (32 bits) from 32 image bytes */
static int decode_int_from_lsb(char *image_buffer)
{
    int value = 0;

    // Combine 32 LSBs into one integer
    for (int i = 0; i < 32; i++)
        value = (value << 1) | (image_buffer[i] & 1);

    return value;
}

/* Decode and verify magic string */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char image_buffer[8], magic_read[10];

    // Decode each byte of magic string
    for (int i = 0; i < strlen(magic_string); i++)
    {
        fread(image_buffer, 1, 8, decInfo->fptr_op_image);     // Read 8 bytes from image
        magic_read[i] = decode_byte_from_lsb(image_buffer);    // Decode 1 character
    }

    magic_read[strlen(magic_string)] = '\0'; // Null terminate string

    // Compare decoded magic string with the expected one
    if (strcmp(magic_read, magic_string) == 0)
    {
        return e_success;  // If match found
    }
    else
    {
        return e_failure;  // If mismatch
    }
}


/* Decode 32 bits to get extension size */
long decode_secret_extn_file_size(DecodeInfo *decInfo)
{
    char image_buffer[32];

    // Read 32 bytes for extension size
    fread(image_buffer, 1, 32, decInfo->fptr_op_image);

    // Convert 32 bits into integer value
    return decode_int_from_lsb(image_buffer);
}

/* Decode extension string (.txt, .c, .sh, etc.) */
Status decode_secret_file_extn(int extn_size, DecodeInfo *decInfo)
{
    char image_buffer[8];

    // Decode each character of file extension
    for (int i = 0; i < extn_size; i++)
    {
        fread(image_buffer, 1, 8, decInfo->fptr_op_image); // Read 8 bytes per char
        decInfo->extn_secret_file[i] = decode_byte_from_lsb(image_buffer); // Decode char
    }

    decInfo->extn_secret_file[extn_size] = '\0'; // Null terminate decoded extension

    return e_success;
}

/* Decode 32 bits to get secret file size */
long decode_secret_file_size(DecodeInfo *decInfo)
{
    char image_buffer[32];

    // Read 32 bytes from image
    fread(image_buffer, 1, 32, decInfo->fptr_op_image);

    // Convert to integer (file size)
    return decode_int_from_lsb(image_buffer);
}

/* Decode the actual secret data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8], ch;

    // Decode each byte of secret data
    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(image_buffer, 1, 8, decInfo->fptr_op_image); // Read 8 bytes
        ch = decode_byte_from_lsb(image_buffer);           // Extract one char
        fwrite(&ch, 1, 1, decInfo->out_secret);            // Write to output file
    }

    return e_success;
}

/* Create output file name by adding decoded extension */
static Status create_output_file_name(DecodeInfo *decInfo)
{
    char full_name[100];

    // Combine output base name + decoded extension
    snprintf(full_name, sizeof(full_name), "%s%s", decInfo->out_fname, decInfo->extn_secret_file);

    // Allocate memory for final file name
    decInfo->out_fname = malloc(strlen(full_name) + 1);
    if (decInfo->out_fname == NULL)
    {
        fprintf(stderr, "ERROR: Memory allocation failed for output filename\n");
        return e_failure;
    }

    strcpy(decInfo->out_fname, full_name); // Copy final file name

    return e_success;
}

/* Perform the decoding operation */
Status do_decoding(DecodeInfo *decInfo)
{
    // Step 1: Open the stego image file
    if (open_decode_files(decInfo) == e_failure)
        return e_failure;

    // Step 2: Skip 54-byte BMP header
    if (skip_bmp_header(decInfo->fptr_op_image) == e_failure)
        return e_failure;

    // Step 3: Decode and check magic string
    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure)
        return e_failure;

    // Step 4: Decode size of file extension
    long extn_size = decode_secret_extn_file_size(decInfo);

    // Step 5: Decode the extension string
    if (decode_secret_file_extn(extn_size, decInfo) == e_failure)
        return e_failure;

    // Step 6: Create output filename with decoded extension
    if (create_output_file_name(decInfo) == e_failure)
        return e_failure;

    // Step 7: Open decoded output file
    decInfo->out_secret = fopen(decInfo->out_fname, "w");
    if (decInfo->out_secret == NULL)
        return e_failure;

    // Step 8: Decode secret file size
    decInfo->size_secret_file = decode_secret_file_size(decInfo);

    // Step 9: Decode and write secret data
    if (decode_secret_file_data(decInfo) == e_failure)
        return e_failure;

    // Step 10: Close both files
    fclose(decInfo->fptr_op_image);
    fclose(decInfo->out_secret);

    return e_success;
}

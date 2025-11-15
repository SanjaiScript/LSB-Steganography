#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);+

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb"); //opeming file in read binary(rb) mode
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname); // printing error in the std error stream

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb"); // opening file in write binary (wb) mode
    printf("DEBUG: trying to create %s\n", encInfo->stego_image_fname);
if (encInfo->fptr_stego_image == NULL)
{
    perror("fopen");
    fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
    return e_failure;
}
else
{
    printf("DEBUG: stego image file opened successfully!\n");
}

    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* Read and validate Encode args from argv */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Validate source image file (.bmp)
    if (argv[2][0] != '.')
    {
        if (strstr(argv[2], ".bmp"))
            encInfo->src_image_fname = argv[2];
        else
            return e_failure;
    }
    else
        return e_failure;

     // Validate secret file
    if (argv[3] == NULL)
    {
        return e_failure;
    }
    // Validate secret file (.txt / .c / .sh)
    if (argv[3][0] != '.')
    {
        if (strstr(argv[3], ".txt") || strstr(argv[3], ".c") || strstr(argv[3], ".sh"))
        {
            encInfo->secret_fname = argv[3];

            // Extract and store file extension (like ".txt")
            char *dot = strchr(argv[3], '.');
            if (dot != NULL)
            {
                strcpy(encInfo->extn_secret_file, dot);
            }
            else
            {
                strcpy(encInfo->extn_secret_file, ".txt"); // default if not found
            }
        }
        else
            return e_failure;
    }
    else
        return e_failure;

    // If output name not given, use default "default.bmp"
    if (argv[4] == NULL)
    {
        encInfo->stego_image_fname = "default.bmp";
    }
    else
    {
        if (argv[4][0] != '.')
        {
            if (strstr(argv[4], ".bmp"))
            {
                encInfo->stego_image_fname = argv[4];
            }
            else
            {
                return e_failure;
            }
        }
    }

    return e_success;
}

/* Encode integer (32 bits) into 32 LSBs of image buffer */
Status encode_int_to_image(int size, char *image_buffer)
{
    for (int i = 31; i >= 0; i--)
    {
        int bit_data = (size >> i) & 1;                  // Extract each bit from MSB to LSB
        image_buffer[31 - i] = image_buffer[31 - i] & (~1); // Clear LSB
        image_buffer[31 - i] = image_buffer[31 - i] | bit_data; // Set LSB with bit_data
    }
    return e_success;
}

/* Check whether image has enough capacity to store secret data */
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image); // Total bytes in image

    // Get secret file size
    fseek(encInfo->fptr_secret, 0, SEEK_END);
    encInfo->size_secret_file = ftell(encInfo->fptr_secret);
    rewind(encInfo->fptr_secret);

    // Total bits required for encoding all data
    long total_size_needed = (strlen(MAGIC_STRING) * 8) + 32 +
                             (strlen(encInfo->extn_secret_file) * 8) + 32 +
                             (encInfo->size_secret_file * 8);

    if (encInfo->image_capacity > total_size_needed)
    {
        return e_success;
    }
    else
    {
        fprintf(stderr, "ERROR: Image does not have enough capacity\n");
        return e_failure;
    }
}

/* Copy 54-byte BMP header from source to destination */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char image_buffer[54];
    rewind(fptr_src_image);                         // Move to beginning
    fread(image_buffer, 1, 54, fptr_src_image);     // Read header
    fwrite(image_buffer, 1, 54, fptr_dest_image);   // Write header
    return e_success;
}

/* Encode a single byte into 8 LSBs of image data */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 7; i >= 0; i--)
    {
        int bit_data = (data >> i) & 1;             // Extract bit (MSB → LSB)
        image_buffer[7 - i] = image_buffer[7 - i] & (~1); // Clear LSB
        image_buffer[7 - i] = image_buffer[7 - i] | bit_data; // Write bit into LSB
    }
    return e_success;
}

/* Encode magic string (used for identification) */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char image_buffer[8];
    for (int i = 0; i < strlen(magic_string); i++)
    {
        fread(image_buffer, 1, 8, encInfo->fptr_src_image);     // Read 8 bytes
        encode_byte_to_lsb(magic_string[i], image_buffer);      // Encode 1 char
        fwrite(image_buffer, 1, 8, encInfo->fptr_stego_image);  // Write back
    }
    return e_success;
}

/* Encode size of secret file extension (32 bits) */
Status encode_secret_extn_file_size(long file_size, EncodeInfo *encInfo)
{
    char image_buffer[32];
    fread(image_buffer, 1, 32, encInfo->fptr_src_image);        // Read 32 bytes
    encode_int_to_image(file_size, image_buffer);               // Encode size
    fwrite(image_buffer, 1, 32, encInfo->fptr_stego_image);     // Write encoded bytes
    return e_success;
}

/* Encode file extension (.txt / .c / .sh) */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char image_buffer[8];
    for (int i = 0; i < strlen(file_extn); i++)
    {
        fread(image_buffer, 1, 8, encInfo->fptr_src_image);      // Read 8 bytes
        encode_byte_to_lsb(file_extn[i], image_buffer);          // Encode one char
        fwrite(image_buffer, 1, 8, encInfo->fptr_stego_image);   // Write encoded bytes
    }
    return e_success;
}

/* Encode size of secret file (32 bits) */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char image_buffer[32];
    fread(image_buffer, 1, 32, encInfo->fptr_src_image);         // Read 32 bytes
    encode_int_to_image(file_size, image_buffer);                // Encode size
    fwrite(image_buffer, 1, 32, encInfo->fptr_stego_image);      // Write encoded bytes
    return e_success;
}

/* Encode secret file content (character by character) */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret);                                // Move to start of secret file
    char image_buffer[8], ch;

    for (int i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(&ch, 1, 1, encInfo->fptr_secret);                  // Read 1 byte from secret file
        fread(image_buffer, 1, 8, encInfo->fptr_src_image);      // Read 8 bytes from image
        encode_byte_to_lsb(ch, image_buffer);                    // Encode that byte
        fwrite(image_buffer, 1, 8, encInfo->fptr_stego_image);   // Write encoded bytes
    }

    return e_success;
}

/* Copy remaining image data after encoding is done */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) > 0)                      // Copy byte by byte till EOF
    {                       
        fwrite(&ch, 1, 1, fptr_dest);
    }

    // Verify same file size after copy
    if (ftell(fptr_src) == ftell(fptr_dest))
        return e_success;
    else
        return e_failure;
}

/* Perform the encoding */
Status do_encoding(EncodeInfo *encInfo)
{
    //Open all required files
    if (open_files(encInfo) == e_failure)
    {
        return e_failure;
    }

    //Check if image has enough capacity
    if (check_capacity(encInfo) == e_failure)
    {
        return e_failure;
    }

    //Copy 54-byte BMP header
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    //Encode magic string to mark image as stego
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        return e_failure;
    }

    //Encode file extension size (.txt -> 4)
    long extn_size = strlen(encInfo->extn_secret_file);
    if (encode_secret_extn_file_size(extn_size, encInfo) == e_failure)
    {
        return e_failure;
    }

    //Encode file extension characters
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        return e_failure;
    }

    //Encode secret file size (in bytes)
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        return e_failure;
    }

    //Encode secret file data
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        return e_failure;
    }

    //Copy remaining image data to stego file
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    //Encoding successful
    return e_success;
}
 

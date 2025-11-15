#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

 /* Check operation type */
OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
        return e_encode;
    else if (strcmp(argv[1], "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    OperationType op_type;

    if (argc < 3)
    {
        printf("Error: Pass the valid arguments\n");
        printf("Usage:\n");
        printf("Encoding: ./a.out -e <source.bmp> <secret.txt> <stego.bmp>\n");
        printf("Decoding: ./a.out -d <stego.bmp> <output.txt>\n");
        return 1;
    }

    op_type = check_operation_type(argv);

    if (op_type == e_encode)
    {
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            if (do_encoding(&encInfo) == e_success)
                printf("INFO: Encoding completed successfully.\n");
            else
                printf("ERROR: Encoding failed.\n");
        }
        else
        {
            printf("ERROR: Validation failed.\n");
        }
    }
    else if (op_type == e_decode)
    {
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            if (do_decoding(&decInfo) == e_success)
                printf("INFO: Decoding completed successfully.\n");
            else
                printf("ERROR: Decoding failed.\n");
        }
        else
        {
            printf("ERROR: Validation failed.\n");
        }
    }
    else
    {
        printf("ERROR: Unsupported operation.\n");
    }

    return 0;
}

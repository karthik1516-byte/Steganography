#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char **argv)
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    if(argc < 3)
    {
        printf("Error:Invalid no. of command line arguments\n");
        printf("For encoding: ./a.out -e <sourcefile.bmp> <secretfile.txt> [outputfile.bmp]\n");
        printf("For decoding: ./a.out -d <sourcefile.bmp> [outputfile.txt]\n");
        return 1;
    }
    if(check_operation_type(argv) == e_encode)
    {
        if(argc < 4)
        {
            printf("For encoding: ./a.out -e <sourcefile.bmp> <secretfile.txt> [outputfile.bmp]\n");
            return 1;
        }
        printf("-------Encoding operation started----------\n");
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Read and validate encode arguments is successfull\n");
            if(do_encoding(&encInfo) == e_success)
                printf("-------------Encoding completed successfully---------------\n");
            else
            {
                printf("Error:Encoding failed\n");
                return e_failure;
            }
        }
        return e_success;
    }
    else if(check_operation_type(argv) == e_decode)
    {
        printf("------Decoding operation started---------\n");
        if(read_and_validate_decode_args(argv, &decInfo) == d_success)
        {
            printf("Read and validate decode arguments is successfull\n");
            if(do_decoding(&decInfo) == d_success)
                printf("----------------Decoding completed successfully-----------------\n"); 
            else
            {
                printf("Error:Decoding failed\n");
                return d_failure;
            }
        }
        return d_success;
    }
    else
    {
        printf("Error: 1st command line argument must be either '-e' or '-d'\n");
        printf("For encoding: ./a.out -e <sourcefile.bmp> <secretfile.txt> [outputfile.bmp]\n");
        printf("For decoding: ./a.out -d <sourcefile.bmp> [outputfile.txt]\n");
    }
    return 0;
}

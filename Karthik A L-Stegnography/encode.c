#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */
OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1],"-e") == 0)
        return e_encode;
    else if(strcmp(argv[1],"-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(strstr(argv[2],".bmp") != NULL) 
    {
        encInfo->src_image_fname = argv[2];
    }  
    else
    {
        printf("Error: In checking source file extension .bmp\n");
        return e_failure;
    }

    if(strstr(argv[3],".txt") != NULL)  
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        printf("Error: In checking source file extension .txt\n");
        return e_failure;
    }

    if(argv[4] != NULL)  
    {
        if(strstr(argv[4],".bmp") != NULL)
        {
            encInfo->stego_image_fname = argv[4]; 
            printf("File created : %s\n",encInfo->stego_image_fname);
        }
        else
            return e_failure;
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp"; 
        printf("The default file name : %s is created\n",encInfo->stego_image_fname);
    }
    return e_success;
}

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
    printf("width = %u\n", width);

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
Status open_encode_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

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
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
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

uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    return ftell(fptr); 
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image); 
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);           
    int total_bits = (strlen(MAGIC_STRING) + 4 + strlen(encInfo->extn_secret_file) + 4 + encInfo->size_secret_file)*8 + 54;
    if(encInfo->image_capacity < total_bits)  
    {
        printf("Error:image capacity is not sufficient to hide the secret data\n");
        return e_failure;
    }
    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image); 
    char buffer[54];        
    fread(buffer, 54, 1, fptr_src_image); 
    fwrite(buffer, 54, 1, fptr_dest_image); 
    return e_success;
}

// Encodes the magic string into the stego image.
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image((char *)magic_string, strlen(MAGIC_STRING), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char buffer[8];
    for(int i = 0; i < size; i++)
    {
        fread(buffer, 8, 1, fptr_src_image); 
        encode_byte_to_lsb(data[i], buffer); 
        fwrite(buffer, 8, 1, fptr_stego_image); 
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++)
    {
        image_buffer[i] = ((data >> (7-i)) & 1) | (image_buffer[i] & (0xFE));
    }
    return e_success;
}

Status encode_secret_file_extn_size(long extn_size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(extn_size, buffer); 
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_size_to_lsb(long size, char *image_buffer)
{
    for(int i = 0; i < 32; i++)
    {
        image_buffer[i] = (1 & (size >> (31-i))) | (image_buffer[i] & (0xFE));
    }
    return e_success;
}


Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image((char *)file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, buffer);
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret); 
    char secret_data[encInfo->size_secret_file];
    fread(secret_data, encInfo->size_secret_file, 1, encInfo->fptr_secret); 
    encode_data_to_image(secret_data, encInfo->size_secret_file, encInfo->fptr_src_image, encInfo->fptr_stego_image);
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch, 1, 1, fptr_src) != 0)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if(open_encode_files(encInfo) == e_success)
        printf("file opened successfully\n");
    else
    {
        printf("Error:failed to open files\n");
        return e_failure;
    }
    if(check_capacity(encInfo) == e_success)
        printf("Check capacity: Sufficient space is available\n"); 
    else
    {
        printf("Error:not sufficient capacity\n");
        return e_failure;
    }
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
        printf("Copied bmp header successfully\n");
    else
    {
        printf("Error:failed to copy the .bmp header\n");
        return e_failure;
    }
    if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
        printf("magic string encoded successfully\n");
    else
    {
        printf("Error:failed to encode magic string\n");
        return e_failure;
    }
    strcpy(encInfo->extn_secret_file,strstr(encInfo->secret_fname,"."));
    printf("Extension size: %ld\n", strlen(encInfo->extn_secret_file));
    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_success)
        printf("Secret file extn size encoded successfully\n");
    else
    {
        printf("Error:failed to encode secret file extn size\n");
        return e_failure;
    }
    printf("Extension: %s\n", encInfo->extn_secret_file);
    if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
        printf("Secret file extn encoded successfully\n");
    else
    {
        printf("Error:failed to encode secret fike extn\n");
        return e_failure;
    }
    printf("secret file size: %ld\n", encInfo->size_secret_file);
    if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
        printf("Secret file size is encoded successfully\n");
    else
    {
        printf("Error:failed to encode secret file size\n");
        return e_failure;
    }                             
    if(encode_secret_file_data(encInfo) == e_success)
        printf("Secret file data encoded successfully\n");
    else
    {
        printf("Error:failed to encode secret file data\n");
        return e_failure;
    }
    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image)  == e_success)
        printf("copied remaining data  successfully\n");
    else
    {
        printf("Error:failed to copy remaining data\n");
        return e_failure;
    }
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);
    return e_success;                       
}
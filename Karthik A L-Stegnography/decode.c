#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "types.h"
#include "decode.h"

/*Function Definitions*/

Status_d read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(strstr(argv[2],".bmp") != NULL) 
        decInfo->inp_image_fname = argv[2]; 
    else
    {
        printf("Error: In checking source file extension .bmp\n");
        return d_failure;
    }
    if(argv[3] != NULL) 
        strcpy(decInfo->sec_name,strtok(argv[3], ".")); 
    else
    {
        strcpy(decInfo->sec_name,"decode"); 
        printf("The default filename is : %s\n",decInfo->sec_name);
    }
    return d_success;
}

Status_d open_decode_files(DecodeInfo *decInfo)
{
    // Src Image file
    decInfo->fptr_inp_image = fopen(decInfo->inp_image_fname, "r");
    
    if (decInfo->fptr_inp_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->inp_image_fname);

    	return d_failure;
    }
    return d_success;
}

Status_d decode_magic_string(DecodeInfo *decInfo)
{
    char magic_string[strlen(MAGIC_STRING) + 1];
    fseek(decInfo->fptr_inp_image, 54, SEEK_SET); 
    decode_image_to_data(strlen(MAGIC_STRING), magic_string, decInfo);
    magic_string[strlen(MAGIC_STRING)] = '\0';
    if (strcmp(magic_string, MAGIC_STRING) != 0) 
    {
        printf("Error: magic string mismatch\n");
        return d_failure;
    }
    printf("MAGIC STRING : %s\n",magic_string);
    return d_success;
}

Status_d decode_image_to_data(int size,char *data, DecodeInfo *decInfo)
{
    char buffer[8];
    for(int i = 0; i < size; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_inp_image);
        decode_lsb_to_byte(&data[i], buffer); 
    }
    return d_success;
}

Status_d decode_lsb_to_byte(unsigned char *data, char *image_buffer)
{
    *data = 0;
    for(int i = 0; i < 8; i++)
    {
        *data = *data | (image_buffer[i] & 1) << (7-i);
    }
    return d_success;
}

Status_d decode_lsb_to_size(long *data, char *image_buffer)
{
    *data = 0;
    for(int i = 0; i < 32; i++)
    {
        *data |= (image_buffer[i] & 1) << (31-i);
    }
    return d_success;
}

Status_d decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer,32,1,decInfo->fptr_inp_image);
    long  extn_size;
    decode_lsb_to_size(&extn_size,buffer); 
    decInfo->extn_secret_file_size = extn_size;
    printf("extn size:%ld\n",extn_size);
    return d_success;
}

Status_d decode_secret_file_extn(DecodeInfo *decInfo)
{
    char str[decInfo->extn_secret_file_size + 1];
    decode_image_to_data(decInfo->extn_secret_file_size,str, decInfo); 
    str[decInfo->extn_secret_file_size] = '\0';
    printf("Extn : %s\n",str);
    strcat(decInfo->sec_name, str); 
    
    // Open the decoded secret file in write mode.
    decInfo->fptr_decode = fopen(decInfo->sec_name, "w");
    if(decInfo->fptr_decode == NULL)
    {
        perror("fopen");
        fprintf(stderr,"ERROR: Unable to open file %s\n",decInfo->sec_name);
        return d_failure;
    }
    printf("the file name: %s is created\n",decInfo->sec_name);
    return d_success;
}

Status_d decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, 32, 1, decInfo->fptr_inp_image);
    decode_lsb_to_size(&decInfo->size_secret_file,buffer); 
    return d_success;
}

Status_d decode_secret_file_data(DecodeInfo *decInfo)
{
    char data[decInfo->size_secret_file + 1];
    decode_image_to_data(decInfo->size_secret_file,data,decInfo);
    data[decInfo->size_secret_file] = '\0';
    fwrite(data,decInfo->size_secret_file,1, decInfo->fptr_decode);
    return d_success;
}

Status_d do_decoding(DecodeInfo *decInfo)
{
    if(open_decode_files(decInfo) == d_success)
        printf("file opened successfully\n");
    else
    {
        printf("Error:failed to open files\n");
        return d_failure;
    }
    if(decode_magic_string(decInfo) == d_success)
        printf("magic string decoded successfully\n");
    else
    {
        printf("Error:failed to decode magic string\n");
        return d_failure;
    }
    if(decode_secret_file_extn_size(decInfo) == d_success)
        printf("Secret file extn size decoded successfully\n");
    else
    {
        printf("Error:failed to decode secret file extn size\n");
        return d_failure;
    }
    if(decode_secret_file_extn(decInfo) == d_success)
        printf("Secret file extn decoded successfully\n");
    else
    {
        printf("Error:failed to decode secret file extn\n");
        return d_failure;
    }
    if(decode_secret_file_size(decInfo) == d_success)
        printf("Secret file size is decoded successfully\n"); 
    else
    {
        printf("Error:failed to decode secret file size\n");
        return d_failure;
    }
    if(decode_secret_file_data(decInfo) == d_success)
        printf("Secret file data decoded successfully\n");
    else
    {
        printf("Error:failed to decode secret file data\n");
        return d_failure;
    }
    fclose(decInfo->fptr_inp_image);
    fclose(decInfo->fptr_decode);
    return d_success;                       
}
#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * decoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *inp_image_fname;
    FILE *fptr_inp_image;

    /* Decode File Info */
    char sec_name[20];
    FILE *fptr_decode;
    char extn_secret_file[MAX_FILE_SUFFIX];
    long extn_secret_file_size;
    long size_secret_file;

} DecodeInfo;


/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status_d read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status_d do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status_d open_decode_files(DecodeInfo *decInfo);

/* Store Magic String */
Status_d decode_magic_string(DecodeInfo *decInfo);

/*decode secret file extension size*/
Status_d decode_secret_file_extn_size(DecodeInfo *decInfo);

/* decode secret file extenstion */
Status_d decode_secret_file_extn(DecodeInfo *decInfo);

/* decode secret file size */
Status_d decode_secret_file_size(DecodeInfo *decInfo);

/* decode secret file data*/
Status_d decode_secret_file_data(DecodeInfo *decInfo);

/* decode function, which does the real decoding */
Status_d decode_image_to_data(int size, char *data,DecodeInfo *decInfo);

/* decode a LSB into byte of image data array */
Status_d decode_lsb_to_byte(unsigned char *data, char *image_buffer);

/* decode a LSB into size of image data array */
Status_d decode_lsb_to_size(long *data, char *image_buffer);

#endif
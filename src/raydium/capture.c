/*
    Raydium - CQFD Corp.
    http://raydium.org/
    License: GPL - GNU General Public License, see "gpl.txt" file.
*/

#ifndef DONT_INCLUDE_HEADERS
#include "index.h"
#else
#include "headers/capture.h"
#endif 

// Present capture code was inspired from various web files...
// Not a pure CQFD Corp. production :)

// proto
FILE *raydium_file_fopen(char *file, char *mode);

void raydium_capture_frame(char *filename)
{
raydium_capture_asked=RAYDIUM_CAPTURE_TGA;
strcpy(raydium_capture_filename,filename);
}

void raydium_capture_frame_now(char *filename)
{
unsigned char cGarbage = 0, type,mode,aux,bpp, *imageData;
short int iGarbage = 0;
GLuint i;
GLuint size;
FILE *file;

mode=3; // RGB (Bpp)
bpp=mode*8;
type=2; // Color


size=raydium_window_tx * raydium_window_ty * mode;
imageData=malloc(size+1);

glReadPixels(0, 0,raydium_window_tx,raydium_window_ty,GL_RGB,GL_UNSIGNED_BYTE,imageData);


// open file and check for errors
file = raydium_file_fopen(filename, "wb");
if (file == NULL) { raydium_log("Error: capture: cannot open %s for writing",filename); return; }

// write the header
fwrite(&cGarbage, sizeof(unsigned char), 1, file);
fwrite(&cGarbage, sizeof(unsigned char), 1, file);
fwrite(&type, sizeof(unsigned char), 1, file);
fwrite(&iGarbage, sizeof(short int), 1, file);
fwrite(&iGarbage, sizeof(short int), 1, file);
fwrite(&cGarbage, sizeof(unsigned char), 1, file);
fwrite(&iGarbage, sizeof(short int), 1, file);
fwrite(&iGarbage, sizeof(short int), 1, file);
fwrite(&raydium_window_tx, sizeof(short int), 1, file);
fwrite(&raydium_window_ty, sizeof(short int), 1, file);
fwrite(&bpp, sizeof(unsigned char), 1, file);
fwrite(&cGarbage, sizeof(unsigned char), 1, file);

// convert the image data from RGB(a) to BGR(A)
    for (i=0; i < size ; i+= mode) 
    {
    aux = imageData[i];
    imageData[i] = imageData[i+2];
    imageData[i+2] = aux;
    }

    // save the image data
    fwrite(imageData, sizeof(unsigned char), size, file);
    fclose(file);
    // release the memory
    free(imageData);
    raydium_log("screenshot saved as %s",filename);
}

void raydium_capture_frame_jpeg(char *filename)
{
raydium_capture_asked=RAYDIUM_CAPTURE_JPG;
strcpy(raydium_capture_filename,filename);
}

void raydium_capture_frame_jpeg_now(char *filename)
{
JSAMPLE * image_buffer;	/* Points to large array of R,G,B-order data */
int image_height;	/* Number of rows in image */
int image_width;		/* Number of columns in image */
int quality;

struct jpeg_compress_struct cinfo;
struct jpeg_error_mgr jerr;
FILE * outfile;		/* target file */
JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
int row_stride;		/* physical row width in image buffer */
int size;

cinfo.err = jpeg_std_error(&jerr);
jpeg_create_compress(&cinfo);

outfile = raydium_file_fopen(filename, "wb");
if (outfile == NULL) { raydium_log("Error: capture: cannot open %s for writing",filename); return; }

size=raydium_window_tx * raydium_window_ty * 3;
image_buffer=malloc(size+1);

glReadPixels(0, 0,raydium_window_tx,raydium_window_ty,GL_RGB,GL_UNSIGNED_BYTE,image_buffer);
image_width=raydium_window_tx;
image_height=raydium_window_ty;
quality=DEBUG_JPEG_QUALITY;

jpeg_stdio_dest(&cinfo, outfile);

cinfo.image_width = image_width; 	/* image width and height, in pixels */
cinfo.image_height = image_height;
cinfo.input_components = 3;		/* # of color components per pixel */
cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */

jpeg_set_defaults(&cinfo);
jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

jpeg_start_compress(&cinfo, TRUE);

row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

while (cinfo.next_scanline < cinfo.image_height) 
    {
    row_pointer[0] = & image_buffer[(cinfo.image_height-cinfo.next_scanline-1) * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }


jpeg_finish_compress(&cinfo);
fclose(outfile);

jpeg_destroy_compress(&cinfo);
free(image_buffer);
}

void raydium_capture_filename_auto(char *dest,char *format)
{
static int cpt=0;
time_t rawtime;
struct tm *ptm;
    
time(&rawtime);
ptm=gmtime(&rawtime); // localtime() ?
sprintf(dest,"raycap%i-%02i-%02i-%02i%02i%02i-%02i.%s",
	ptm->tm_year+1900,
	ptm->tm_mon+1,
	ptm->tm_mday,
	ptm->tm_hour,
	ptm->tm_min,
	ptm->tm_sec,
	cpt,
	format);
cpt++;
}

void raydium_capture_frame_auto(void)
{

char f[RAYDIUM_MAX_NAME_LEN];
raydium_capture_filename_auto(f,"tga");
raydium_capture_frame(f);
}

void raydium_capture_frame_jpeg_auto(void)
{
char f[RAYDIUM_MAX_NAME_LEN];
raydium_capture_filename_auto(f,"jpg");
raydium_capture_frame_jpeg(f);
}

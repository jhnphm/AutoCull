#include <stdio.h>
#include <time.h>
#include <libraw/libraw.h>
#include <libexif/exif-loader.h>
//#include <getopt.h>

struct burst{
    char **names;
    int count;
    int best_index;
} burst_t;


int main(int argc, char **argv){
    int retval;
    int burst_count;
    burst_t *burst_list = malloc(argc*sizeof(burst_t));
    libraw_data_t *libraw = libraw_init(0);
    for(int i = 1; i < argc; i++){

        /* Load raw thumb */{
            if (libraw_open_file(libraw, argv[i])){
                goto libraw_error;
            }
            if (libraw_unpack(libraw)){
                goto libraw_error;
            }
            if (libraw_unpack_thumb(libraw)){
                goto libraw_error;
            }
        }
        
        
        /* Exif stuff */{
            ExifLoader *exif_l = exif_loader_new();
            ExifData *exif_d;
            ExifEntry *exif_e;
//            if(exif_l == NULL){
//                goto exif_error;
//            }
            exif_loader_write(exif_l,libraw->thumbnail.thumb, libraw->thumbnail.tlength);
            exif_d = exif_loader_get_data(exif_l);
            exif_loader_unref(exif_l);
            exif_data_dump(exif_d);
            //exif_e = exif_content_get_entry(exif_d->ifd[EXIF_IFD_EXIF], EXIF_TAG_SUB_SEC_TIME);
            //exif_entry_dump(exif_e,4);
            //exif_entry_unref(exif_e);
            exif_data_unref(exif_d);
        }


        
        printf("%d\n", libraw->other.timestamp);
libraw_error:
        libraw_recycle(libraw);
    }
    libraw_close(libraw);
    return 0;

}

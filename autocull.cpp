#include <iostream>
#include <memory> 
#include <ctime>
#include <string.h>
//#pragma GCC diagnostic ignored "-Wfloat-equal".
#include <exiv2/exiv2.hpp>
#include <jpeglib.h>

#define Y 0
#define CB 1
#define CR 2


using namespace std;

class Burst{
    private:
        vector<string> burst_files;
        string best;
    public:
        void addFileName(const string& filename);
        std::string getBest();
};

void Burst::addFileName(const string& filename){
    burst_files.push_back(filename);
}

string Burst::getBest(){
    for(auto& filename: burst_files){
        cout << filename << ": ";

        //Get preview image 
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename);
        image->readMetadata();
        Exiv2::PreviewManager preview_m = Exiv2::PreviewManager(*image);
        Exiv2::PreviewPropertiesList preview_pl = preview_m.getPreviewProperties();
        /* Decompress jpeg and get average dct coefficient */
        if(!preview_pl.empty()){
            Exiv2::PreviewImage pimage = preview_m.getPreviewImage(preview_pl.back());
            int sum_coeff[DCTSIZE2];
            struct jpeg_decompress_struct cinfo;
            struct jpeg_error_mgr jerr;

            //Initialize jpeg stuff
            cinfo.err = jpeg_std_error(&jerr);
            jpeg_create_decompress(&cinfo);
            jpeg_mem_src(&cinfo, (unsigned char *)pimage.pData(), pimage.size());
            jpeg_read_header(&cinfo, TRUE);

            //Read out coefficients
            jvirt_barray_ptr *coeffs = jpeg_read_coefficients(&cinfo);

            for(JDIMENSION i = 0; i < DCTSIZE2; i++){
                sum_coeff[i] = 0;
            }

            //Sum up all block coefficients
            for(JDIMENSION i = 0; i < cinfo.comp_info[Y].height_in_blocks; i++){
                JBLOCKARRAY blockrow = (cinfo.mem->access_virt_barray)(
                        (j_common_ptr)&cinfo, coeffs[Y], 0,
                        (JDIMENSION)1,
                        FALSE
                        );
                for(JDIMENSION j = 0; j < cinfo.comp_info[Y].width_in_blocks; j++){
                    for(JDIMENSION k = 0; k < DCTSIZE2; k++){
                        sum_coeff[k] += abs(blockrow[i][j][k]);
                    }
                }
            }
            jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);


            for(JDIMENSION i = 0; i < DCTSIZE2-1; i++){
                cout << sum_coeff[i] << ", ";
            }
            cout << sum_coeff[DCTSIZE2-1] << endl;
        }

    }
    return "";
}



int main(int argc, char **argv){
    vector<Burst> bursts = vector<Burst>();
    long lasttime = 0;
    for(int i = 1; i < argc; i++){
        string datetime;
        long time;
        struct tm tm;

        //Open file
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[i]);
        image->readMetadata();
        Exiv2::ExifData &exif_data = image->exifData();

        //Get datetime and convert to long
        datetime = exif_data.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal"))->toString();
        strptime(datetime.c_str(),"%Y:%m:%d %H:%M:%S",&tm);
        time = (long)mktime(&tm);

        //Multiply by 100 and add subsecs
        time *= 100;
        time += stoi(exif_data.findKey(Exiv2::ExifKey("Exif.Photo.SubSecTimeOriginal"))->toString());

        if(time - lasttime > 50){
            bursts.emplace_back();
        }
        bursts.back().addFileName(argv[i]);  
        lasttime = time;
    }

    for(auto& i: bursts){
        cout << "----" << endl;
        i.getBest();
        cout << "----" << endl;
    }
    return 0;

}

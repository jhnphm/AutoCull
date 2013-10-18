#include <iostream>
#include <memory> 
#include <ctime>
#include <string.h>
#include <fstream>
#include <sstream>
//#pragma GCC diagnostic ignored "-Wfloat-equal".
#include <exiv2/exiv2.hpp>
#include <exiv2/xmp.hpp>
#include <jpeglib.h>

#define Y 0
#define CB 1
#define CR 2


using namespace std;

class Burst{/*{{{*/
    private:
        vector<string> burst_files;
        vector<vector<unsigned>> dct_coeff_sums;
        vector<unsigned> hf_energies;
        void getCoefficients(const string& filename);
    public:
        void addFileName(const string& filename);
        const string& getBest();
        const vector<string>& getBurstFiles();
};
/*{{{*/
/**
 * Adds file to burst
 * @param filename Filename to add
 */
void Burst::addFileName(const string& filename){/*{{{*/
    burst_files.push_back(filename);
}/*}}}*/

/**
 * Unzigzags DCT coefficients in block
 * @param sum_coeff_zag Src
 * @param sum_coeff Dest
 */
static void unzig(unsigned *sum_coeff_zag, vector<unsigned>& sum_coeff){/*{{{*/
    JDIMENSION index = 0, zag_c = 0, zag_l = 0;
    bool asc = false, zag_l_asc = true;

    for(JDIMENSION i = 0; i < DCTSIZE2; i++){
        sum_coeff.push_back(sum_coeff_zag[index]);
        zag_c++;
        if(zag_c > zag_l){
            zag_c = 0;
            asc = !asc;
            if(zag_l_asc && zag_l <  DCTSIZE -1){
                zag_l++;
                if(asc){
                    index += 1;
                }else{
                    index += DCTSIZE;
                }
            }else{
                zag_l_asc = false;
                zag_l--;
                if(asc){
                    index += DCTSIZE;
                }else{
                    index += 1;
                }
            }
        }else{
            if(asc){
                index += DCTSIZE-1;
            }else{
                index -= (DCTSIZE-1);
            }
        }
    }
}/*}}}*/

/**
 * Gets sum of DCT coefficients and pushes them into this->dct_coeff_sums
 *
 * TODO Fail gracefully if thumbnail not openable
 * Actually check if thumbnail is available and actually JPEG data
 *
 * @param filename File to extract DCT coefficients
 */
void Burst::getCoefficients(const string& filename){/*{{{*/
    //Get preview image 
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename);
    image->readMetadata();
    Exiv2::PreviewManager preview_m = Exiv2::PreviewManager(*image);
    Exiv2::PreviewPropertiesList preview_pl = preview_m.getPreviewProperties();
    /* Decompress jpeg and get average dct coefficient */
    if(!preview_pl.empty()){
        Exiv2::PreviewImage pimage = preview_m.getPreviewImage(preview_pl.back());
        unsigned sum_coeff_zag[DCTSIZE2];
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
            sum_coeff_zag[i] = 0;
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
                    sum_coeff_zag[k] += abs(blockrow[i][j][k]);
                }
            }
        }
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        /* Unzigzag coefficients */
        dct_coeff_sums.emplace_back();
        unzig(sum_coeff_zag, dct_coeff_sums.back());
    }
}/*}}}*/

/**
 * Returns filename of sharpest image in burst
 * @return Sharpest image
 */
const string& Burst::getBest(){/*{{{*/
    typedef vector<unsigned>::size_type v_sz;
    if(dct_coeff_sums.size() != burst_files.size()){
        for(auto& filename: burst_files){
            getCoefficients(filename);
            // sum hf component energies
            unsigned sum = 0;
            vector<unsigned>& dct_coeffs = dct_coeff_sums.back();
            v_sz size = dct_coeffs.size();
            for(v_sz i = size/2; i < size; i++){
                sum += dct_coeffs[i];
            }
            hf_energies.push_back(sum);
        }
    }
    v_sz best_index = 0;
    for(v_sz i = 0; i < burst_files.size(); i++){
        if(hf_energies[i] > hf_energies[best_index]){
            best_index = i;
        }
    }
    
    return this->burst_files[best_index];
}/*}}}*/

/**
 * Returns list of files in burst
 * @return List of files in burst
 */
const vector<string>& Burst::getBurstFiles(){/*{{{*/
    return this->burst_files;
}/*}}}*//*}}}*/
/*}}}*/

static void write_xmp(const string& filename){
    //const string keyStr = "Xmp.darktable.colorlabels";
    //const string valStr = "0";
    const string keyStr = "Xmp.lr.hierarchicalSubject";
    const string valStr = "autoculled_sharpest";

    Exiv2::XmpProperties::registerNs("http://darktable.sf.net/", "darktable");
    Exiv2::XmpProperties::registerNs("http://ns.adobe.com/lightroom/1.0/", "lr");

    Exiv2::XmpKey  key = Exiv2::XmpKey(keyStr);
    Exiv2::Value::AutoPtr value = Exiv2::Value::create(Exiv2::xmpText);
    Exiv2::XmpData data;
    value->read(valStr);

    /* Read in existing data */{
        ifstream fin;
        stringstream buf;
        fin.open(filename+".xmp",ios::binary);
        buf << fin.rdbuf();
        fin.close();
        Exiv2::XmpParser::decode(data, buf.str());
    }

    /* Exit if xmp exists w/ label */
    for(auto& i: data){
        if(i.key() == keyStr && i.toString() == valStr ){
            return;
        }
    }

    data.add(key,& *value);

    /* Write out data */{
        ofstream fout;
        string buf;
        fout.open(filename+".xmp",ios::binary);
        Exiv2::XmpParser::encode(buf, data);
        fout << buf;
        fout.close();
    }
}

int main(int argc, char **argv){
    vector<Burst> bursts = vector<Burst>();
    long lasttime = 0;
    for(int i = 1; i < argc; i++){
        string datetime;
        long time;
        struct tm tm;

        try{
            //Open file
            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[i]);

            //Skip file if open fails
            if(image.get() == 0){
                continue;
            }

            image->readMetadata();
            Exiv2::ExifData& exif_data = image->exifData();
            
            //Skip if no exif data
            if(exif_data.empty()){
                continue;
            }
                    
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
        }catch(Exiv2::BasicError<char> e){}
    }

    for(auto& i: bursts){
        const string& best = i.getBest();
        for(auto& j: i.getBurstFiles()){
            if(j == best){
                write_xmp(j);
                cout << j << "*" <<endl;
            }else{
                cout << j << endl;
            }
        }
        cout <<endl;
    }
    return 0;

}

// vim:fdm=marker:

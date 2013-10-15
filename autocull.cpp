#include <iostream>
#include <memory> 
#include <ctime>
#include <string.h>
#include <exiv2/exiv2.hpp>

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
    for(auto& i: burst_files){
        cout << i << ":" << endl;
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(i);
        Exiv2::PreviewManager preview_m = Exiv2::PreviewManager(*image);
        //Use libjpeg here to extract dct coefficients
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

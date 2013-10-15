#include <iostream>
#include <ctime>
#include <string.h>
#include <exiv2/exiv2.hpp>

using namespace std;
using namespace Exiv2;


int main(int argc, char **argv){
    long lasttime = 0;
    for(int i = 1; i < argc; i++){
        std::string datetime;
        long time;
        struct tm tm;

        Image::AutoPtr image = ImageFactory::open(argv[i]);
        image->readMetadata();
        ExifData &exifData = image->exifData();
        datetime = exifData.findKey(ExifKey("Exif.Photo.DateTimeOriginal"))->toString();
        strptime(datetime.c_str(),"%Y:%m:%d %H:%M:%S",&tm);
        time = 100*(long)mktime(&tm);
        time += atoi(exifData.findKey(ExifKey("Exif.Photo.SubSecTimeOriginal"))->toString().c_str());
        if(time - lasttime > 50){
            cout << "----------------" << endl;
        }
        lasttime = time;
        cout << time << endl;
    }
    return 0;

}

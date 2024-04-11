#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


typedef struct bitMap{
    unsigned char signature[2];
    unsigned char size[4];
    unsigned char reservedFirst[2];
    unsigned char reservedSecond[2];
    unsigned char offsetOfImage[4];
}bitMapFileHeader;

typedef struct bitMapInfo{
    unsigned char infoSize[4];
    unsigned char width[4];
    unsigned char height[4];
    unsigned char planes[2];
    unsigned char bitCount[2];
    unsigned char compression[4];
    unsigned char imageSize[4];
    unsigned char XpelsPerMeter[4];
    unsigned char YpelsPerMeter[4];
    unsigned char colorsUsed[4];
    unsigned char colorsImportant[4];
}bitMapInfoHeader;

typedef struct p{
    unsigned char rgbRed;
    unsigned char rgbGreen;
    unsigned char rgbBlue;
}pixel;

void toBlack(pixel *p){
    p->rgbBlue = 0;
    p->rgbGreen = 0;
    p->rgbRed = 0;
}

void toWhite(pixel *p){
    p->rgbBlue = 255;
    p->rgbGreen = 255;
    p->rgbRed = 255;
}

int charToInt(unsigned char *str,int size){
    int result=0,j=0;
    for (int i = size-1; i >= 0 ; --i) {
        if(str[i] != '\0'){
            result += (int)str[i]*pow(256,j);
            j++;
        }
    }
    return result;
}

int isAlive(pixel p){
    return (p.rgbBlue == 0 && p.rgbGreen == 0 && p.rgbRed == 0) ? 1 : 0;
}

void pixelCheck(pixel **image,pixel **newImage,int i, int j,int height,int width){//1 1
    int countOfAlive = 0;
    if (isAlive(image[(i+1+height)%height][(j+width)%width])) countOfAlive++;//2 1
    if (isAlive(image[(i+height)%height][(j+1+width)%width])) countOfAlive++;//1 2
    if (isAlive(image[(i+1+height)%height][(j+1+width)%width])) countOfAlive++;//2 2
    if (isAlive(image[(i-1+height)%height][(j+width)%width])) countOfAlive++;//0 1
    if (isAlive(image[(i+height)%height][(j-1+width)%width])) countOfAlive++;//1 0
    if (isAlive(image[(i-1+height)%height][(j-1+width)%width])) countOfAlive++;//0 0
    if (isAlive(image[(i+1+height)%height][(j-1+width)%width])) countOfAlive++;//2 0
    if (isAlive(image[(i-1+height)%height][(j+1+width)%width])) countOfAlive++;//0 2

    if(isAlive(image[i][j])){
        if (!(countOfAlive >= 2 && countOfAlive <=3)){
            toWhite(&newImage[i][j]);
        }else{
            toBlack(&newImage[i][j]);
        }
    }else{
        if(countOfAlive == 3){
            toBlack(&newImage[i][j]);
        }else{
            toWhite(&newImage[i][j]);
        }
    }
}


void createBmp(bitMapFileHeader *fileHeader,bitMapInfoHeader *infoHeader,pixel **image,
               char *dir,int n,int height,int width){
    char result[100];
    strncpy(result, dir, 8);
    char nameOfFile[100];
    int buf=n,countOfDigit=1;
    while(buf>10){
        buf/=10;
        countOfDigit++;
    }
    char numberOfFile[countOfDigit];
    int i=countOfDigit-1;
    while (n>10){
        numberOfFile[i] = n%10 + '0';
        i--;
        n /= 10;
    }
    numberOfFile[i] = n%10 + '0';
    char s[2]="/";
    char g[11]="generation";
    printf("%s\n", result);
    strcpy(nameOfFile,result);
    strcat(nameOfFile,s);
    strcat(nameOfFile,g);
    strcat(nameOfFile, numberOfFile);
    strcat(nameOfFile,".bmp");
    FILE *newFile = fopen(nameOfFile,"w");
    fwrite(fileHeader, sizeof(bitMapFileHeader),1,newFile);
    fwrite(infoHeader,sizeof(bitMapInfoHeader),1,newFile);
    for (int j = 0; j < height; ++j) {
        for (int k = 0; k < width; ++k) {
            fwrite(&image[j][k], sizeof(pixel),1,newFile);

        }
    }
    fclose(newFile);
}

pixel **emptyBMP(int height,int widht){
    pixel **image = static_cast<pixel **>(malloc(sizeof(pixel *) * height));
    for (int i = 0; i < height; ++i) {
        image[i] = static_cast<pixel *>(malloc(sizeof(pixel) * widht));
    }
    return image;
}

void copyImage(pixel **dist,pixel **source,int height,int width){
    for (int i = 0; i <height ; ++i) {
        for (int j = 0; j < width; ++j) {
            dist[i][j].rgbGreen = source[i][j].rgbGreen;
            dist[i][j].rgbBlue = source[i][j].rgbBlue;
            dist[i][j].rgbRed = source[i][j].rgbRed;
        }
    }
}

int main(int argc, char **argv) {
    if(argc < 5){
        printf("!invalid amount of arguments");
        return 0;
    }
    //printf("%s  %s  %s  %s  %s\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
    long int countOfGenerations=0, frequency=1;
    char nameOfFile[100],nameOfDir[100];
    for (int i = 0; i < argc; ++i) {
        if(strcmp(argv[i],"--input") == 0){
            memcpy(nameOfFile,argv[i+1],strlen(argv[i+1]));
        }
        if(strcmp(argv[i],"--output.txt") == 0){
            memcpy(nameOfDir,argv[i+1],strlen(argv[i+1]));
        }
        if(strcmp(argv[i],"--max_iter") == 0){
            char *end;
            countOfGenerations = strtol(argv[i+1],&end,10);
        }
        if(strcmp(argv[i],"--dump_freq") == 0){
            char *end;
            frequency = strtol(argv[i+1],&end,10);
        }
    }

    char result[100];
    strncpy(result, nameOfFile, 8);

    FILE *file = fopen(result,"rb");
    if(file == NULL){
        printf("!Invalid .bmp file!");
        return 0;
    }
    bitMapFileHeader fileHeader;
    bitMapInfoHeader infoHeader;
    fread(&fileHeader,sizeof(bitMapFileHeader),1,file);
    fread(&infoHeader,sizeof(bitMapInfoHeader),1,file);
    int imageHeight = charToInt(infoHeader.height,4),
            imageWidth = charToInt(infoHeader.width,4);
    printf("%d %d\n", imageHeight, imageWidth);
    pixel **image = static_cast<pixel **>(malloc(sizeof(pixel *) * imageHeight));
    for (int i = 0; i < imageHeight; ++i) {
        image[i] = static_cast<pixel *>(malloc(sizeof(pixel) * imageWidth));
    }
    for (int i = 0; i < imageHeight; ++i) {
        for (int j = 0; j < imageWidth; ++j) {
            fread(&image[i][j], sizeof(pixel),1,file);
            //printf("%d %d %d\n",image[i][j].rgbRed,image[i][j].rgbGreen,image[i][j].rgbBlue);
        }
    }
    fclose(file);
    pixel **newImage = emptyBMP(imageHeight,imageWidth);
    if(countOfGenerations != 0){
        for (int k = 1; k <= countOfGenerations; ++k) {
            for (int i = 0; i < imageHeight; ++i) {
                for (int j = 0; j < imageWidth; ++j) {
                    pixelCheck(image,newImage, i, j, imageHeight, imageWidth);
                }
            }
            copyImage(image,newImage,imageHeight,imageWidth);
            if(k % frequency == 0){
                createBmp(&fileHeader,&infoHeader, image,nameOfDir,k,imageHeight,imageWidth);
            }

        }
    }else{
        int k=0;
        while(true){
            for (int i = 0; i < imageHeight; ++i) {
                for (int j = 0; j < imageWidth; ++j) {
                    pixelCheck(image,newImage,i,j,imageHeight,imageWidth);
                }
            }
            k++;
            copyImage(image,newImage,imageHeight,imageWidth);
            if(k % frequency == 0)
                createBmp(&fileHeader,&infoHeader,image,nameOfDir,k,imageHeight,imageWidth);
        }
    }

    return 0;
}


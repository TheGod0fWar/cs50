#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "helpers.h"

int main(int argc, char *argv[])
{
    // Define allowable filters
    char *filters = "begr";

    // Get filter flag and check validity
    char filter = getopt(argc, argv, filters);
    if (filter == '?')
    {
        printf("Invalid filter.\n");
        return 1;
    }

    // Ensure only one filter
    if (getopt(argc, argv, filters) != -1)
    {
        printf("Only one filter allowed.\n");
        return 2;
    }

    // Ensure proper usage
    if (argc != optind + 2)
    {
        printf("Usage: ./filter [flag] infile outfile\n");
        return 3;
    }

    // Remember filenames
    char *infile = argv[optind];
    char *outfile = argv[optind + 1];

    // Open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 4;
    }

    // Open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        printf("Could not create %s.\n", outfile);
        return 5;
    }

    // Read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // Read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // Ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        printf("Unsupported file format.\n");
        return 6;
    }

    // Get image's dimensions
    int height = abs(bi.biHeight);
    int width = bi.biWidth;

    // Allocate memory for image
    RGBTRIPLE(*image)[width] = calloc(height, width * sizeof(RGBTRIPLE));
    if (image == NULL)
    {
        printf("Not enough memory to store image.\n");
        fclose(outptr);
        fclose(inptr);
        return 7;
    }

    // Determine padding for scanlines
    int padding = (4 - (width * sizeof(RGBTRIPLE)) % 4) % 4;

    // Iterate over infile's scanlines
    for (int i = 0; i < height; i++)
    {
        // Read row into pixel array
        fread(image[i], sizeof(RGBTRIPLE), width, inptr);

        // Skip over padding
        fseek(inptr, padding, SEEK_CUR);
    }

    // Filter image
    switch (filter)
    {
        // Blur
        case 'b':
            blur(height, width, image);
            break;

        // Edges
        case 'e':
            edges(height, width, image);
            break;

        // Grayscale
        case 'g':
            grayscale(height, width, image);
            break;

        // Reflect
        case 'r':
            reflect(height, width, image);
            break;
    }

    // Write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // Write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // Write new pixels to outfile
    for (int i = 0; i < height; i++)
    {
        // Write row to outfile
        fwrite(image[i], sizeof(RGBTRIPLE), width, outptr);

        // Write padding at end of row
        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outptr);
        }
    }

    // Free memory for image
    free(image);

    // Close files
    fclose(inptr);
    fclose(outptr);
    return 0;
}

void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    int h;
    int w;

    for (int h = 0; h < height, h++)
    {
        for (int w = 0; w < width, w++)
        {
            // get arrethmic worth

            const THREECOLOURS = 3;

            int middle = round(( (float) image[h][w].rgbtBlue + image[h][w].rgbtGreen + image[h][w].rgbtRed) / THREECOLOURS);

            image[h][w].rgbtBlue = middle;

            image[h][w].rgbtGreen = middle;

            image[h][w].rgbtRed = middle;
        }
    }




    return;
}

void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    int w;
    int h;



    for (int h = 0; h < height, h++)
    {
        for (int w = 0; i < width/2; w++)

        {
            image[h][i] = image[h][width - i - 1]
        }

    }





    return;
}


void blur(int height, int width, RGBTRIPLE image[height][width])
{   RGBTRIPLE copy[height][width];
    int i;
    int j;
    
    for (int i = 0; i < height; i++ )
    {
     for (int j = 0; j < width; j++) {

      copy[i][j] = image[i][j];
    }
    }
    
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            int sum_red = 0;
            int sum_blue = 0;
            int sum_green = 0;

            int counter = 0;

            for (int h2 = h - 1; h2 <= h + 1; h2++)
            {
                if (h2 < 0 || h2 == height)
                {
                    continue;
                }

                for (int w2 = w - 1; w2 <= w + 1; w2++)
                {
                    if (w2 < 0 || w2 == width)
                    {
                        continue;
                    }

                    sum_red = copy[h2][w2].rgbtRed + sum_red;
                    sum_blue = copy[h2][w2].rgbtBlue + sum_blue;
                    sum_green = copy[h2][w2].rgbtGreen + sum_green;
                    counter ++;
                }
            }



            int middle_red = round((float) sum_red / counter);

            int middle_blue = round((float) sum_blue / counter);

            int middle_green = round((float) sum_green / counter);



            image[h][w].rgbtRed = middle_red;

            image[h][w].rgbtBlue = middle_blue;

            image[h][w].rgbtGreen = middle_green;
        }
    }
    return;
}

void edges(int height, int width, RGBTRIPLE image[height][width])
{
    int xh;
    int xw;
    
    for (int xh = 0; xh < height; xh++) 
    {
        for (int xw = 0; xw < width; xw++) 
        {
        // Hier kommt deine Logik rein, zum Beispiel:
        
        }
    } // Nur zwei schließende Klammern für zwei Schleifen!

    return;
}


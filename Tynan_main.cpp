/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    <Stephen Tynan>

- All project requirements fully met? (YES or NO):
    <YES>

- If no, please explain what you could not get to work:
    <ANSWER>

- Did you do any optional enhancements? If so, please explain:
    <ANSWER>
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


//
// YOUR FUNCTION DEFINITIONS HERE
vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image) // Vignette image
{
    int num_rows =image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            double distance = sqrt(pow((col - num_columns/2),2)+pow((row - num_rows/2),2));
            double scaling_factor = (num_rows - distance)/num_rows;
            new_image[row][col].blue = blue_color*scaling_factor;
            new_image[row][col].red = red_color*scaling_factor;
            new_image[row][col].green = green_color*scaling_factor;    
        }
    }
    return new_image;
    
}

vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double x) //apply claredon effect to image
{
    int num_rows =image.size();
    int num_columns = image[0].size();
    double scaling_factor = x;
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            double average_value = (blue_color+red_color+green_color)/3;
            if (average_value >= 170)
            {
                new_image[row][col].blue = int(255-(255-blue_color)*scaling_factor);
                new_image[row][col].red = int(255-(255-red_color)*scaling_factor);
                new_image[row][col].green = int(255-(255-green_color)*scaling_factor);
            }
            else if (average_value < 90)
            {
                new_image[row][col].blue = blue_color*scaling_factor;
                new_image[row][col].red = red_color*scaling_factor;
                new_image[row][col].green = green_color*scaling_factor;
            }
            else
            {
                new_image[row][col].blue = blue_color;
                new_image[row][col].red = red_color;
                new_image[row][col].green = green_color;
            }
            
        }
    }
    return new_image;
}

vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image) //grayscale image
{
    int num_rows =image.size();
    int num_columns = image[0].size();
    double scaling_factor = 0.3;
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            double gray_value = (red_color+blue_color+green_color)/3;
            new_image[row][col].blue = gray_value;
            new_image[row][col].green = gray_value;
            new_image[row][col].red = gray_value;
            
        }
    }
    return new_image;
}

vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image) // rotate image 90 degrees
{
    int num_rows =image.size();
    int num_columns = image[0].size();
    double scaling_factor = 0.3;
    vector<vector<Pixel>> new_image(num_columns,vector<Pixel> (num_rows));
    
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            new_image [col][num_rows-1-row] = image[row][col];
        }
    }
    return new_image;
}

vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int deg) // rotate by multiples of 90deg
{ 
    int angle = int(deg*90);
    if (angle%90 !=0)
    {
        cout << "angle must be a multiple of 90 degrees." << endl;
        return image;
    }
    else if (angle%360 == 0)
    {
        return image;
    }
    else if (angle%360 ==90)
    {
        return process_4(image);
    }
    else if (angle%360 == 180)
    {
        return process_4(process_4(image));
    }
    else 
    {
        return process_4(process_4(process_4(image)));
    }
}

vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, double xscale, double yscale) // scale image
{
    int num_rows =image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> new_image(num_rows* yscale, vector<Pixel> (num_columns*xscale));
    
    for (int row = 0; row < num_rows*yscale; row++)
    {
        for (int col = 0; col < num_columns*xscale; col++)
        {
            new_image[row][col] = image[int(row/yscale)][int(col/xscale)];
        }
    }
    return new_image;
    
}

vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image) // high contrast
{
    int num_rows =image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            if ((blue_color+red_color+green_color)/3 >= 255/2)
            {
                new_image[row][col].blue = 255;
                new_image[row][col].red = 255;
                new_image[row][col].green = 255;
            }
            else
            {
                new_image[row][col].blue = 0;
                new_image[row][col].red = 0;
                new_image[row][col].green = 0;
            }
        }
    }
    return new_image;
}

vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double x) // lighten image
{
    int num_rows =image.size();
    int num_columns = image[0].size();
    double scaling_factor = x;
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            new_image[row][col].blue = int(255-(255-blue_color)*scaling_factor);
            new_image[row][col].green = int(255-(255-green_color)*scaling_factor);
            new_image[row][col].red = int(255-(255-red_color)*scaling_factor);
        }
    }
    return new_image;
}

vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double x) // darken image
{
    int num_rows =image.size();
    int num_columns = image[0].size();
    double scaling_factor = 0.5;
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            new_image[row][col].blue = blue_color*scaling_factor;
            new_image[row][col].green = green_color*scaling_factor;
            new_image[row][col].red = red_color*scaling_factor;
        }
    }
    return new_image;
}

vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image) // darken image
{
    int num_rows =image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));
    
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int blue_color = image[row][col].blue;
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int max_color = max(blue_color, max(red_color, green_color));
            
            if (red_color + blue_color +green_color >= 550)
            {
                new_image[row][col].blue = 255;
                new_image[row][col].red = 255;
                new_image[row][col].green = 255;
            }
            else if (red_color + blue_color +green_color <= 150)
            {
                new_image[row][col].blue = 0;
                new_image[row][col].red = 0;
                new_image[row][col].green = 0;
            }
            else if (max_color == red_color)
            {
                new_image[row][col].blue = 0;
                new_image[row][col].red = 255;
                new_image[row][col].green = 0;
            }
            else if (max_color == green_color)
            {
                new_image[row][col].blue = 0;
                new_image[row][col].red = 0;
                new_image[row][col].green = 255;
            }
            else
            {
                new_image[row][col].blue = 255;
                new_image[row][col].red = 0;
                new_image[row][col].green = 0;
            }
        }
    }
    return new_image;
}

void Menu (string file_name)
{
    cout << "IMAGE PROCESSING MENU" << endl;
    cout << " 0) Change image (current: ";
    cout << file_name;
    cout << ")" << endl;
    cout << " 1) Vignette" << endl;
    cout << " 2) Claredon" << endl;
    cout << " 3) Grayscale" << endl;
    cout << " 4) Rotate 90 degrees" << endl;
    cout << " 5) Rotate multiples of 90 degrees" << endl;
    cout << " 6) Enlarge" << endl;
    cout << " 7) High contrast" << endl;
    cout << " 8) Lighten" << endl;
    cout << " 9) Darken" << endl;
    cout << " 10) Black, white, red, green, blue" << endl;
    cout << " "<< endl;
    cout << "Enter menu selection (Q to quit): ";
}
//


int main()
{
    
    //
    // YOUR CODE HERE
    //
    string file_name = "";
    cout << "CSPB 1300 Image Processing Application" << endl;
    cout << "Enter input BMP filename: ";
    cin >> file_name;
    double scaling_factor;
    string menu_input= "";
    string output_name = "";
    do
    {
        menu_input= "z";
        Menu(file_name);
        cin >> menu_input;
        int process = 15;
        if (menu_input <"A" || menu_input >"z")
        {
            process = stoi(menu_input);
            if (process <0 || process >10)
            {
                cout << "Enter a valid Number" <<endl;
            }
        }
        else if (menu_input != "Q")
        {
            cout << "Error Enter a Number 0-10 or Q to quit" <<endl;
            process = 20;
        }
            
        switch (process)
        {
            case 0: 
                {
                    cout << "Change image selected" << endl;
                    cout << "Enter new input BMP filename: : ";
                    cin >> file_name;
                    cout << "Successfully changed input image!" << endl;
                    break;
                }
            case 1:
                {
                    cout << "Vignette selected" << endl;
                    cout << "Enter output BMP filename: ";
                    cin >> output_name;
                    if(output_name == file_name)
                    {
                        cout << "Fail, File names are the same" <<endl;
                        break;
                    }
                    vector<vector<Pixel>> image = read_image(file_name);
                    vector<vector<Pixel>> new_image = process_1(image);
                    write_image(output_name, new_image);
                    cout << "Successfully applied vignette!" << endl;
                    break;
                }
            case 2:
                {
                    cout << "Claredon selected" << endl;
                    cout << "Enter output BMP filename: ";
                    cin >> output_name;
                    cout << "Enter scaling factor: ";
                    cin >> scaling_factor;
                    vector<vector<Pixel>> image = read_image(file_name);
                    vector<vector<Pixel>> new_image = process_2(image, scaling_factor);
                    write_image(output_name, new_image);
                    cout << "Successfully applied claredon!" << endl;
                    break;
                }
            case 3:
                {
                    cout << "Grayscale selected" << endl;
                    cout << "Enter output BMP filename: ";
                    cin >> output_name;
                    vector<vector<Pixel>> image = read_image(file_name);
                    vector<vector<Pixel>> new_image = process_3(image);
                    write_image(output_name, new_image);
                    cout << "Successfully applied grayscale!" << endl;
                    break;
                }
            case 4:
                {
                    cout << "Rotate 90 degrees selected" << endl;
                    cout << "Enter output BMP filename: ";
                    cin >> output_name;
                    vector<vector<Pixel>> image = read_image(file_name);
                    vector<vector<Pixel>> new_image = process_4(image);
                    write_image(output_name, new_image);
                    cout << "Successfully applied 90 degree rotation!" << endl;
                    break;
                }
            case 5:
                {
                    cout << "Rotate multiple 90 degrees selected" << endl;
                    cout << "Enter output BMP filename: ";
                    cin >> output_name;
                    int num_rotate;
                    cout << "Enter number of 90 degree rotations: ";
                    cin >> num_rotate;
                    vector<vector<Pixel>> image = read_image(file_name);
                    vector<vector<Pixel>> new_image = process_5(image, num_rotate);
                    write_image(output_name, new_image);
                    cout << "Successfully applied multiple 90 degree rotations!" << endl;
                    break;
                }
            case 6:
                {
                    cout << "Enlarge selected" << endl;
                    cout << "Enter output BMP filename: ";
                    cin >> output_name;
                    double x_scale, y_scale;
                    cout << "Enter X scale: ";
                    cin >> x_scale;
                    cout << "Enter Y scale: ";
                    cin >> y_scale;
                    vector<vector<Pixel>> image = read_image(file_name);
                    vector<vector<Pixel>> new_image = process_6(image, x_scale, y_scale);
                    write_image(output_name, new_image);
                    cout << "Successfully Enlarged!" << endl;
                    break;
                }
            case 7:
                {
                    cout << "High contrast selected" << endl;
                    cout << "Enter output BMP filename: ";
                    cin >> output_name;
                    vector<vector<Pixel>> image = read_image(file_name);
                    vector<vector<Pixel>> new_image = process_7(image);
                    write_image(output_name, new_image);
                    cout << "Successfully applied high contrast!" << endl;
                    break;
                }
             case 8:
                {
                    cout << "Lighten selected" << endl;
                    cout << "Enter output BMP filename: ";
                    cin >> output_name;
                    cout << "Enter scaling factor: ";
                    cin >> scaling_factor;
                    vector<vector<Pixel>> image = read_image(file_name);
                    vector<vector<Pixel>> new_image = process_8(image, scaling_factor);
                    write_image(output_name, new_image);
                    cout << "Successfully lightened!" << endl;
                    break;
                }
            case 9:
                {
                    cout << "Darken selected" << endl;
                    cout << "Enter output BMP filename: ";
                    cin >> output_name;
                    cout << "Enter scaling factor: ";
                    cin >> scaling_factor;
                    vector<vector<Pixel>> image = read_image(file_name);
                    vector<vector<Pixel>> new_image = process_9(image, scaling_factor);
                    write_image(output_name, new_image);
                    cout << "Successfully darkened!" << endl;
                    break;
                }
            case 10:
                {
                    cout << "Black, white, red, green, blue selected" << endl;
                    cout << "Enter output BMP filename: ";
                    cin >> output_name;
                    vector<vector<Pixel>> image = read_image(file_name);
                    vector<vector<Pixel>> new_image = process_10(image);
                    write_image(output_name, new_image);
                    cout << "Successfully applied black, white, red, green, blue filter!" << endl;
                    break;
                }

        }
        
        
        
    }
    while(menu_input != "Q");
    cout << "Thank you for using my Program!" << endl;
    cout << "Quitting..." << endl;
    return 0;
}
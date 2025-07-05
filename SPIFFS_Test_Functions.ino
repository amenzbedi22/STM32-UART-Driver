#include "FS.h"        // Include the File System library
#include "SPIFFS.h"    // Include the SPIFFS library

/* 
   You only need to format SPIFFS the first time you run a test.
   For subsequent use, you can use the SPIFFS plugin to create a partition:
   https://github.com/me-no-dev/arduino-esp32fs-plugin 
*/
#define FORMAT_SPIFFS_IF_FAILED true // Format SPIFFS if mount fails

// Function to list all files and directories in a given directory
// This function recursively lists directories and files up to a specified level.
void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname); // Open the directory
    if(!root) {
        Serial.println("- failed to open directory"); // Error message if directory cannot be opened
        return;
    }
    if(!root.isDirectory()) {
        Serial.println(" - not a directory"); // Error message if the path is not a directory
        return;
    }

    File file = root.openNextFile(); // Open the next file or directory
    while(file) {
        if(file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name()); // Print directory name
            if(levels) {
                listDir(fs, file.path(), levels - 1); // Recursively list contents if levels > 0
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name()); // Print file name
            Serial.print("\tSIZE: ");
            Serial.println(file.size()); // Print file size
        }
        file = root.openNextFile(); // Get next file or directory
    }
}

// Function to read and print the contents of a file
// This function reads a file byte-by-byte and prints its contents to the Serial Monitor.
void readFile(fs::FS &fs, const char * path) {
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path); // Open the file
    if(!file || file.isDirectory()) {
        Serial.println("- failed to open file for reading"); // Error message if file cannot be opened
        return;
    }

    Serial.println("- read from file:");
    while(file.available()) {
        Serial.write(file.read()); // Read and print each byte
    }
    file.close(); // Close the file
}

// Function to write a message to a file, overwriting any existing content
// This function writes a given message to a file, creating or overwriting the file.
void writeFile(fs::FS &fs, const char * path, const char * message) {
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE); // Open the file for writing
    if(!file) {
        Serial.println("- failed to open file for writing"); // Error message if file cannot be opened
        return;
    }
    if(file.print(message)) {
        Serial.println("- file written"); // Success message if file is written
    } else {
        Serial.println("- write failed"); // Error message if writing fails
    }
    file.close(); // Close the file
}

// Function to append a message to a file
// This function appends a given message to the end of a file without overwriting its content.
void appendFile(fs::FS &fs, const char * path, const char * message) {
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND); // Open the file for appending
    if(!file) {
        Serial.println("- failed to open file for appending"); // Error message if file cannot be opened
        return;
    }
    if(file.print(message)) {
        Serial.println("- message appended"); // Success message if message is appended
    } else {
        Serial.println("- append failed"); // Error message if appending fails
    }
    file.close(); // Close the file
}

// Function to rename a file
// This function renames a file from path1 to path2.
void renameFile(fs::FS &fs, const char * path1, const char * path2) {
    Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("- file renamed"); // Success message if file is renamed
    } else {
        Serial.println("- rename failed"); // Error message if renaming fails
    }
}

// Function to delete a file
// This function deletes a file at the specified path.
void deleteFile(fs::FS &fs, const char * path) {
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)) {
        Serial.println("- file deleted"); // Success message if file is deleted
    } else {
        Serial.println("- delete failed"); // Error message if deletion fails
    }
}

// Function to test file I/O operations
// This function performs stress testing by writing and reading large amounts of data to/from a file.
void testFileIO(fs::FS &fs, const char * path) {
    Serial.printf("Testing file I/O with %s\r\n", path);

    static uint8_t buf[512]; // Buffer for file operations
    size_t len = 0; // Length of file
    File file = fs.open(path, FILE_WRITE); // Open file for writing
    if(!file) {
        Serial.println("- failed to open file for writing"); // Error message if file cannot be opened
        return;
    }

    size_t i;
    Serial.print("- writing");
    uint32_t start = millis(); // Start time
    for(i = 0; i < 2048; i++) {
        if ((i & 0x001F) == 0x001F) {
            Serial.print("."); // Print progress dots
        }
        file.write(buf, 512); // Write buffer to file
    }
    Serial.println("");
    uint32_t end = millis() - start; // End time
    Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end); // Print result
    file.close(); // Close the file

    file = fs.open(path); // Open file for reading
    start = millis(); // Start time
    end = start;
    i = 0;
    if(file && !file.isDirectory()) {
        len = file.size(); // Get file size
        size_t flen = len;
        start = millis(); // Start time
        Serial.print("- reading");
        while(len) {
            size_t toRead = len;
            if(toRead > 512) {
                toRead = 512;
            }
            file.read(buf, toRead); // Read data from file
            if ((i++ & 0x001F) == 0x001F) {
                Serial.print("."); // Print progress dots
            }
            len -= toRead;
        }
        Serial.println("");
        end = millis() - start; // End time
        Serial.printf("- %u bytes read in %u ms\r\n", flen, end); // Print result
        file.close(); // Close the file
    } else {
        Serial.println("- failed to open file for reading"); // Error message if file cannot be opened
    }
}

void setup() {
    Serial.begin(115200); // Initialize Serial Monitor
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        Serial.println("SPIFFS Mount Failed"); // Error message if SPIFFS fails to mount
        return;
    }
    
    // Perform various file operations to test functionality
    listDir(SPIFFS, "/", 0); // List all files and directories
    writeFile(SPIFFS, "/hello.txt", "Hello "); // Write to a file
    appendFile(SPIFFS, "/hello.txt", "World!\r\n"); // Append to a file
    readFile(SPIFFS, "/hello.txt"); // Read and print file contents
    renameFile(SPIFFS, "/hello.txt", "/foo.txt"); // Rename the file
    readFile(SPIFFS, "/foo.txt"); // Read and print new file contents
    deleteFile(SPIFFS, "/foo.txt"); // Delete the file
    testFileIO(SPIFFS, "/test.txt"); // Test file I/O operations
    deleteFile(SPIFFS, "/test.txt"); // Delete the test file
    Serial.println("Test complete"); // Print test completion message
}

void loop() {
    // No code in loop
}

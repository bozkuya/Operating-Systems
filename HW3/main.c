//Yasincan Bozkurt
//2304202
//include libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//define char buffer
char buffer[512];
struct FAT
{
    int entry;    
};
// fileList structure for keeping information of files
struct fileList
{
    char fileName[248];
    int first_block;
    int file_size;
};
// Data structure for storing actual data
struct Data
{
    char data[512];    
};
// Function for little endian conversion
int little_endian(int value)
{
    // source https://stackoverflow.com
    return (((value>>24)&0xff) | ((value<<8)&0xff0000) | ((value>>8)&0xff00) | ((value<<24)&0xff000000));
}
// Function to format a file
void Format(char *srcFileName)
{
    // Initialize file pointer and open file in write mode
    FILE *fileptr = fopen(srcFileName, "w+");
    
    // Verify if the file was opened successfully
    if (fileptr == NULL) 
    {
        printf("Error opened file\n");
        exit(1);
    }

    // Prepare FAT and fileList structs for usage
    struct FAT FAT[1] = { {0xFFFFFFFF} };
    struct fileList fileList[1] = { {0}, 0, 0 };

    // Set file position to the beginning and write the first FAT entry
    fseek(fileptr, 0, SEEK_SET);
    fwrite(FAT, sizeof(FAT), 1, fileptr);

    // Prepare the rest of the FAT and fileList entries
    FAT[0].entry = 0x00000000;

    for(int i = 1; i < 4096; i++)
    {
        fwrite(FAT, sizeof(FAT), 1, fileptr);    
    }

    for(int i = 1; i <= 128; i++)
    {
        fwrite(fileList, sizeof(fileList), 1, fileptr);
    }

    // Close the file
    fclose(fileptr);
}
// Function to write data from source file to a virtual disk file
void Write(char *srcFileName, char *source_file, char *destination_file)
{
    // Open source file and virtual disk file
    FILE *fileptr = fopen(srcFileName, "r+");
    if (!fileptr) 
    {
        //error mesage
        printf("Error: %s disk is not found or opened.\n", srcFileName);
        exit(1);
    }
// Create structs for FAT, fileList and Data
    struct FAT FAT[4096];
    struct fileList fileList[128];
    struct Data Data[4096];
    
    FILE *srcptr = fopen(source_file, "r+");

    // Populate buffers
    fread(FAT, sizeof(FAT), 1, fileptr);
    fread(fileList, sizeof(fileList), 1, fileptr);
    fread(Data, sizeof(Data), 1, fileptr);

    int i = 0, j = 0, readsize = 0, prevIndex = 1, endian = 0, count = 0;
    
    while (true)
    {
        // Clear buffer
        memset(&buffer[0], 0, sizeof(buffer));
        readsize = fread(buffer, sizeof(char), sizeof(buffer), srcptr);

        for (i = prevIndex; i < 4096; i++)
        {
            if (FAT[i].entry == 0) break;
        }

        if (i == 4096) 
        {
            printf("No available space\n");
            fclose(fileptr);
            return;
        }

        FAT[i].entry = 0xFFFFFFFF;
        fseek(fileptr, i * sizeof(int), SEEK_SET);
        endian = little_endian(i);

        if (count == 0) 
        {
            // Keep track of the previous index
            fseek(fileptr, 4096 * sizeof(struct FAT) + 128 * sizeof(struct fileList) + i * sizeof(struct Data), SEEK_SET);
            fwrite(&buffer, sizeof(char), readsize, fileptr);

            for (j = 0; j < 128; j++)
            {
                if (fileList[j].first_block == 0 && fileList[j].file_size == 0) break;
            }

            strcpy(&fileList[j].fileName[0], destination_file);
            fileList[j].first_block = i;
        } 
        else 
        {
            FAT[prevIndex].entry = endian;
            fseek(fileptr, 4096 * sizeof(struct FAT) + 128 * sizeof(struct fileList) + i * sizeof(struct Data), SEEK_SET);
            fwrite(&buffer, sizeof(char), readsize, fileptr);
        }

        prevIndex = i;
        count++;

        if (readsize != sizeof(buffer)) break;
    }

    fileList[j].file_size = 512 * (count - 1) + readsize;
    fseek(fileptr, 0, SEEK_SET);
    
    fwrite(FAT, sizeof(FAT), 1, fileptr);
    fwrite(fileList, sizeof(fileList), 1, fileptr);

    fclose(fileptr);
    fclose(srcptr);
}

void Write(char *srcFileName, char *source_file, char *destination_file)
{
    FILE *fileptr = fopen(srcFileName, "r+");
    if (!fileptr) 
    {
        printf("Error: %s disk is not found or opened.\n", srcFileName);
        exit(1);
    }

    struct FAT FAT[4096];
    struct fileList fileList[128];
    struct Data Data[4096];
    
    FILE *srcptr = fopen(source_file, "r+");

    // Populate buffers
    fread(FAT, sizeof(FAT), 1, fileptr);
    fread(fileList, sizeof(fileList), 1, fileptr);
    fread(Data, sizeof(Data), 1, fileptr);

    int i = 0, j = 0, readsize = 0, prevIndex = 1, endian = 0, count = 0;
    
    while (true)
    {
        memset(&buffer[0], 0, sizeof(buffer));
        readsize = fread(buffer, sizeof(char), sizeof(buffer), srcptr);

        for (i = prevIndex; i < 4096; i++)
        {
            if (FAT[i].entry == 0) break;
        }

        if (i == 4096) 
        {
            printf("No available space\n");
            fclose(fileptr);
            return;
        }
             // Set the last FAT entry to the end of file marker
    
        FAT[i].entry = 0xFFFFFFFF;
        fseek(fileptr, i * sizeof(int), SEEK_SET);
        endian = little_endian(i);

        if (count == 0) 
        {
            fseek(fileptr, 4096 * sizeof(struct FAT) + 128 * sizeof(struct fileList) + i * sizeof(struct Data), SEEK_SET);
            fwrite(&buffer, sizeof(char), readsize, fileptr);

            for (j = 0; j < 128; j++)
            {
                if (fileList[j].first_block == 0 && fileList[j].file_size == 0) break;
            }

            strcpy(&fileList[j].fileName[0], destination_file);
            fileList[j].first_block = i;
        } 
        else 
        {
            FAT[prevIndex].entry = endian;
            fseek(fileptr, 4096 * sizeof(struct FAT) + 128 * sizeof(struct fileList) + i * sizeof(struct Data), SEEK_SET);
            fwrite(&buffer, sizeof(char), readsize, fileptr);
        }

        prevIndex = i;
        count++;

        if (readsize != sizeof(buffer)) break;
    }

    fileList[j].file_size = 512 * (count - 1) + readsize;
    fseek(fileptr, 0, SEEK_SET);
    
    fwrite(FAT, sizeof(FAT), 1, fileptr);
    fwrite(fileList, sizeof(fileList), 1, fileptr);

    fclose(fileptr);
    fclose(srcptr);
}

void List(char *srcFileName)
{
    FILE *fileptr = fopen(srcFileName, "r");
    if (!fileptr) 
    {
        printf("Error: Disk %s not opened.\n", srcFileName);
        exit(1);
    }

    struct fileList fileList[128];
    fseek(fileptr, 4096 * sizeof(struct FAT), SEEK_SET);

    // Populate fileList from file
    fread(fileList, sizeof(fileList), 1, fileptr);

    printf("File name\t File size (bytes)\n");

    for (int i = 0; i < 128; i++)
    {
        if (fileList[i].file_size != 0) 
        {
            printf("%s\t%d\n", fileList[i].fileName, fileList[i].file_size);
        }
    }

    fclose(fileptr);
}
void SortA(char *srcFileName)
{
    FILE *fileptr = fopen(srcFileName,"r");
    if(fileptr == NULL)
    {
        printf("Error: %s disk is not opened.\n",srcFileName);
        exit(1);
    }
    struct fileList fileList[128];
    fseek(fileptr, 4096 * sizeof(struct FAT), SEEK_SET);
    fread(fileList,sizeof(fileList),1,fileptr);
    
    // Bubble sort in ascending order
    for (int i = 0; i < 128 - 1; i++) {     
       for (int j = 0; j < 128 - i - 1; j++) {
           if (fileList[j].file_size > fileList[j + 1].file_size) {
              struct fileList temp = fileList[j];
              fileList[j] = fileList[j + 1];
              fileList[j + 1] = temp;
           }
       }
    }

    printf("file name\t file size(bytes)\n");
    for(int i = 0; i < 128; i++)
    {
        if(fileList[i].file_size != 0)
        {
            printf("%s\t%d\n",fileList[i].fileName,fileList[i].file_size);
        }
    }
    fclose(fileptr);
}

void SortD(char *srcFileName)
{
    FILE *fileptr = fopen(srcFileName,"r");
    if(fileptr == NULL)
    {
        printf("Error: %s disk is not opened.\n",srcFileName);
        exit(1);
    }
    struct fileList fileList[128];
    fseek(fileptr, 4096 * sizeof(struct FAT), SEEK_SET);
    fread(fileList,sizeof(fileList),1,fileptr);
    
    // Bubble sort in descending order
    for (int i = 0; i < 128 - 1; i++) {     
       for (int j = 0; j < 128 - i - 1; j++) {
           if (fileList[j].file_size < fileList[j + 1].file_size) {
              struct fileList temp = fileList[j];
              fileList[j] = fileList[j + 1];
              fileList[j + 1] = temp;
           }
       }
    }

    printf("file name\t file size(bytes)\n");
    for(int i = 0; i < 128; i++)
    {
        if(fileList[i].file_size != 0)
        {
            printf("%s\t%d\n",fileList[i].fileName,fileList[i].file_size);
        }
    }
    fclose(fileptr);
}

// function to delete
void Delete(char *srcFileName, char *source_file)
{
    FILE *fileptr = fopen(srcFileName, "r+");

    if (!fileptr) 
    {
        printf("Error: Disk %s not opened.\n", srcFileName);
        exit(1);
    }

    struct fileList fileList[128];
    fseek(fileptr, 4096 * sizeof(struct FAT), SEEK_SET);

    fread(fileList, sizeof(fileList), 1, fileptr);

    int fileIndex;
    for (fileIndex = 0; fileIndex < 128; fileIndex++)
    {
        // Find the index of the file to delete in the file list
        if (strcmp(fileList[fileIndex].fileName, source_file) == 0) 
        {
            break;
        }
    }

    // Check if the file was found
    if (fileIndex == 128) 
    {
        printf("The file %s does not exist.\n", source_file);
        fclose(fileptr);
        return;
    }

    struct FAT FAT[4096];
    struct Data Data[4096];

    fread(Data, sizeof(Data), 1, fileptr);
    fseek(fileptr, 0, SEEK_SET);
    fread(FAT, sizeof(FAT), 1, fileptr);

    // Clear the file list entry for the deleted file
    memset(&fileList[fileIndex].fileName[0], 0, sizeof(fileList[fileIndex].fileName));
    fileList[fileIndex].first_block = 0;
    fileList[fileIndex].file_size = 0;

    int temp, index = fileList[fileIndex].first_block;

    // Clear the data blocks and update the FAT entries
    while (true)
    {
        memset(&Data[index].data[0], 0, sizeof(Data[index].data));

        if (FAT[index].entry == 0xFFFFFFFF) 
        {
            break;
        }
        
        temp = little_endian(FAT[index].entry);
        FAT[index].entry = 0;
        index = temp;
    }

    FAT[index].entry = 0;

    // Write the modified FAT, file list, and data back to the disk image file
    fseek(fileptr, 0, SEEK_SET);
    fwrite(FAT, sizeof(FAT), 1, fileptr);
    fwrite(fileList, sizeof(fileList), 1, fileptr);
    fwrite(Data, sizeof(Data), 1, fileptr);

    fclose(fileptr);
}


void PrintFileList(char *srcFileName)
{
    FILE *fileptr = fopen(srcFileName, "r");

    if (!fileptr) 
    {
        printf("Error: Disk %s is not accessible.\n", srcFileName);
        exit(1);
    }

    FILE *filelisttxt = fopen("filelist.txt", "w+");

    if (!filelisttxt) 
    {
        printf("Creation of filelist.txt failed\n");
        exit(1);
    }

    struct fileList fileList[128];

    fseek(fileptr, 4096 * sizeof(struct FAT), SEEK_SET);
    fread(fileList, sizeof(fileList), 1, fileptr);

    // Write header to filelist.txt
    fseek(filelisttxt, 0, SEEK_SET);
    fprintf(filelisttxt, "Item\tFile name\tFirst block\tFile size(Bytes)\n");

    for (int i = 0; i < 128; i++)
    {
        if (fileList[i].file_size != 0)
        {
            // Write non-empty file entry to filelist.txt
            fprintf(filelisttxt, "%03d\t%s\t%d\t\t%d\n", i, fileList[i].fileName, fileList[i].first_block, fileList[i].file_size);
        }
        else
        {
            // Write empty file entry to filelist.txt
            fprintf(filelisttxt, "%03d\t%s\t\t%d\t\t%d\n", i, "NULL", 0, fileList[i].file_size);
        }
    }

    fclose(fileptr);
    fclose(filelisttxt);
}

void PrintFAT(char *srcFileName)
{
    FILE *fileptr = fopen(srcFileName, "r");

    if (!fileptr) 
    {
        printf("Unable to access disk %s.\n", srcFileName);
        exit(1);
    }

    FILE *fattxt = fopen("fat.txt", "w+");

    if (!fattxt) 
    {
        printf("Creation of fat.txt failed.\n");
        exit(1);
    }

    struct FAT FAT[4096];

    fseek(fileptr, 0, SEEK_SET);
    fread(FAT, sizeof(FAT), 1, fileptr);

    // Write header to fat.txt
    fseek(fattxt, 0, SEEK_SET);
    fprintf(fattxt, "Entry\tValue\t\tEntry\tValue\t\tEntry\tValue\t\tEntry\tValue\n");

    for (int i = 0; i < 4096; i += 4)
    {
        // Write FAT entries to fat.txt
        fprintf(fattxt, "%04d\t%08X\t%04d\t%08X\t%04d\t%08X\t%04d\t%08X\n",
                i, FAT[i].entry, i + 1, FAT[i + 1].entry, i + 2, FAT[i + 2].entry, i + 3, FAT[i + 3].entry);
    }


    fclose(fileptr);
    fclose(fattxt);
}


void Defragment(char *srcFileName)
{
    FILE *fileptr = fopen(srcFileName, "r+");

    if (!fileptr) 
    {
        printf("Failed to open disk: %s.\n", srcFileName);
        exit(1);
    }

    struct FAT FAT[4096], defragmentedFAT[4096];
    struct fileList fileList[128], defragmentedfileList[128];
    struct Data Data[4096], defragmentedData[4096];

    // Read the existing data from the disk image file
    fseek(fileptr, 0, SEEK_SET);
    fread(FAT, sizeof(FAT), 1, fileptr);
    fread(fileList, sizeof(fileList), 1, fileptr);
    fread(Data, sizeof(Data), 1, fileptr);

    int fileLocation = 0;
    int isNewFile = 1;
    int dataIndex = 0;
    int FATindex = 1, lastFATindex = 1;

    // Initialize the defragmented FAT table
    defragmentedFAT[0].entry = 0xFFFFFFFF;

    // Iterate over the file list entries
    for (int i = 0; i < 128; i++) 
    {
        if (fileList[i].first_block != 0 && fileList[i].file_size != 0) 
        {
            // Copy the file list entry to the defragmented file list
            memcpy(&defragmentedfileList[fileLocation], &fileList[i], sizeof(struct fileList));
            defragmentedfileList[fileLocation].first_block = FATindex;
            fileLocation++;

            dataIndex = fileList[i].first_block;

            while (1) 
            {
                // Copy the data block to the defragmented data array
                memcpy(&defragmentedData[FATindex], &Data[dataIndex], sizeof(struct Data));
                
                if (isNewFile == 0) 
                {
                    // Update the FAT entry for the previous block in the defragmented FAT table
                    defragmentedFAT[lastFATindex].entry = little_endian(FATindex);
                }
                else 
                {
                    // Set the FAT entry for the first block of a file to -1 (end of file)
                    defragmentedFAT[lastFATindex].entry = 0xFFFFFFFF;
                    isNewFile = 0;
                }
                
                lastFATindex = FATindex;
                
                if (FAT[dataIndex].entry == 0xFFFFFFFF) 
                {
                    // Update the FAT entry for the last block of a file to -1 (end of file)
                    defragmentedFAT[FATindex].entry = 0xFFFFFFFF;
                    FATindex++;
                    isNewFile = 1;
                    break;
                }
                
                FATindex++;
                dataIndex = little_endian(FAT[dataIndex].entry);
            }
        }
    }

    // If there are no files on the disk, return
    if (fileLocation == 0) 
    {
        fclose(fileptr);
        return;  // Disk is empty
    }

    // Write the defragmented data back to the disk image file
    fseek(fileptr, 0, SEEK_SET);
    fwrite(defragmentedFAT, sizeof(defragmentedFAT), 1, fileptr);
    fwrite(defragmentedfileList, sizeof(defragmentedfileList), 1, fileptr);
    fwrite(defragmentedData, sizeof(defragmentedData), 1, fileptr);
    
    fclose(fileptr);
}

//Maın functıon
int main(int argc, char *argv[])
{
    char *command = argv[2];

    if (strcmp(command, "-format") == 0)
    {
        Format(argv[1]);
    }
    else if (strcmp(command, "-write") == 0)
    {
        Write(argv[1], argv[3], argv[4]);
    }
    else if (strcmp(command, "-read") == 0)
    {
        Read(argv[1], argv[3], argv[4]);
    }
    else if (strcmp(command, "-list") == 0)
    {
        List(argv[1]);
    }
    else if (strcmp(command, "-delete") == 0)
    {
        Delete(argv[1], argv[3]);
    }
    else if (strcmp(command, "-printfilelist") == 0)
    {
        PrintFileList(argv[1]);
    }
    else if (strcmp(command, "-printfat") == 0)
    {
        PrintFAT(argv[1]);
    }
    else if(strcmp(argv[2],"-sorta") == 0)
    {
        SortA(argv[1]);
    }
    
    else if(strcmp(argv[2],"-sortd") == 0){
        SortD(argv[1]);
    }

    else if (strcmp(command, "-defragment") == 0)
    {
        Defragment(argv[1]);
    }
    else
    {
        printf("Request not recognized.\n");
    }

    return 0;
}
//Yasincan Bozkurt
//2304202
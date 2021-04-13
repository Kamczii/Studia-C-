
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>

using namespace std;

struct File {
    unsigned int id;
    size_t size;
};

void swap(File* a, File* b) {
    File temp = *a;
    *a = *b;
    *b = temp;
}

int partId(File arr[], int first, int last) {
    unsigned int pivot = arr[first].id;
    int l_p = first - 1;
    int r_p = last + 1;

    while (true) {
        while (true) {
            l_p++;
            if (arr[l_p].id <= pivot)
                break;
        }
        while (true) {
            r_p--;
            if (arr[r_p].id >= pivot)
                break;
        }
        if (l_p < r_p)
            swap(&arr[l_p], &arr[r_p]);
        else
            return r_p;
    }
}

void quickSortById(File arr[], int first, int last) {

    //dopóki długość większa od jeden
    if (first < last) {
        int partition = partId(arr, first, last);

        quickSortById(arr, first, partition);
        quickSortById(arr, partition + 1, last);
    }


}

int partSize(File**& arr, int first, int last) {
    unsigned int pivot = arr[first]->size;
    int l_p = first - 1;
    int r_p = last + 1;

    while (true) {
        while (true) {
            l_p++;
            if (arr[l_p]->size >= pivot)
                break;
        }
        while (true) {
            r_p--;
            if (arr[r_p]->size <= pivot)
                break;
        }
        if (l_p < r_p)
            swap(arr[l_p], arr[r_p]);
        else
            return r_p;
    }
}

void quickSortBySize(File**& arr, int first, int last) {

    if (first < last) {
        int partition = partSize(arr, first, last);

        quickSortBySize(arr, first, partition);
        quickSortBySize(arr, partition + 1, last);
    }


}

class Vector {
private:
    size_t size, top;
    File* array;

    void resize() {
        size *= 2;
        File* old_array = array;
        if ((array = (File*)realloc(array, size * sizeof(File))) == NULL) {
            free(old_array);
        }

    }

public:
    Vector() {
        top = 0;
        size = 1;
        array = (File*)malloc(sizeof(File) * size);
    }

    Vector(const Vector& other) //Konstruktor kopiujacy
        : size{ other.size }, top{ other.top }, array{ (File*)malloc(sizeof(File) * size) } {
        if(array != nullptr)
            for (size_t i = 0; i < size; i++)
                array[i] = other.array[i];
    }

    Vector(Vector&& other)
    {
        array = other.array;
        size = other.size;
        top = other.top;

        other.array = nullptr;
        other.size = 0;
        other.top = 0;
    }

    Vector& operator=(const Vector& right) {
        Vector tmp = right;
        std::swap(array, tmp.array);
        std::swap(size, tmp.size);
        std::swap(top, tmp.top);
        return *this;
    }

    Vector& operator=(Vector&& right) {
        std::swap(array, right.array);
        std::swap(size, right.size);
        std::swap(top, right.top);
        return *this;
    }

    File& operator[](int i)
    {
        return array[i];
    }
    const File& operator[](int i) const
    {
        return array[i];
    }


    void sort() {
        quickSortById(array, 0, top - 1);
    }

    void push(File& element) {
        if (top >= size)
            resize();
        array[top] = element;
        top++;
    }

    ~Vector() {
        if (array != nullptr) {
            free(array);
        }
    }


};

class Pendrive {
private:
    size_t bytes, maxSize;
    Vector files;
public:
    Pendrive() : bytes(0), maxSize(0) {

    }
    Pendrive(int maxSize) : bytes(0), maxSize(maxSize) {

    }

    //zwraca dodany rozmiar lub 0 jeśli nie dodano
    int addFileToPendrive(File& file) {
        size_t size = file.size;
        if (size <= maxSize - bytes) {
            files.push(file);
            bytes += size;
            return size;
        }
        else
            return -1;
    }

    Vector getFiles() const {
        return files;
    }
};



void PrintResults(const size_t& i, const size_t& j, Pendrive& pendrive_one, Pendrive& pendrive_two)
{
    printf("%zu\n1: ", i + j);

    Vector files1 = pendrive_one.getFiles();
    files1.sort();
    for (size_t k = 0;k < i;k++) {
        printf("%zu ", files1[k].id);
    }

    printf("\n2: ");
    Vector files2 = pendrive_two.getFiles();
    files2.sort();
    for (size_t k = 0;k < j;k++) {
        printf("%zu ", files2[k].id);
    }
}

void FreeMemory(unsigned int n, File** arr)
{
    for (size_t k = 0; k < n;k++) {
        free(arr[k]);
    }
    free(arr);
}

int ReadData(unsigned int& n, unsigned int& p, File**& arr)
{
    int scan = scanf("%u %u", &n, &p);

    if (scan)
        arr = (File**)malloc(n*sizeof(File*));
    else
        return 0;

    if (arr == nullptr)
        return 0;

    for (size_t i = 0; i < n;i++) {
        File* file = (File*)malloc(sizeof(File));
        if (file) {
            int result = scanf("%u %zu", &file->id, &file->size);
            if (result)
                arr[i] = file;
            else {
                FreeMemory(n, arr);
                return 0;
            }
        }
    }
    return 1;
}



int main()
{
 


    unsigned int n, p; //number of files, pendrive's size

    File** arr;


    int read_succeed = ReadData(n, p, arr);

    if (!read_succeed) {
        return 0;
    }


    quickSortBySize(arr, 0, n - 1);


    Pendrive pendrive_one = Pendrive(p);
    Pendrive pendrive_two = Pendrive(p);

    size_t i = 0;
    while ((i < n) && pendrive_one.addFileToPendrive(*arr[i]) > -1) {
        i++;
    }

    size_t j = 0;
    while ((i + j < n) && pendrive_two.addFileToPendrive(*arr[i + j]) > -1) {
        j++;
    }


    FreeMemory(n, arr);

    PrintResults(i, j, pendrive_one, pendrive_two);

}

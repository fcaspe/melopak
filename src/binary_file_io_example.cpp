#include <iostream>
#include <fstream>
#include <vector>

int main()
{
    int nx = 10, ny = 10;

    // buffers for allocation
    std::vector<long double> buff1(nx*ny);
    std::vector<long double> buff2(nx*ny);

    // holds pointers into original
    std::vector<long double*> data(nx);
    std::vector<long double*> data_read(nx);

    for (int i = 0; i < nx; i++)
    {
        data[i] = buff1.data() + (i*ny);
        data_read[i] = buff2.data() + (i*ny);
    }

    data[4][4] = 10.0;
    std::cout << data[4][4] << std::endl;

    std::ofstream ofp("data.bin", std::ios::out | std::ios::binary);
    ofp.write(reinterpret_cast<const char*>(buff1.data()), buff1.size() * sizeof(buff1[0]));
    ofp.close();

    std::ifstream ifp("data.bin", std::ios::in | std::ios::binary);
    ifp.read(reinterpret_cast<char*>(buff2.data()), buff2.size() * sizeof(buff2[0]));
    ifp.close();

    std::cout << data_read[4][4] << std::endl;

    return 0;
}
#include <iostream>
#include <fstream>
#include <vector>
#include "seq_map.hpp"
#include "AudioFile.h"

class wave_packer
    {
    public:
    wave_packer(bool verbose):debug(verbose)
        {
        is_init = false;}
    
    int init(const uint32_t instance_size, const uint32_t out_rate, const char sampling_option,
        const char* wave_file, const char* map_file, const char* pack_file, const char* label_file);
    int pack();
    void print_job_report();
    ~wave_packer()
        {
        if(is_init)
            {
            flabel->close();
            fpack->close();
            if(debug)
                std::cout << "[DEBUG] Files closed." << std::endl;
            }
        }
    private:
    int load_map(const char* in_file);
    int open_pack_file(const char* out_file);
    int open_label_file(const char* out_file);
    bool write_scratchpad();
    std::vector<map_entry_t> seq_map;
    unsigned int sample_rate;
    unsigned int chunk_size;
    char channel_source;
    bool is_init;
    bool debug;

    AudioFile<float> fwave;
    std::ofstream *fpack = NULL;
    std::ofstream *flabel = NULL;
    };
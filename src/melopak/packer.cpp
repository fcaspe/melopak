#include "packer.hpp"
#include <string>
#include <cmath>
#include "csv.h"
#include "slicer_fsm.hpp"

int wave_packer::init(const uint32_t instance_size, const uint32_t out_rate, const char sampling_option,
    const char* wave_file, const char* map_file, const char* pack_file, const char* label_file)
    {

    sample_rate = out_rate;
    chunk_size = instance_size;
    if(sampling_option != 'L' && sampling_option != 'R' && sampling_option != 'M')
        {
        std::cout << "ERROR: Invalid sampling option: "<< sampling_option<< std::endl;
        return -1;             
        }
    channel_source = sampling_option;
    if(load_map(map_file) == -1)
        {
        std::cout << "ERROR: While opening sequence map file: "<< map_file << std::endl;
        return -1;     
        }

    if(open_pack_file(pack_file) == -1)
        {
        std::cout << "ERROR: While opening audio pack file: "<< pack_file << std::endl;
        return -1;
        }

    if(open_label_file(label_file) == -1)
        {
        std::cout << "ERROR: While opening label file: "<< label_file << std::endl;
        return -1;
        }

    fwave.load(wave_file);
    is_init = true;
    return 0;
    }
    
int wave_packer::load_map(const char* in_file)
    {
    std::ifstream fmap;
    fmap.open(in_file, std::ios::in | std::ios::binary);
    if(fmap.is_open() == false)
        return -1;

    fmap.seekg(0, fmap.end);
    int fsize_X8 = fmap.tellg();
    fmap.seekg(0, fmap.beg);
    /*Get vector length*/
    seq_map.resize(fsize_X8/sizeof(map_entry_t));
    /*Read content and dump to vector*/
    fmap.read(reinterpret_cast<char*>(seq_map.data()), fsize_X8);
    fmap.close();
    
    return 0;
    }

int wave_packer::open_pack_file(const char* out_file)
    {
    fpack = new std::ofstream();
    fpack->open(out_file, std::ios::out | std::ios::binary);
    if(fpack->is_open() == false)
        return -1;
    else 
        return 0;
    
    }

int wave_packer::open_label_file(const char* out_file)
    {
    flabel = new std::ofstream();
    flabel->open(out_file, std::ios::out | std::ios::binary);
    if(flabel->is_open() == false)
        return -1;
    else 
        return 0;
    
    }

void wave_packer::print_job_report()
    {
    if(is_init == false)
        {
        std::cout << "ERROR: Packer is not initialized!" <<std::endl;
        return;
        }
    std::cout << std::endl << "JOB REPORT" << std::endl
              << "-----------------------------------" << std::endl;
    
    std::cout << " - " << seq_map.size() <<" audio entries to process" << std::endl;
    std::cout << "   in instances of " << chunk_size <<" samples." << std::endl;
    std::cout <<  " - Resample wave file to " <<sample_rate << " Hz." << std::endl;
    switch(channel_source)
        {
        case 'L':
            std::cout << " - Use the LEFT channel" << std::endl;
        case 'R':
            std::cout << " - Use the RIGHT channel" << std::endl;
        case 'M':
            std::cout << " - Mix both channels onto a single one" << std::endl;
        }
    std::cout << std::endl << "\tWave file info: " <<std::endl;
    
    fwave.printSummary();
    
    std::cout << "-----------------------------------" << std::endl << std::endl;
    }

/** 
    Mix the channels
    Resample -- OK
    And write scratchpad. -- OK
*/
bool wave_packer::write_scratchpad()
    {
    std::ofstream ofp("scratch.pad", std::ios::out | std::ios::binary);
    if(ofp.is_open() == false)
        {
        return false;
        }    
    //int sampleRate = audioFile.getSampleRate();
    //double lengthInSeconds = audioFile.getLengthInSeconds();
    //int numChannels = audioFile.getNumChannels();
    //bool isMono = audioFile.isMono();
    //bool isStereo = audioFile.isStereo();
    const unsigned int numSamples = fwave.getNumSamplesPerChannel();
    const float step = (float)fwave.getSampleRate() / ((float)sample_rate);
    float idx = 0.0f;
    unsigned int i = 0;
    
    int channel = 0;
    bool mix = false;
    if(channel_source != 'M' && fwave.isMono())
        {
        std::cout << "[WARNING] Channel source was set to: " << channel_source << " but wave file is MONO. Using unique channel." << std::endl;
        }
    if(channel_source == 'R' && fwave.isStereo())
        {
        channel = 1;
        }
    if(channel_source == 'M' && fwave.isStereo())
        {
        mix = true;
        }
    while(i < numSamples)
        {
        float sample;
        if(mix == false)
            {
            sample = fwave.samples[channel][i];
            }
        else
            {
            sample = 0.5f*fwave.samples[0][i] + 0.5f*fwave.samples[1][i];
            }
        ofp.write(reinterpret_cast<const char*>(&sample), sizeof(float));
        idx += step;
        i = (unsigned int)ceilf(idx);
        
        }

    ofp.close();
    return true;
    }

int wave_packer::pack()
    {
    unsigned int instances = 0;
    //Step 1: create scratchpad file for storing mixed and resampled data.

    //Step 2: Mix and resample data. Store raw content to scratchpad file.

    /* Let's skip the resampling stage for now. */
    
    if(write_scratchpad() == false)
        {
        std::cout << "ERROR While creating scratchpad file." <<std::endl;
        return instances;
        }   
    /*Open the scratchpad file.*/
    std::ifstream scratchpad("scratch.pad", std::ios::in | std::ios::binary);
    if(scratchpad.is_open() == false)
        {
        std::cout << "ERROR While opening scratchpad file." <<std::endl;
        return instances;
        }    
    /*Create the slicer entries*/
    std::vector<f0_entry_t> slicer_entries;
    float time_acc = 0.0f;
    for(const auto &prog_entry : seq_map)
        {
        f0_entry_t entry;
        entry.f0 = prog_entry.f0;
        time_acc += prog_entry.timestep;
        entry.end_sample = time_acc*sample_rate;
        slicer_entries.push_back(entry);
        }
    if(debug) std::cout << "[DEBUG] Loaded " << slicer_entries.size() << " entries." << std::endl;
    
    SlicerFSM slicer(&scratchpad, //in_audio
                    fpack,        //out_instances
                    flabel,       //out_labels
                    chunk_size,   //instance size
                    16,           //sync window size
                    0.0002f,         //sync threshold
                    0.0002f,         //sync valid
                    debug);       //debug flag
                    
    instances = slicer.process(slicer_entries);
    scratchpad.close();
    std::remove("scratch.pad");
    
    return instances;

    /**/
    


    }
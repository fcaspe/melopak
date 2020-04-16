#include <iostream>
#include <ctime>
#include <chrono>

#include "packer.hpp"
#include "optparse.h"


int main(int argc,char **argv)
    {
    optparse::OptionParser parser =
        optparse::OptionParser().description("Packs the wave and map files into dataset's labels and raw samples.\
\n\nGeneral Instructions: Use 'melogen' to generate the MIDI stream and map file.\
\nRecord the synthesizer output. After the recording is done, use 'melopak'\
\nto process the recording and the map file to generate the dataset.");


    parser.add_option("--len").dest("chunk_size")
          .help("set instance size (in samples)").metavar("SIZE");
    parser.add_option("--rate").dest("sample_rate")
          .help("set target sample rate in Hz").metavar("RATE");
    parser.add_option("--seq").dest("sequence")
          .help("set input path to sequence map").metavar("FILE");
    parser.add_option("--wave").dest("wave")
          .help("set input path to wav file").metavar("FILE");
    parser.add_option("--label").dest("labels")
          .help("set output path to f0 label list (float format)").metavar("FILE");
    parser.add_option("--pack").dest("pack")
          .help("set output path to packed audio data (float format)").metavar("FILE");
    parser.add_option("--ch").dest("channel").set_default("M")
          .help("specify which channel to process. Default is M (mix stereo channels)").metavar("[L,R,M]");
    parser.add_option("-d", "--debug")
          .action("store_true")
          .dest("debug")
          .set_default("0")
          .help("print status messages to stdout");
    
    const optparse::Values options = parser.parse_args(argc, argv);
    const std::vector<std::string> args = parser.args();
    
   if(!(options.is_set("chunk_size") & options.is_set("sample_rate") &
        options.is_set("sequence")   & options.is_set("wave") &
        options.is_set("labels")     & options.is_set("pack")))
        {
        parser.error("Invalid options.");
        return 0;
        }
    
    /*Load options*/
    bool debug = options.get("debug");
    const unsigned int chunk_size = atoi(options["chunk_size"].c_str());
    const unsigned int target_rate = std::atoi(options["sample_rate"].c_str());
    char channel_option = options["channel"].c_str()[0];
    const char* pack_file = options["pack"].c_str();
    const char* label_file = options["labels"].c_str();
    const char* wav_file= options["wave"].c_str();
    const char* map_file = options["sequence"].c_str();
    
    
    auto packer = wave_packer(debug);
    
    int result = packer.init(chunk_size,
                             target_rate,
                             channel_option,
                             wav_file,
                             map_file,
                             pack_file,
                             label_file);
    if(result != 0)
        {
        return -1;
        }
    
    packer.print_job_report();
    
    result = packer.pack();
    if(result == 0)
        {
        std::cout << "ERROR! No saved instances." <<std::endl;
        return 0;
        }    
    
    std::cout << "Done! Saved datset at: " << pack_file << ". Saved labels at: " 
        << label_file << ". Processed " << result << " instances." <<std::endl;
        
    return 0;
    }
#include <iostream>
#include <ctime>
#include <chrono>

#include "sequencer.hpp"
#include "optparse.h"


int main(int argc,char **argv)
    {
    optparse::OptionParser parser =
        optparse::OptionParser().description("Generates an f0-annotated sequence for a MIDI synthesizer and its map file.\
\n\nGeneral Instructions: Use 'melogen' to generate the MIDI stream and map file.\
\nRecord the synthesizer output. After the recording is done, use 'melopak'\
\nto process the recording and the map file to generate the dataset.");

    parser.add_option("-f", "--freq").dest("frequencies")
          .help("set input path to f0 CSV list").metavar("FILE");
    parser.add_option("-p", "--prg").dest("program")
          .help("set input path to MIDI program CSV list").metavar("FILE");
    parser.add_option("-v", "--vel").dest("velocities")
          .help("set input path to MIDI velocities CSV list").metavar("FILE");
    parser.add_option("-s", "--seq").dest("sequence")
          .help("set output path to sequence map").metavar("FILE");
    parser.add_option("-r", "--range").dest("range")
          .help("set synthesizer pitch bend +/- range (in semitones)").metavar("RNG");
    parser.add_option("-d", "--debug")
          .action("store_true")
          .dest("debug")
          .set_default("0")
          .help("print status messages to stdout");
    
    const optparse::Values options = parser.parse_args(argc, argv);
    const std::vector<std::string> args = parser.args();
    
   if(!(options.is_set("frequencies") & options.is_set("program") &
        options.is_set("velocities")  & options.is_set("sequence") & options.is_set("range")))
        {
        parser.error("Invalid options.");
        return 0;
        }
    
    /*Load options*/
    bool debug = options.get("debug");
    const char* f0_file = options["frequencies"].c_str();
    const char* prog_file = options["program"].c_str();
    const char* vel_file = options["velocities"].c_str();
    const char* map_file = options["sequence"].c_str();
    auto const pitch_wheel_range = std::atoi(options["range"].c_str());
    
    auto sequencer = f0_sequencer(debug);
    
    int result = sequencer.init(prog_file,
                                f0_file,
                                vel_file,
                                pitch_wheel_range,
                                map_file);
    if(result != 0)
        {
        return -1;
        }

    sequencer.print_job_report();
    
    std::cout << "OK to start ? (we should hit record at this point) " << std::endl;
    getchar();
    
    auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); 
    std::cout << "Starting job at: " << ctime(&timenow) << std::endl;
    
    sequencer.start_job();
    
    timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); 
    std::cout << "Sequence map saved at: " << map_file << std::endl;
    std::cout << "Finished job at: " << ctime(&timenow) << std::endl;
    
    return 0;
    }
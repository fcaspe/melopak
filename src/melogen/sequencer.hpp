#include <iostream>
#include <fstream>
#include <vector>
#include "midi_output.hpp"
#include "seq_map.hpp"

constexpr uint8_t DEFAULT_SEQ_MIDI_CH = 0;
constexpr float DEFAULT_SEQ_TIMESTEP = 5.0f;

typedef struct
    {
    uint8_t midi_program;
    uint8_t midi_channel;
    float timestep;
    }seq_program_t;

class f0_sequencer
    {
    public:
    f0_sequencer(bool verbose):debug(verbose)
        {
        is_init = false;}
    
    int init(const char* prog_list, const char* f0_list, const char* vel_list, int mod_pitch_range, const char* map_file);
    void print_job_report();
    int start_job();

    ~f0_sequencer()
        {
        if(is_init)
            {
            fmap->close();
            if(debug)
                std::cout << "[DEBUG] Map file closed." << std::endl;
            }
        }
    private:
    void parse_frequencies(const char* in_file);
    void parse_program(const char* in_file);
    void parse_velocities(const char* in_file);
    void f0_2_midi(const float f0_in, uint8_t &note_out,int16_t &pitch_val_out);
    int open_map_file(const char* out_file);
    void seq_wait(float time);
    
    std::vector<float> f0_list;
    std::vector<seq_program_t> prog_list;
    std::vector<uint8_t> vel_list;
    std::vector<map_entry_t> seq_map;
    unsigned int pitch_wheel_range;
    bool is_init;
    bool debug;
    midi_if midi_out;
    /*Defer construction ?*/
    std::ofstream *fmap = NULL;
    };
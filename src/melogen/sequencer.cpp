#include "sequencer.hpp"
#include <string>
#include <cmath>
#include "csv.h"

int f0_sequencer::init(const char* prog_list, const char* f0_list, const char* vel_list, int mod_pitch_range, const char* map_file)
    {

    if(mod_pitch_range < 0)
        {
        std::cout << "ERROR: Enter a positive Mod Wheel +/- range in semitones." << std::endl;
        return -1;
        }
    pitch_wheel_range = mod_pitch_range;
    
    parse_frequencies(f0_list);
    parse_program(prog_list);
    parse_velocities(vel_list);

    if(open_map_file(map_file) == -1)
        {
        std::cout << "ERROR: While opening job map file: "<< map_file << std::endl;
        return -1;
        }
    
    if(midi_out.init() == -1 )
        {
        std::cout << "ERROR: While initializing MIDI output IF." << std::endl;
        return -1;
        }
    
    is_init = true;
    return 0;
    }
    
int f0_sequencer::open_map_file(const char* out_file)
    {
    fmap = new std::ofstream();
    fmap->open(out_file, std::ios::out | std::ios::binary);
    if(fmap->is_open() == false)
        return -1;
    else 
        return 0;
    
    }
void f0_sequencer::parse_frequencies(const char* in_file)
    {
    io::CSVReader<1> in(in_file);
    in.read_header(io::ignore_extra_column, "f0_list");
    double f0_entry;
    while(in.read_row(f0_entry))
        {
        f0_list.push_back(f0_entry);
        }

    }

/**
CSV PARSER: Number of columns is fixed.
*/
void f0_sequencer::parse_program(const char* in_file)
    {
    io::CSVReader<3> in(in_file);
    in.read_header(io::ignore_extra_column, "prog_number", "channel", "timestep(s)");
    seq_program_t prog_entry;
    
    while(in.read_row(prog_entry.midi_program,
                        prog_entry.midi_channel,
                        prog_entry.timestep))
        {
        prog_list.push_back(prog_entry);
        }

    }

void f0_sequencer::parse_velocities(const char* in_file)
    {
    io::CSVReader<1> in(in_file);
    in.read_header(io::ignore_extra_column, "velocity_list");
    uint8_t vel_entry;
    while(in.read_row(vel_entry))
        {
        vel_list.push_back(vel_entry);
        }

    }

void f0_sequencer::print_job_report()
    {
    std::cout << std::endl << "JOB REPORT" << std::endl
              << "------------------------------" << std::endl;
   
    std::cout << "Programs:" << std::endl;
    for(auto prog: prog_list)
        {
        std::cout << " - " << (int)prog.midi_program << " @ ch " << (int)prog.midi_channel << " time: " << prog.timestep << " s." << std::endl;
        }

    
    std::cout << "f0:\n[";
    for(auto freq: f0_list)
        {
        std::cout << freq << ", ";
        }
    std::cout << "]" << std::endl;    
    
    std::cout << "Velocities:\n[";
    for(auto vel: vel_list)
        {
        std::cout << (int)vel << ", ";
        }
    std::cout << "]" << std::endl; 
    
    /*Calculate total job time.*/
    float total_time = 0.0f;
    for(auto prog : prog_list)
        {
        total_time += prog.timestep;
        }
    total_time = total_time * (float)f0_list.size()* (float)vel_list.size() * 2.0f; //Consider both NOTE_ON and NOTE_OFF times.
    const float total_min = floorf(total_time/60.0f);
    std::cout << "------------------------------" << std::endl;
    std::cout << "Total job time: " << total_min  << " min, "
              << total_time - total_min*60.0f << " s." << std::endl;
    std::cout << "------------------------------" << std::endl << std::endl;
    }

void f0_sequencer::f0_2_midi(const float f0_in, uint8_t &note_out,int16_t &pitch_val_out)
    {
	float exact_note = 12*log2f((f0_in)/ 440.0f) + 69;
    //std::cout << "f0 " << f0_in << " note: " << exact_note << std::endl;
    const float rounded_note = roundf(exact_note);
    note_out = (uint8_t)rounded_note;

	const float note_cents = (exact_note -rounded_note);
    //std::cout << "cents: " << note_cents << std::endl;
    float pitch_shift = note_cents/(2.0f*(float)pitch_wheel_range) + 0.5f;
	//std::cout << "pb: " << pitch_shift << std::endl;
    pitch_val_out = (int16_t)(pitch_shift * 16383.f - 8192.0);    
    //std::cout << "pb val out: " << pitch_val_out << std::endl;
    }

int f0_sequencer::start_job()
    {
    /*TODO: GENERATE A SYNC TONE. It is better if we just leave blank space and the packer synchronizes automatically. */
    /*Loop over the sequencer programs.*/
    for(const auto &current_prog : prog_list)
        {
        if(debug)
            {
            std::cout << "[DEBUG] Fetching prog: " <<(int)current_prog.midi_program
                      << " ch: " << (int)current_prog.midi_channel 
                      << " t: " << current_prog.timestep << std::endl;  
            }
        midi_out.send_prog_change(current_prog.midi_program,current_prog.midi_channel);
        /*Loop over the frequencies to synthesize.*/
        for(const auto &current_f0 : f0_list)
            {
            for(const auto &current_vel :vel_list)
                {
                uint8_t note;
                int16_t pitch_bend;
                f0_2_midi(current_f0,note,pitch_bend);
            
                /*Send the NOTE ON message and go to sleep.*/
                midi_out.send_pitch_bend(pitch_bend,current_prog.midi_channel);
                midi_out.send_note(midi_event::NOTE_ON, 
                                  note, 
                                  current_vel,
                                  current_prog.midi_channel);
            
                seq_wait(current_prog.timestep);
            
                /*Send the NOTE OFF message and go to sleep.*/
                midi_out.send_note(midi_event::NOTE_OFF, 
                                  note,
                                  current_vel,
                                  current_prog.midi_channel);
            
                seq_wait(current_prog.timestep);
            
                map_entry_t entry;
                entry.f0 = current_f0;
                entry.timestep = 2.0f * current_prog.timestep;
            
                fmap->write(reinterpret_cast<const char*>(&entry), sizeof(map_entry_t));
                }
            }

        
        }
    return 0;
    }

void f0_sequencer::seq_wait(float time)
    {
    int seconds = (int)floorf(time);
    int nanosecs = (int)((time - floorf(time))*1000000000.0f);
    struct timespec delta = {seconds, nanosecs};
    while (nanosleep(&delta, &delta));
    }
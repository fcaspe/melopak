#include "slicing_window.hpp"
typedef struct
    {
    float f0;
    unsigned int end_sample;
    }f0_entry_t;

class SlicerFSM
    {
    public:
    SlicerFSM(std::ifstream *audio_file,std::ofstream *slices_file,std::ofstream *labels_file,
        unsigned int len, unsigned int sync_len, float th_sync,float th_valid,bool debug_flag):
        slice_len(len),in_audio(audio_file),out_slices(slices_file),out_labels(labels_file),debug(debug_flag)
        {
        audio_idx = 0;
        
        window = new SlicingWindow<float>(len,sync_len,th_valid,th_sync);
        buffer.resize(len);
        }

    unsigned int process(std::vector<f0_entry_t> &entries)
        {
        unsigned int instances = 0;
        /*Initialization of window: We sync always on the oldest samples of the window.
        So we initialize the window by filling it with the first [slice_len] samples.*/

        if(forward(slice_len) == false)
            {
            std::cout << "ERROR! Can't initialize SlicingWindow. " << std::endl;
            return instances;
            }        
        
        /*FSM starts: SYNC*/
        if(sync() == false)
            {
            std::cout << "ERROR! Can't sync after reading " << audio_idx << " samples." << std::endl;
            return instances;
            }

        /* Now we are on sync, so reset internal index to match the f0 entries */
        audio_idx = 0;
        if(debug) std::cout << "[DEBUG] SYNC OK!" << std::endl;
        
        for(const auto &entry : entries)
            {
            /* We don't sync again. Just proceed blindly after the first sync. */
            auto end_mark = entry.end_sample; 
            
            if(debug) 
                {
                std::cout  << "[DEBUG] idx: " << audio_idx                 << "\tf0: "         << entry.f0 
                           << "\tend_sample: "<< entry.end_sample          << "\tis_valid() " << window->is_valid()
                           << "\tboundary() " << check_boundary(end_mark)  << "\tinstances: " << instances << std::endl;
                }
            /* Slice the file while the frames are valid and bounded to the current entry */
            while(window->is_valid() && check_boundary(end_mark))
                {
                write_slice(entry.f0);
                //if(debug) std::cout << "\t Slice Saved! " << std::endl;
                instances++;
                if(forward(audio_idx+slice_len) == false)
                    {
                    std::cout << "[WARNING] SlicerFSM::process() EOF Reached while forwarding to next frame." << std::endl;
                    return instances;
                    }
                if(debug) 
                    {
                    std::cout  << "[DEBUG] idx: " << audio_idx                 << "\tf0: "         << entry.f0 
                               << "\tend_sample: "<< entry.end_sample          << "\tis_valid() " << window->is_valid()
                               << "\tboundary() " << check_boundary(end_mark)  << "\tinstances: " << instances << std::endl;
                    }
                }
            /* At this point we are either out of bounds or dealing with silence. */
            /* Forward to the end mark to process next sample. */
            if(forward(end_mark) == false)
                {
                std::cout << "[WARNING] SlicerFSM::process() EOF Reached while forwarding to next entry." << std::endl;
                return instances;
                }
            }
        
        std::cout << "[INFO] Slicing finished gracefully." << std::endl;
        return instances;
        }
    private:
    /**
    @brief Returns true if we have not exceeded our boundaries.
    */
    inline bool check_boundary(unsigned int mark)
        {
        /* Our index contains the index of the oldest sample.*/
        /* So it can happen that we have already loaded samples at
           a new mark and the index doesn't account for that. */
        return (audio_idx+slice_len < mark);
        
        }
    /**
    @brief Moves the window to the desired mark.
    */    
    inline bool forward(unsigned int mark)
        {
         while(audio_idx < mark)
            {
            /*If we fail while doing a step, then we are at E0F. return false.*/
            if(step() == false)
                {
                return false; //Error state. Means we cannot sync.
                }
            }
        /*If we reach here we are at the mark!. */
        return true;       
        }
    /**
    @brief Writes a instance and a label to output files.
    */      
    inline void write_slice(const float f0_label)
        {
        window->copy_buffer(buffer);
        out_slices->write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(float));
        out_labels->write(reinterpret_cast<const char*>(&f0_label), sizeof(float));
        }
 
    /**
    @brief Syncs the window when it detects energy in the oldest samples.
    */    
    inline bool sync()
        {
        while(window->is_sync() == false)
            {
            /*If we fail while doing a step, then we are at E0F. return false.*/
            if(step() == false)
                {
                return false; //Error state. Means we cannot sync.
                }
            }
        /*If we reach here we are synced!. */
        return true;
        }
    /**
    @brief Pushes one sample to the window.
    */    
    inline bool step()
        {
        if(in_audio->eof() == true)
            {
            if(debug) std::cout << "[DEBUG] SlicerFSM::step() AUDIO EOF Reached" << std::endl;
            return false;
            }
        float sample;
        in_audio->read(reinterpret_cast<char*>(&sample), sizeof(float));
        window->push(sample);
        audio_idx++;
        return true;
        }
    unsigned int audio_idx;
    unsigned int slice_len;
    SlicingWindow<float> *window;
    std::vector<float> buffer;
    std::ifstream *in_audio;
    std::ofstream *out_slices;
    std::ofstream *out_labels;
    bool debug;
    };
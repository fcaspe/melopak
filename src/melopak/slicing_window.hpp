#include <vector>

/**
@brief It is a sliding, syncing and slicing audio window! Implemented around a circular buffer.
*/
template <typename T>
class SlicingWindow
    {
    public:
    SlicingWindow(unsigned int len, unsigned int sync_len,T thresh_valid, T thresh_sync):
        my_sync_len(sync_len),my_len(len),my_th_valid(thresh_valid),my_th_sync(thresh_sync)
        {
        window = new T[my_len];
        head = 0;
        tail = 0;
        //std::cout << "[DEBUG] Created SlicingWindow with sync_th: " << my_th_sync << " valid_th: " << my_th_sync << std::endl;
        }
    inline void push(const T sample)
        {
        head_step();
        window[head] = sample;
        }
    inline unsigned int get_head()
        {
        return head;
        }
    inline unsigned int get_tail()
        {
        return tail;
        }
    /**
    @brief Asserts signal presence in sync area.
    */    
    bool is_sync()
        {
        /*Get the oldest my_sync_len samples.*/
        unsigned int i = tail;
        unsigned int count = 0;
        T acc = (T)(0);
        while(count < my_sync_len)
            {
            //std::cout << "\t[DEBUG] tail: " << tail << " head: " << head << " idx: " << i <<std::endl;
            acc += window[i]*window[i];
            i = step(i);
            count ++;
            }
        acc = acc / (T)my_sync_len;
        //std::cout << "\t[DEBUG] SYNC: Square energy accum: " << acc <<std::endl;
        if(acc < my_th_sync)
            return false;
        else
            return true;
        }
    
    /**
    @brief Asserts valid signal presence in buffer. (It is not silent outside my_sync_len)
    */
    bool is_valid()
        {
        unsigned int i = head;
        T acc = window[i]*window[i];
        do
            {
            i = step_back(i);
            acc += window[i]*window[i];
            }
        while(i != tail);
        
        acc = acc / (T)my_len;
        //std::cout << "\t[DEBUG] VALID: Accumulated: " << acc <<std::endl;
        if(acc < my_th_valid)
            return false;
        else
            return true;
        }
    /**
    @brief Copies all content of window to vector.
        We read the buffer from the newest to the oldest position,
        So we index the vector from the end to the beginning.
    */
    void copy_buffer(std::vector<T> &copy)
        {
        if(copy.size() != my_len)
            {
            std::cout << "ERROR! Size mismatch between buffer and window." <<std::endl;
            return;
            }
        unsigned int i = head; //Buff index
        unsigned int j = my_len-1;//Vector index
        copy[j] = window[i];
        do
            {
            i = step_back(i);
            j--;
            copy[j] = window[i];
            }
        while(i != tail);

        }
    
    private:
    inline void head_step()
        {
        /*Sum the positions*/
        head = (head + 1) % my_len;
        /*If we collide with tail, move tail*/
        if(head == tail)
            {
            tail = (tail + 1) % my_len;
            }
        }
    /**
    @brief To loop around the buffer.
    */
    inline unsigned int step(unsigned int idx)
        {
        return (idx + 1) % my_len;
        }
    
    inline unsigned int step_back(unsigned int idx)
        {
        if(idx == 0)
            {
            return my_len -1;
            }
        else
            {
            return idx -1;
            }
        }
    inline unsigned int size()
        {
        if(head >= tail)
            {
            return head - tail;
            }
        else
            {
            return my_len - tail + head;
            }
        }
    /*Attributes*/
    T* window;
    unsigned int head;
    unsigned int tail;
    unsigned int my_sync_len;
    unsigned int my_len;
    T my_th_valid;
    T my_th_sync;
    };
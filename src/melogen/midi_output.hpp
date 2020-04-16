#include <alsa/asoundlib.h>
#include <unistd.h>
#include <cstdint>

typedef enum:uint8_t
    {
    NOTE_ON = 0,
    NOTE_OFF = 1,
    PITCH_BEND = 2,
    }midi_event;

class midi_if
    {
    public:
    midi_if()
        {}
    int send_note(midi_event evt,uint8_t data1, uint8_t data2, uint8_t channel);
    int send_pitch_bend(int control_value, uint8_t channel);
    int send_prog_change(uint8_t program, uint8_t channel);
    int init(void);
    private:
    void send_event(snd_seq_event_t *ev);
    snd_seq_t *seq_handle;
    int out_port;
    };
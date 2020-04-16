
#include <iostream>
#include <cstdint>
#include "midi_output.hpp"

#define CHK(stmt, msg) if((stmt) < 0) {puts("ERROR: "#msg); return -1;}
int midi_if::init(void)
{
    printf("[DEBUG] midi_open(): Opening sequencer . . .\n");
    CHK(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, SND_SEQ_NONBLOCK),
            "Could not open sequencer");
			
    CHK(snd_seq_set_client_name(seq_handle, "DataGen seq out"),
            "Could not set client name");
    CHK(out_port = snd_seq_create_simple_port(seq_handle, "sender:out",
                SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
                SND_SEQ_PORT_TYPE_APPLICATION),
            "Could not open port");
    printf("[DEBUG] midi_open(): DONE.\n");
    return 0;
}

inline void midi_if::send_event(snd_seq_event_t *ev)
    {
    snd_seq_ev_set_direct(ev);
    snd_seq_ev_set_subs(ev);
    snd_seq_ev_set_source(ev, out_port);
    snd_seq_event_output_direct(seq_handle,ev); 
    }
    

int midi_if::send_note(midi_event evt,uint8_t data1, uint8_t data2, uint8_t channel)
    {
    snd_seq_event_t ev;
    switch(evt)
        {
        case midi_event::NOTE_ON:
            ev.type = SND_SEQ_EVENT_NOTEON;
            ev.data.note.note = data1;
            ev.data.note.velocity = data2;
            ev.data.note.channel = channel; /* 0 is MIDI channel 1 */
            break;
        case midi_event::NOTE_OFF:
            ev.type = SND_SEQ_EVENT_NOTEOFF;
            ev.data.note.note = data1;
            ev.data.note.velocity = data2;
            ev.data.note.channel = channel; /* 0 is MIDI channel 1 */
            break;
        default:
            std::cout << "ERROR: Unrecognized midi event." <<std::endl;
            return -1;
        }
    send_event(&ev);
    return 0;
    }

int midi_if::send_pitch_bend(int control_value, uint8_t channel)
    {
    snd_seq_event_t ev;

    ev.type = SND_SEQ_EVENT_PITCHBEND;
    ev.data.control.value = control_value; /*data from -8192 to 8191*/
    ev.data.control.channel = channel;
    ev.data.control.param = 0; //Tested with VMPK and it sends a param = 0.
    
    send_event(&ev);
    return 0;
    }    

int midi_if::send_prog_change(uint8_t program, uint8_t channel)
    {
    snd_seq_event_t ev;

    ev.type = SND_SEQ_EVENT_PGMCHANGE;
    ev.data.control.value = program; /*data from -8192 to 8191*/
    ev.data.control.channel = channel;
    ev.data.control.param = 0; //Tested with VMPK and it sends a param = 0.
    
    send_event(&ev);
    return 0;
    }    
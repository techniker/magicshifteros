/*
 * MIDIMode.cpp
 *
 * Created: Wed Jul  9 22:19:46 CEST 2014
 *  Author: ibisum  <ibisum@gmail.com
 *
 *  NOTE: camelBack for types and functions, snake_case for vars, SCREAMING_SNAKE_CASE for defs
 *
 */

#include "../firmware.h"
#include "list.h"  // if its good enough for linus, its good enough for the rest of us.


// This is the list abstraction from the Linux kernel
#include "list.h"

// MIDI-specific definitions
#include "midi_defs.h"

// LAB/HAK ZONE:
#define SEQ_LEN 10
uint8_t midi_sequence[SEQ_LEN] = {0xF0, 0x00, 0x00, 0x50, 0x22, 0x03, 0x40, 0x09, 0x00, 0xF7};

#define CSEQ_LEN (2 * 3)
uint8_t midi_complex[CSEQ_LEN] =
{
0x90, 0x40, 0x40, //Start of E3 note, pitch = 64)
// 0x90, 0x43, 0x40, //Start of G3 note, pitch= 67)
// 0x80, 0x43, 0x00, //End of G3 note, pitch=67)
// 0x90, 0x45, 0x40, //Start of A3 note, pitch=69)
// 0x80, 0x45, 0x00, //End of A3 note, pitch=69)
0x80, 0x40, 0x00, //End of E3 note, pitch=64)
// 0x90, 0x3C, 0x40, //Start of C3 note, pitch = 60)
// 0x90, 0x47, 0x40, //Start of B3 note, pitch= 71)
// 0x80, 0x47, 0x00, //End of B3 note, pitch= 71)
// 0x90, 0x48, 0x40, //Start of C4 note, pitch= 72)
// 0x80, 0x48, 0x00, //End of C4 note, pitch= 72)
// 0x80, 0x3C, 0x40, //End of C3 note, pitch = 60)
};
// END OF LAB/HAK ZONE


// from TEPIS - time divisors
#define DEFAULT_DIVISOR 976
#define DEFAULT_TIME_BASE 1000000
// up to NUM_MIDI_SEQS can be recorded, per NUM_CHANNELS
#define NUM_MIDI_SEQS 64
#define NUM_CHANNELS 16

// current state of the MIDI processing system
static uint16_t midi_state = 0;
static uint16_t midi_frame = 0;

// what we are looking at - the current View
typedef struct {
    uint8_t channel;                 	// MIDI loved MTV
    uint16_t time_base;      	// time lords too
    void *v_arg;                  	// this ones for you
} tMIDIView;

tMIDIView curr_midiview;			// current MIDI View

// a MIDI record
typedef struct  {
	uint8_t buffer[4]; 				// raw midi bytes
	uint16_t pkt_time;			// time of arrival
	uint8_t is_used; 			// is currently in use
	uint16_t r_stat;						// read status
	uint16_t w_stat;						// write status
} tMIDIPacket;

// head of MIDI_sequences
typedef struct  {
	uint16_t in_time;			//
	struct list_head list;			// for larger MIDI seqs (sysex)
	tMIDIPacket midi_seqs[NUM_MIDI_SEQS];	// set of midi_seqs
} tMIDISequence;

// MIDI Sequences
static tMIDISequence MIDI_sequences[NUM_CHANNELS];

// current packets per frame
tMIDIPacket *midi_in_pkt;
tMIDIPacket *midi_out_pkt;

// inbox/outbox for packets per frame
uint8_t midi_inbox = 0;
uint8_t midi_outbox = 0;

// uint8_t d_period=50;
uint8_t sync_count;    				// sync counter

// Send a MIDI message
static uint16_t MIDIPut(uint8_t *data, uint16_t cnt) {
	 // setRGB(15, 100, 100, 0);
	// Serial.print(data[ret], HEX);
	// blink(STAT2, d_period);
// setRGB(15, 0, 100, 0);
// delay(10);

	return(Serial1.write(data, cnt));
}

// Receive a MIDI message if its available
static uint16_t MIDIGet(uint8_t *data, uint16_t cnt) {
	uint16_t ret;
	ret = 0;
	while ((cnt>0) && (data[ret] = Serial1.read()) != -1) {
		ret++; cnt--;
	}
  // blink(STAT1, d_period);
// setRGB(0, 100, 0, 0);
// delay(10);

	return ret;
}


void MIDIFrame ()
{

// ANALYZE STATE:
#if 0
	static uint16_t  knob_1;
	static uint16_t  knob_2;
	static uint8_t button1;
	// LED's are watching the clock.
	update_leds();
	// knobs
	knob_1 = analogRead(KNOB1);
	knob_2 = analogRead(KNOB2);
	// reset hack
	if (pad1.isPressed())
	{
		uint16_t i;
		for(i=0;i<NUM_MIDI_SEQS;i++){
			//      MIDI_sequences[curr_midiview.channel].midi_seqs[i].is_used = 0;
			if(i%4)
	  			blink(STAT1, d_period);
			else
	  			blink(STAT2, d_period);
			//      midi_inbox=0; midi_outbox=0;
		}
		curr_midiview.channel = 0;
		curr_midiview.time_base = DEFAULT_TIME_BASE / 3;
		curr_midiview.v_arg = NULL;
		midi_state = 0;
		//button1 = (button1 == 0)?1:0;
	}

	if (pad2.isPressed())
	{
		// blink(STAT1, d_period / 2);
		curr_midiview.time_base = DEFAULT_TIME_BASE / 2;
		midi_state = 1;
	}

	if (pad3.isPressed())
	{
		// blink(STAT2, d_period * 2);
		curr_midiview.time_base = DEFAULT_TIME_BASE * 2;
		midi_state = -1;
	}
#endif
  	curr_midiview.time_base = DEFAULT_DIVISOR; //knob_1 * DEFAULT_DIVISOR;

// PROCESS I/O:
	midi_in_pkt = &MIDI_sequences[curr_midiview.channel].midi_seqs[midi_inbox];
  	midi_out_pkt = &MIDI_sequences[curr_midiview.channel].midi_seqs[midi_outbox];

  	// pull midi_inbox
  	if (Serial1.available()) {
setRGB(7, 100, 0, 0);
delay(10);

	  	if (midi_in_pkt->is_used == 0)
	  	{
  			midi_in_pkt->r_stat = MIDIGet(&midi_in_pkt->buffer[0], 4);
	    	midi_in_pkt->pkt_time = micros() ; ///  time
   	 		midi_in_pkt->is_used = 1;

   	 		// inbox wrap
    		midi_inbox++;
    		if (midi_inbox >= NUM_MIDI_SEQS)
    			midi_inbox = 0;
		}
	}
  	else
  	// push midi_outbox
  	{
setRGB(7, 0, 0, 100);
delay(10);

	  	if (midi_out_pkt->pkt_time <= (micros() - curr_midiview.time_base))
  		{
  			if (midi_out_pkt->is_used == 1)
  			{
  				midi_out_pkt->w_stat = MIDIPut(&midi_out_pkt->buffer[0], 4);
  				midi_out_pkt->is_used = 0;

  				// if (midi_state==1)
  				// {
  				// 	midi_out_pkt->pkt_time += (micros() + curr_midiview.time_base * 2);
  				// 	midi_out_pkt->is_used = 1;
      //       	// blink(STAT2, d_period * 2);
  				// }
  				// else
  				// 	midi_out_pkt->is_used = 0;

  				// outbox wrap
	  			midi_outbox++;
  				if (midi_outbox >= NUM_MIDI_SEQS)
  					midi_outbox = 0;

  			}
  		}
  	}

	midi_frame++;

  //if (midi_outbox == midi_inbox) midi_inbox=0;

	/*
	switch(midi_byte1)
	 {
	case midi_start:
	case midi_continue:
	play_flag = 1;
	break;
	case midi_stop:
	play_flag = 0;
	break;
	case midi_clock:
	Sync();
	break;
	}
	*/
}

// TODO: MIDI Sync
void MIDISync()
{
	if (sync_count < 24)
	{
		sync_count = sync_count + 1;
    	// blink(STAT2, d_period);
	}
	else
	{
		sync_count = 0;
		//MIDIPut(0x80, 60, 0x00);
		//MIDIPut(0x90, 60, 0x40);
    	// blink(STAT1);
	}
}


void MIDIMode()
{
	setAll(0,0,0);
	while (1) {

		MIDIFrame();

		// setAll(midi_frame % 300 == 0 ? 256 : 0, midi_frame % 300 == 100 ? v : 256, midi_frame % 300 == 200 ? v : 256);
		// setRGB(15, centerBtnPressed ? 100 : 0, powerBtnPressed ? 100 : 0, 0);
		updateLedsWithBlank();
		// delay(1);
		if (!MagicShifter_Poll())
			break;

		// Debug:
		if (powerBtnClickedTime != 0) {
			setAll(100, 0, 0);
			updateLedsWithBlank();
			delay(10);
			setAll(0,0,0);
			powerBtnClickedTime = 0;
		}
		if (centerBtnClickedTime != 0) {
			setAll(0, 0, 100);
			updateLedsWithBlank();
			delay(10);
			setAll(0,0,0);
			centerBtnClickedTime = 0;
		}
	}
}


#ifndef __SYSTEM_MESSAGES_HPP
#define __SYSTEM_MESSAGES_HPP
/**
 * System messages is mandatory for all threads. It's can't be avoided.
 * FORMAT:
 * 1) 8 bit - signal
 * 2) 8 bit - service numbers mask. max=8 services.
 * */
//TERMINATE THREAD
#define INTERRUPT        0x0100

#endif

#pragma once
// AudioQueue.h — thread-safe command queue for DFRobot player commands.
// Core 0 enqueues play/pause requests; Core 1 drains and executes them.
// This avoids SoftwareSerial being driven from two cores simultaneously.

#if USE_AUDIO >= 1

#include <Arduino.h>
#include <pico/mutex.h>

enum AudioCmd { AUDIO_PLAY, AUDIO_PAUSE };

struct AudioRequest {
    AudioCmd cmd;
    int      player;   // 1 or 2
    int      track;    // track number (only for AUDIO_PLAY)
};

#define AUDIO_QUEUE_SIZE 8

class AudioQueue {
public:
    AudioQueue() : _head(0), _tail(0) { mutex_init(&_mtx); }

    // Called from Core 0 (Action::trigger / Action::stop)
    bool enqueue(AudioCmd cmd, int player, int track = 0) {
        mutex_enter_blocking(&_mtx);
        int next = (_tail + 1) % AUDIO_QUEUE_SIZE;
        bool ok = (next != _head);
        if (ok) {
            _buf[_tail] = {cmd, player, track};
            _tail = next;
        }
        mutex_exit(&_mtx);
        return ok;
    }

    bool isEmpty() {
        mutex_enter_blocking(&_mtx);
        bool empty = (_head == _tail);
        mutex_exit(&_mtx);
        return empty;
    }

    // Called from Core 1
    bool dequeue(AudioRequest& out) {
        mutex_enter_blocking(&_mtx);
        bool ok = (_head != _tail);
        if (ok) {
            out   = _buf[_head];
            _head = (_head + 1) % AUDIO_QUEUE_SIZE;
        }
        mutex_exit(&_mtx);
        return ok;
    }

private:
    mutex_t      _mtx;
    AudioRequest _buf[AUDIO_QUEUE_SIZE];
    volatile int _head;
    volatile int _tail;
};

extern AudioQueue audioQueue;

#endif // USE_AUDIO >= 1
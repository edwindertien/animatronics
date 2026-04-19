#include "config.h"
#if USE_AUDIO >= 1
#include "AudioQueue.h"

// Single global instance — Core 0 enqueues, Core 1 drains
AudioQueue audioQueue;

#endif

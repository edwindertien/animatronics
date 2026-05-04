# picoControl — Development Context & Technical Notes

## Purpose
This document records architectural decisions, bugs encountered, and lessons learned during development of the picoControl receiver firmware. It is intended for future developers (or future AI assistant sessions) to avoid repeating the same investigations.

---

## Core Architecture Decisions

### Audio Must Never Run on Core 1
The most painful recurring mistake during development: audio processing (DFRobot DF1201S via SoftwareSerial) kept migrating back to Core 1 alongside the CRSF receive loop. Every time this happened, CRSF processing would halt when audio stalled, causing the entire vehicle to freeze.

**The rule, established permanently:** Core 1 runs CRSF and nothing else. `platformSetup1()` and `platformLoopCore1()` are empty for all vehicles. Audio is initialised on Core 0 in `platformSetup()` and drained on Core 0 in `platformLoop()` via `AudioQueue`.

The `AudioQueue` is a mutex-protected ring buffer. Core 0 enqueues play/pause commands from `Action::trigger()` and `Action::stop()`. Core 0 also drains the queue in `platformLoop()`. The queue is same-core for all current vehicles — the mutex is harmless overhead but was designed for cross-core use.

### Core 0 Setup Must Complete Before Core 1 Starts
Core 1 `setup1()` spins on `_core0SetupDone` flag. This flag is set at the very end of Core 0's `setup()`, after `platformSetup()` (which includes `audioInit()`). This ensures Core 1 never touches peripherals before Core 0 has initialised them.

Early versions called `audioInit()` from `platformSetup1()` on Core 1, causing a chicken-and-egg problem: CRSF wouldn't start until audio finished initialising, and audio init could block indefinitely if a player wasn't connected.

### Audio Initialisation Timeout
`audioInit()` was originally a blocking `while (!player.begin()) { delay(1000); }` infinite loop. If a player was disconnected or powered off, the entire system would hang at startup — no CRSF, no display, nothing. 

Fixed by limiting to 5 attempts × 500ms = 2.5s maximum per player, after which the system continues without that player. Serial output clearly indicates which player failed.

---

## Relay System Struggles

### BOARD_Vx Define Order — Critical
`PicoRelay.h` is included via `Action.h` which is included near the top of `config.h`. The `BOARD_Vx` defines for each vehicle were originally placed inside each vehicle's `#ifdef` block — **after** the `#include "Action.h"` line. This meant `PicoRelay.h` was compiled without any board version defined, resulting in empty `begin()` and `writeRelay()` stubs. Relay calls were silently no-ops.

**Fix:** `BOARD_Vx` is now defined in an early block at the top of `config.h`, before any `#include` lines. For SCUBA and AMI (which can use either board), the version is set in a dedicated `#if defined(SCUBA) || defined(AMI)` block in that same early section. **Never** define `BOARD_Vx` again inside a vehicle's config block.

### PCA9685 (V1) — Startup Flash
The PCA9685 defaults all outputs to LOW on power-up, before any firmware runs. Since relays are low-active, this briefly activates all 16 relays at startup. This is a hardware-level behaviour that cannot be prevented in software — the RP2040 hasn't started executing yet when the PCA9685 powers on.

This was the primary motivation for switching to the PCA9635 for the V2 board.

### PCA9635 (V2) — Open-Drain Output Polarity
The PCA9635 was initially wired with inverted logic. `PCA963X_LEDON` (01) was assumed to mean "output HIGH" based on the name, but the PCA9635 has **open-drain outputs**:
- `PCA963X_LEDON` (01) = output **sinks LOW** = relay ON
- `PCA963X_LEDOFF` (00) = output **floating** (pulled HIGH externally) = relay OFF

Multiple complete inversions of the relay logic were applied and reverted during debugging. The confirmed correct mapping:
- `writeRelay(n, true)` → `PCA963X_LEDON` → output LOW → relay activated
- `writeRelay(n, false)` → `PCA963X_LEDOFF` → floating/HIGH → relay deactivated
- Initialisation: all channels set to `PCA963X_LEDON` wait — `PCA963X_LEDOFF` (safe off)

Additionally, the PCA9635's LEDOUT registers are uninitialised after `begin()`. Without explicitly calling `setLedDriverMode()` on all 16 channels, subsequent writes are silently ignored. Always initialise all channels immediately after `begin()`.

### Relay Numbering Lost in Translation
During the migration from the old codebase (which used char-based button values like `'5'`, `'*'`) to the new protocol, the AMI action list was rebuilt from scratch using estimated relay numbers. The original relay assignments (0-15 on PCA9685, 16-23 on GPIO) were partially lost.

Recovery was done by cross-referencing the original `config.cpp` file against the new action list. Key lesson: **always keep the original relay wiring comment in the action list** (e.g., `// arm uit — relay 0`). The relay number is the physical wiring; the button/switch assignment can change but the relay number is fixed by the hardware.

For AMI, relay numbers 16-23 are GPIO-driven via the `EXTRA_RELAY` mechanism, mapped to GPIO pins 18, 19, 20, 21, 22, 26, 27, 28.

---

## Action System Evolution

### Old Protocol — Char-Based Buttons
The original codebase used `char` values for button encoding: `'5'` for keypad 5, `16` for switch 16, etc. Switch numbers above 15 were used for a different multiplexing scheme. This was completely incompatible with the new CRSF protocol.

### New Protocol — SW(n, state) and KEY_ACTION
The new protocol encodes all button inputs as integers:
- `SW(n, 0/1/2)` = MUX_LOW/MID/HIGH for mux channel n
- `KEY_ACTION(KEY_BIT_x)` = keypad bit
- `-1` = internal/sequence-driven only

The `_buttonPressed()` function routes based on integer ranges:
- `< 0`: always false
- `0-29`: `getRemoteSwitch()` (HIGH = state 2)
- `30-49`: `getRemoteSwitchLow()` (LOW = state 0)
- `50-65`: `getRemoteSwitchMid()` (MID = state 1)
- `>= 100`: `getRemoteKey()`

`ActionSequence` initially used `getRemoteSwitch()` directly and ignored the `SW()` encoding — fixed by adding `_seqButtonPressed()` with identical routing logic.

### Sequence-Driven Actions Must Use TRIGGER Mode with Button = -1
A subtle bug: if a sequence calls `trigger()` on an action that has a real button assigned and uses `DIRECT` mode, `update()` will immediately call `stop()` on the next 20Hz tick because the button is not physically held. The audio or relay activates and immediately deactivates.

Solution: create a separate action entry with `button=-1` and `TRIGGER` mode for sequence use. The same relay/audio can have a sibling action with a real button for manual triggering. For SCUBA, `myActionList[1]` (jaws audio) has `button=-1, TRIGGER` so the sequence can control it without `update()` interference.

### getRemoteSwitch() Initially Too Permissive
After the char-to-int migration, `getRemoteSwitch()` was temporarily changed to `state >= 1` (MID or HIGH) to fix a case where 2-position switches only produced state 1 instead of state 2. This broke 3-position switches by treating MID and HIGH identically, and caused the AMI "looking" sequence to be permanently triggered (mux 12 rests at state 1).

**Fix:** `getRemoteSwitch()` strictly checks `state == 2`. Use `SW(n, 1)` explicitly when MID is the active state.

---

## RS485 Inter-Packet Timing
The receiving RS485 nodes need time to process each incoming packet before the next one arrives. Early code sent eye servo updates (node 10, channels 0 and 1) on every 20Hz tick, flooding the bus. When an eyelid command (channel 2) was sent immediately after, it was dropped.

Two fixes applied:
1. Eye servo updates only sent when the value actually changes (`static int lastEye`)
2. Eyelid command gets its own dedicated 20Hz tick — eye servo update is suppressed that tick via a `blinkPending` flag
3. `RS485WriteBuf()` includes `delayMicroseconds(500)` after each packet as a universal inter-packet gap

A `delay(1)` in `RS485WriteByte()` was the first working workaround — the 500µs version is cleaner and less disruptive to the 20Hz loop.

---

## Animation System

### Animation Key Channel Always Updated During Playback
During animation playback, `channels[]` is frozen from the live CRSF data so the animation can write its own values. But the animation key channel itself must remain live — otherwise releasing the switch doesn't stop the animation.

`main.cpp` always updates `channels[8]` (nunchuck) and `channels[11 + ANIMATION_KEY/4]` (animation key bank) from fresh CRSF data, even during playback.

### Animation Key State (ANIMATION_KEY_STATE)
`ANIMATION_KEY` was originally always checked with `getRemoteSwitch()` (state == 2 = HIGH). Some vehicles use MID (state 1) as their animation trigger. `ANIMATION_KEY_STATE` in `config.h` (0, 1, or 2) selects which state triggers animation. The `ANIMATION_KEY_PRESSED` macro in `main.cpp` resolves to the correct `getRemoteSwitch*()` call.

### Background Track Race Condition (SCUBA)
SCUBA plays a background bubble track that restarts automatically via `processBackground()`. When the jaws sequence starts, it enqueues a play command for the jaws track. But `processBackground()` would immediately play the bubble track over it on the same tick.

Fix: `_bg1Foreground` flag is set when any foreground track is enqueued on player 1, and cleared when player 1 is paused. `processBackground()` returns immediately if `_bg1Foreground` is true.

---

## OLED Display

### Screen Name Truncation
Vehicle names were originally printed in full (e.g., "KREEFT", "SCHILD"). These overflowed into the mode field. All names truncated to 3 characters: SCU, AMI, LUM, KRE, VIS, SCH, LOV, WAS.

### "seq:" vs "sq:"
The sequence status field uses "sq:" (3 chars + time) rather than "seq:" (4 chars) to avoid overlapping the "LQ:" link quality field at x=86.
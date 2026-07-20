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

Recovery was done by cross-referencing the original `config.cpp` file against the new action list. Key lesson: **always keep the original relay wiring comment in the action list** (e.g., [ami] `// arm uit — relay 0`). The relay number is the physical wiring; the button/switch assignment can change but the relay number is fixed by the hardware.

For AMI specifically, relay numbers 16-23 are GPIO-driven via the `EXTRA_RELAY` mechanism, mapped to GPIO pins 18, 19, 20, 21, 22, 26, 27, 28.

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

### Background Track Race Condition [scuba]
[scuba] plays a background bubble track that restarts automatically via `processBackground()`. When the jaws sequence starts, it enqueues a play command for the jaws track. But `processBackground()` would immediately play the bubble track over it on the same tick.

Fix: `_bg1Foreground` flag is set when any foreground track is enqueued on player 1, and cleared when player 1 is paused. `processBackground()` returns immediately if `_bg1Foreground` is true. (First encountered on scuba.)

---

## OLED Display

### Screen Name Truncation
Vehicle names were originally printed in full (e.g., "KREEFT", "SCHILD"). These overflowed into the mode field. All names truncated to 3 characters: SCU, AMI, LUM, KRE, VIS, SCH, LOV, WAS.

### "seq:" vs "sq:"
The sequence status field uses "sq:" (3 chars + time) rather than "seq:" (4 chars) to avoid overlapping the "LQ:" link quality field at x=86.

---

## Track file editing — lessons learned

### Switch encoding confusion [animal_love]

Significant time was lost trying to identify which fields in the `animationStep` struct controlled which actuators. The root cause was conflating three different representations of the same information:

1. **Debug terminal** prints `SW[N]=state` where N is the mux channel number (0-15)
2. **Action list** uses `SW(mux, state)` macro notation
3. **Track file** stores packed bits: mux channel N → field `5+N/4`, bits `(N%4)*2` and `(N%4)*2+1`

Initially the wrong field/bit was modified (e.g. editing field7 thinking it was mux2, when mux2 is actually in field5 bits 4-5). The correct lookup [animal_love]: `SW(2,1)` = siren = field5 value 16; `SW(2,2)` = spray = field5 value 32.

### Top board uses raw bitmask, not mux decode [animal_love]

The animal love top board's `getRemoteSwitch(button)` reads bit `button` directly from the packed switch bytes — it does NOT decode mux channel states. Button number 5 = bit 5 of `switches1` (field5) = mux channel 2, state bit 1 (MSB of the 2-bit state = state2). This means:

- [animal_love] `SW(2,1)` (siren, sets bit4 of field5) does NOT trigger the spray relay
- [animal_love] `SW(2,2)` (spray, sets bit5 of field5) DOES trigger the spray relay (top board action(5))
- The two states are completely independent at the relay level

This was non-obvious because the bottom board uses the standard mux decode while the top board treats the same bytes as a flat bitmask.

### Track visualiser workflow [animal_love]

The track visualiser (`tools/track_visualiser.py` + `tools/channel_map.yaml`) was essential for catching errors after programmatic track edits. Key lessons:

- Always re-run the visualiser after any edit to verify the result before flashing
- The `--start` and `--end` arguments use step numbers (20 steps = 1 second), not seconds
- States that share a mux channel (siren=state1, spray=state2 on mux2) must be on separate rows in the channel map — the visualiser plots one row per (mux, state) pair

### Default vs expo animation — copy discipline [animal_love]

The default and expo animations must be kept in sync manually. Several sessions were spent debugging the robot running the old default animation because:

1. Edits were applied to expo but not propagated to default
2. The `clear_mux()` function correctly zeroed the POTEN bits but left other field7 bits unchanged, causing subtle drift between the arrays

**Correct procedure:** always rebuild default as a complete copy of expo with mux9 cleared, rather than patching it incrementally. Verify with a byte-by-byte comparison script before committing.

### Siren was recorded as always-on [animal_love]

In the original recorded animal_love expo track, the siren switch (mux8 state1, later correctly identified as mux2 state1) was held on for the entire 49-second track by the operator during recording. This required clearing it from 962 steps and re-adding it as a deliberate 1-second burst. Always check the full field value distribution after recording before using a track in production.

### Spray was never recorded [animal_love]

The animal_love spray actuator (`SW(2,2)`) did not appear in the original recording — the operator never triggered it during the session. It was added programmatically after the fact. The two original spray-like periods in the recording (field5 values of 32 at steps 557-641 and 753-836) were actually `mux2 state2` which correctly mapped to spray but were 4.2 seconds each and overlapped with the head motor period. These were replaced with a single clean 2-second burst at the 5-second mark.

---

## Stofzuiger DDSM210 integration — lessons learned

### DDSM library uses HardwareSerial* — patched to Stream*

The `ddsm_ctrl` library declared `pSerial` as `HardwareSerial*`, which does not accept `SoftwareSerial`. Patched to `Stream*` — both `HardwareSerial` and `SoftwareSerial` inherit from `Stream`, and the library only calls `write()`, `available()`, and `read()` which are all `Stream` methods. The library also had a bug in `ddsm115_fb()` where it read from hardcoded `Serial1` instead of `pSerial` — fixed to `pSerial->readBytes()`.

### DDSM ownership — keep in platform file, not main.cpp

The DDSM `dc` and `DDSMport` objects were initially declared in `main.cpp` under `USE_DDSM`, matching the washmachine pattern. This caused multiple definition linker errors when the platform file also needed to declare them. Resolution: guard main.cpp's `USE_DDSM` blocks with `!defined(STOFZUIGER)` and declare `dc`/`DDSMport` entirely in `platform_stofzuiger.cpp`. Other vehicles using DDSM (washmachine) are unaffected.

### saveValues — extern pattern for platform-defined arrays

`saveValues[]` is referenced by `main.cpp` but should be defined per-vehicle in the platform file, not in `config.h`. The correct pattern: define in `platform_xxx.cpp`, declare `extern const int saveValues[]` in the vehicle's `config.h` block. This matches how `myActionList` and sequences are handled in other platforms.

### DDSM startup unreliability — USB serial noise

The DDSM init was unreliable: motor stayed locked in position loop on many startups. Root cause: RP2040 boots fast and USB `Serial.begin(115200)` causes brief GPIO noise at the same baud rate as the DDSM SoftwareSerial, corrupting the first `ddsm_change_mode()` packet. Fix: 200ms delay before first UART command, mode + zero command sent 3× with 20ms gaps. A 5-second periodic mode re-assertion in `platformLoop()` provides ongoing recovery.

### DDSM free-float is impossible via UART

Extensive investigation and manual review confirmed the DDSM210 (FOC/PMSM) has no coast/free-float mode accessible via UART. Mode 0 (open loop) with cmd=0 gives minimal but nonzero holding torque. True free rotation only when unpowered. The `DDSM_HOLD` config flag and mode-switching code were added and then removed after this was confirmed.

### Software ramping removed — shocks are inherent

A software RPM ramp (`currentRpm += DDSM_RAMP_STEP` per tick) was implemented to smooth speed changes. It reduced responsiveness without eliminating shocks. Removed — the FOC controller applies full current to reach commanded RPM instantly, and only very gradual joystick movement avoids shocks without a ramp. The hardware `act` byte in the DDSM packet was investigated but the manual confirms it has no documented acceleration function for the DDSM210.

### mode == ACTIVE gate for non-speedscaling motor path [stofzuiger]

The `#else` branch of the `USE_MOTOR` / `USE_SPEEDSCALING` conditional in `main.cpp` previously drove motors immediately with raw channel values before CRSF was active (channels all 0 → motors at -255). Added `mode == ACTIVE` check. Only affects STOFZUIGER since all other motor vehicles use `USE_SPEEDSCALING` which has its own arming check.

### M5Unit8Servos requires USE_M5_SERVOS in config.h

The `servos` object is declared in `main.cpp` under `#ifdef USE_M5_SERVOS`. Adding `extern M5Unit8Servos servos` in the platform file without defining `USE_M5_SERVOS` in config.h causes a linker error (undefined reference). Always add `USE_M5_SERVOS` to the config block of any vehicle using the servo board.

---

## Stofzuiger — additional lessons

### Servo detach on link loss — platformLoop runs unconditionally [stofzuiger]

Several approaches were tried to detach the mouth servo when the RF link is lost:
- `crsfReady()` check — failed because `crsfReady()` is still true on the same tick `platformOnIdle()` fires
- `crsfLost()` check in `platformLoop()` with `detach()` every tick — compiled but did not detach in practice
- `servoAttached` flag set by `platformOnIdle()`, cleared by `platformLoop()` re-attach — race condition

Root cause: `platformLoop()` is called every tick regardless of link state, immediately after the link-loss block. Any `detach()` in `platformOnIdle()` is undone on the very next tick by `writeServoPulse(..., true)`.

**Correct solution:** set `saveValues[2] = 0` so `channels[2]` resets to 0 on link loss, which maps to 1150µs (closed) through the normal servo write path. No detach needed — the servo holds closed via the existing write mechanism. This is the correct picoControl pattern for safe default states.

### Pico W LED routing [stofzuiger]

The Pico W routes `LED_BUILTIN` through the CYW43439 WiFi chip via SPI. `digitalWrite(LED_BUILTIN, HIGH)` does nothing without the correct board definition. Fix: `board = rpipicow` in the stofzuiger platformio.ini env. The EarlPhilhower core then correctly routes the LED — no WiFi initialisation required.

### DDSM210 motor ID must match DDSM_ID define [stofzuiger]

The DDSM210 stores its address in flash (factory default ID=1). The codebase uses `DDSM_ID 4`. A replacement motor that does not respond is almost certainly still at ID 1. Diagnose with `dc.ddsm_id_check()` (broadcasts to 0xC8), then either change `DDSM_ID` in the platform file or reprogram with `dc.ddsm_change_id(4)`.

### USE_M5_SERVOS must be defined for extern servos to link [stofzuiger]

`servos` is declared in `main.cpp` under `#ifdef USE_M5_SERVOS`. Without this define in config.h, `extern M5Unit8Servos servos` in the platform file produces an undefined reference linker error. Always add `USE_M5_SERVOS` to any vehicle config that uses the servo board.

---

## DDSM210 free-spin investigation [stofzuiger, experimental]

### All UART modes apply active FOC — no true coast

Systematic investigation of all available DDSM210 modes:

- **Speed loop (mode 2) at cmd=0**: motor holds at 0 rpm with active torque. `cur=-15` constant = magnetic cogging, not FOC. Cannot be eliminated via software.
- **Open loop (mode 0) at cmd=0**: worse than speed loop. Shorts the windings → regenerative braking. `cur` spikes to -9000+ when pushed. Abandoned.
- **Position loop (mode 3) with current pos as setpoint**: motor spins at full speed near deadband. FOC loop runs at ~10kHz; our 20Hz update rate means the motor fights movement for 50ms before setpoint updates. Abandoned.
- **Speed loop with position-following**: same problem — controller always applies full current to reach each new setpoint.
- **Heartbeat timeout**: motor continues at last commanded RPM after timeout, not passive. Only works if we also send zero-velocity commands, which defeats the purpose.

**Root cause**: PMSM motors with permanent magnets always have magnetic cogging torque that cannot be eliminated by software. The FOC controller prevents any passive state while powered.

### MOSFET power cut — only viable solution

Hardware MOSFET (N-channel) on GPIO11 cuts 24V to the DDSM. Motor is truly passive when unpowered. Boot time after power restore: ~80ms empirically determined. Non-blocking implementation uses `millis()` so drive motors continue normally during DDSM boot.

TX pin leakage: SoftwareSerial TX (GP14) held HIGH while active creates a current path through DDSM internals when motor power is cut. Fix: `DDSMport.end()` + `pinMode(14, INPUT)` on power cut.

### Experimental vehicle as sandbox

`platform_experimental.cpp` created as an exact copy of stofzuiger with added serial debug output (`spd`, `cur`, `pos`, `tmp`, `flt` every 500ms). All DDSM experiments done here first, then ported to stofzuiger once confirmed. This is the correct workflow for any future DDSM changes.

### Feedback is valid — use it for diagnosis

`ddsm210_fb()` after `ddsm_ctrl()` populates `speed_data`, `current`, `temperature`, `fault_code`. `ddsm_get_info()` additionally populates `ddsm_pos` (0-32767 = 0-360°). These are essential for verifying motor behaviour. Initial session had `spd=0` always because no `ddsm_get_info()` was called — the 0x64 feedback doesn't include position.

---

## BetaFPV OLED layout [washmachine, stofzuiger, desklight, experimental]

`#define USE_BETAFPV` switches the bottom OLED rows to match the physical BetaFPV Lite 3 controller layout. Key decisions:

- Left stick (X2/Y2) shown on left — Y2 shown as vertical with inversion, X2 as horizontal (matching DDSM and mouth servo axes)
- Right stick (X1/Y1) shown on right — Y1 inverted
- SA/SB/SC/SD shown as toggle switch icons (11×7px rectangles) without labels
- SB above SA, SC above SD — matches physical controller layout
- SA/SD flush with bottom of 32px display
- No volume bar, no keypad, no nunchuck — these inputs don't exist on BetaFPV
- RSSI bars and value aligned below LQ column on top row

The toggle icon design: outline rectangle = state 0, filled block slides position for state 1/2/3. Distinguishes 2-pos (one block, left/right) from 3-pos (block slides through three positions).

---

## Desklight platform migration [desklight]

STS3215 servo control moved from `main.cpp` `USE_STS` block to `platform_desklight.cpp`. The `USE_STS` declaration and init (STSport, st, STSport.begin()) remain in `main.cpp` as infrastructure — only the per-tick `WritePosEx` calls moved to `platformLoop()`. Same pattern as DDSM migration for stofzuiger.

SD park mode overrides all other inputs with `return` — this is the correct pattern for an emergency/safe-state switch in `platformLoop()`. SA speed mode uses two sets of speed/accel values selected per tick — no mode switch needed since `WritePosEx` takes speed and accel as parameters.

---

## DDSM210 free-spin investigation [stofzuiger, experimental]

### No true coast mode exists via UART

All software modes apply active FOC:
- **Speed loop (mode 2) cmd=0**: holds at 0 rpm with `cur=-15` constant (magnetic cogging, not FOC — cannot be eliminated)
- **Open loop (mode 0) cmd=0**: worse — shorts windings causing regenerative braking. `cur` spikes to -9000+ when pushed. Abandoned.
- **Position loop (mode 3) tracking current pos**: motor spins at full speed near deadband. FOC runs at ~10kHz; 20Hz update rate means motor fights movement for 50ms before setpoint updates. Abandoned.
- **Heartbeat timeout**: motor keeps last commanded RPM; only stops if no commands sent. Implemented but failed because `platformLoop()` keeps sending `ddsm_ctrl(id,0,0)` in deadband — heartbeat never expires.

**Root cause**: PMSM permanent magnets always have cogging torque that no software command can eliminate. The `-15` current reading is the physical motor, not the controller.

### MOSFET power cut — confirmed working solution

N-channel MOSFET on GPIO11: HIGH = motor powered, LOW = power cut. Motor truly free-spins when unpowered.

**TX pin leakage**: SoftwareSerial TX (GP14) stays driven HIGH when active, creating a leakage path through DDSM internals when power is cut. Fix: `DDSMport.end()` + `pinMode(14, INPUT)` on power cut. Re-init with `DDSMport.begin(115200)` on restore.

**Non-blocking boot sequence**: after power restore, 80ms `millis()`-based wait (not `delay()`) before re-sending `ddsm_change_mode` + `ddsm_ctrl`. During boot wait, `return` skips DDSM commands but allows drive motors and other platform functions to continue.

**SA switch mode selection** (channels[4], 3=free-spin, 252=lock):
- SA off: GPIO11 LOW + TX released → true free-spin
- SA on: speed loop at 0 rpm → holds position

Tune boot wait: reduce from 80ms in steps of 10ms until occasionally unreliable, add 20ms back.

### Experimental vehicle as sandbox

`platform_experimental.cpp` is an exact copy of stofzuiger used as a sandbox. All DDSM changes go here first; confirmed working changes are ported to stofzuiger. This prevents breaking a working vehicle during experimentation. Build with `pio run -e experimental`.

**Current state**: experimental reverted to clean stofzuiger base (no CAN). CAN integration was attempted but caused boot freeze — see AK60/CAN section below.

### Feedback is valid — use ddsm_get_info()

`ddsm_ctrl()` calls `ddsm210_fb()` internally, populating `speed_data`, `current`, `temperature`, `fault_code`. These came back plausible. `ddsm_pos` (0-32767 = 0-360°) requires a separate `ddsm_get_info()` call (0x74 packet). Without it, `ddsm_pos` always reads 0.

Debug output confirmed working:
```
OFF   spd=0   cur=0     pos=0     tmp=33 flt=0   — power cut
SPD   spd=450 cur=194   pos=25160 tmp=33 flt=0   — active
LOCK  spd=0   cur=-15   pos=20222 tmp=33 flt=0   — holding at 0
```

---

## BetaFPV Lite 3 OLED layout [washmachine, stofzuiger, desklight, experimental]

Add `#define USE_BETAFPV (1)` to config.h block for these vehicles. Implemented in `main.cpp` as `#ifdef USE_BETAFPV` / `#else` / `#endif` around the bottom display rows.

### BetaFPV channel mapping (confirmed via debug output)

| Switch | Channel | Values | Type |
|---|---|---|---|
| SA | ch4 | 3=off, 252=on | 2-pos |
| SB | ch5 | 3=low, 128=mid, 252=high | 3-pos |
| SC | ch6 | 3=low, 128=mid, 252=high | 3-pos |
| SD | ch7 | 3=off, 252=on | 2-pos |

Sticks: ch0=X1, ch1=Y1, ch2=X2, ch3=Y2. No nunchuck, no keypad, no volume.

### OLED layout (128×32)

```
x=0        x=14   x=17-35         x=40-58         x=63-73  x=86       x=96
[EXP/WAS]         [J_left cx=26]  [J_right cx=50]  [SC][SD] [LQ:xxx%]  [▂▄▆ dBm]
                                                    [SC][SD]
                  [SB top]                                              [RSSI val]
                  [SA bot]
```

- SB at y=17, SA at y=25 (flush bottom) on left side (x=0)
- SC at y=17, SD at y=25 on right side (x=63)
- Left joystick: Y2 as horizontal, X2 inverted as vertical (DDSM and mouth axes)
- Right joystick: X1 horizontal, Y1 inverted
- Toggle icons: 11×7px rect, 2-pos half-filled, 3-pos block slides
- Signal bars at x=96, RSSI value at x=109

Vehicle name (EXP/WAS/STO/DSK) printed at x=14 on top row after SB label.

---

## Desklight — STS3215 servo arm

Confirmed BetaFPV switch mapping for desklight. All servo `WritePosEx` calls moved from `main.cpp` to `platform_desklight.cpp` `platformLoop()`.

### SA speed mode
SA (ch4): 3=slow (speed=2000, accel=100), 252=fast (speed=4000, accel=254). Applied per-tick via `bool fastMode = (channels[4] > 128)`.

### SD park override
SD (ch7 > 128): sends all servos to safe positions and returns immediately, overriding all other input. This `return` pattern is the correct approach for any emergency override in `platformLoop()`.

Park positions (confirmed hardware):
```
ID16: 2048  ID15: 512   ID14: 2700  ID11: 2048
ID20: 0     ID13: 2800  ID12: 1250
```

### SB/SC light controller (ID 20)
SB position: 3→12, 128→526, 252→1036 (via map 0-1048)
SC speed: 3→24, 128→1028, 252→2040 (via map 0-2048)
Light controller source code not yet verified against these ranges.

---

## Omniwheel drive [washmachine]

Confirmed working mixing matrix (after physical testing):
```cpp
int fl =  Y + X + R;
int fr =  Y - X - R;
int bl = -Y - X + R;
int br = -Y + X - R;
```
Previous session had this inverted. The confirmed matrix is now in `platform_washmachine.cpp`.

---

## CLI — Serial command interface

New file: `src/CLI.h` + `src/CLI.cpp`. Called via `processCLI()` from `main.cpp` loop.

Commands (all vehicles):
- `status` — vehicle name, mode, CRSF state, RSSI, LQ, all 16 channels, brake state
- `debug` — toggle 200ms channel dump
- `ver` — vehicle name + build date/time
- `can` — CAN/AK60 status (EXPERIMENTAL only, calls `ak60_print_status()`)
- `help` / `?` — list commands

Implementation: line-based (newline-terminated), static 32-char buffer. `mode` promoted from static local in `loop()` to global `int mode = 0` so CLI.cpp can extern it. `CrsfLinkStats` accessed via `crsfGetLinkStats(ls)` from `core1_crsf.h` — forward-declaring `struct LinkStats` does not work.

---

## AK60 CubeMars CAN motor — ATTEMPTED, NOT YET WORKING

### Hardware
M5Stack CAN Unit (CA-IS3050G transceiver): Grove connector, GP2=TX (yellow), GP3=RX (white). CAN-H/CAN-L to AK60. AK60 CAN ID = **104** (0x68), confirmed from previous session (`#define ID_MOTOR_1 104` in old main.cpp). MIT mode, 1Mbps. AK60-6 min voltage: 12V, rated 24V. AK60 LEDs: blue=power, green=CAN comms, red=fault.

### Library
`ACAN2040` from https://github.com/obdevel/ACAN2040. Installed via `lib_deps` in `[env:experimental]`. Wraps Kevin O'Connor's can2040 PIO driver.

**Correct API** (from source, NOT from docs):
```cpp
// Constructor — all params inline, no separate begin(settings)
ACAN2040(pio_num, gpio_tx, gpio_rx, bitrate, sys_clock, callback);

// Send
can2040.send_message(&msg);  // takes pointer to can2040_msg struct

// Check before send
can2040.ok_to_send();

// Stats
can2040.get_statistics(&stats);  // struct: rx_total, tx_total, tx_attempt, parse_error

// Receive: callback only (interrupt-driven), not polling
```

### Root problem — `irq_set_exclusive_handler` crash

`ACAN2040::begin()` calls `irq_set_exclusive_handler(PIO0_IRQ_0, ...)` which **hard panics** if that IRQ is already claimed. On RP2040 (including standard Pico, not just Pico W), SoftwareSerial (DDSMport) claims PIO IRQs. Switching to PIO1 and/or Hardware Serial2 for DDSM did not resolve the freeze — the exact PIO allocation depends on EarlPhilhower's internal state at the time `begin()` is called.

Attempts made:
- PIO0 → PIO1 switch: still freezes
- SoftwareSerial → Serial2 (hardware UART) for DDSM: reduced conflict but still froze
- Deferred `begin()` to `platformLoop()` first tick: system freezes on that tick
- `board = rpipicow`: did not help (and wasn't needed for plain Pico)

**Current status**: ABANDONED for this session. `platform_experimental.cpp` reverted to clean stofzuiger base without CAN. The ACAN2040 library is fundamentally incompatible with EarlPhilhower + SoftwareSerial due to shared PIO IRQ handlers.

### Next steps for CAN
Options to investigate in a new session:
1. **Patch ACAN2040** to use `irq_set_shared_handler` instead of `irq_set_exclusive_handler` — allows multiple handlers on the same IRQ
2. **Move CAN init to Core 1** via `platformSetup1()` — separate IRQ context, Core 1 has its own interrupt table
3. **Use can2040 directly** (without the ACAN2040 wrapper) and manually configure the IRQ with shared handler
4. **Different CAN library** — look for RP2040 CAN libraries that don't use `irq_set_exclusive_handler`

### MIT protocol packet format (implemented, ready to use once CAN init works)

Send (8 bytes):
```
data[0-1] = position (uint16, P_MIN..P_MAX mapped to 0-65535)
data[2] + data[3]>>4 = velocity (12-bit, V_MIN..V_MAX)
data[3]&0xF + data[4] = kp (12-bit)
data[5] + data[6]>>4 = kd (12-bit)
data[6]&0xF + data[7] = torque_ff (12-bit)
```

Receive (8 bytes, CAN ID = motor ID):
```
data[0] = driver ID
data[1-2] = position (16-bit)
data[3] + data[4]>>4 = velocity (12-bit)
data[4]&0xF + data[5] = current (12-bit)
data[6] = temperature
data[7] = error code (1=overtemp, 2=overcurrent, 3=overvolt, 4=undervolt, 5=encoder, 6=phase)
```

Special commands: enable=0xFC, disable=0xFD, zero=0xFE in last byte with all others 0xFF.

---

## Files changed this session

| File | Status | Notes |
|---|---|---|
| `src/platform_stofzuiger.cpp` | Updated | MOSFET free-spin, SA switch, non-blocking boot |
| `src/platform_experimental.cpp` | Updated | Same as stofzuiger (CAN attempt reverted) |
| `src/platform_desklight.cpp` | Updated | SA speed, SD park, SB/SC light |
| `src/platform_washmachine.cpp` | Updated | Confirmed omni mixing matrix |
| `src/main.cpp` | Updated | USE_BETAFPV OLED layout, CLI integration, mode global |
| `src/CLI.cpp` | NEW | Serial CLI implementation |
| `include/CLI.h` | NEW | CLI header |
| `src/ddsm_ctrl.cpp` | Updated | `ddsm_set_heartbeat()` added (not used in stofzuiger) |
| `include/ddsm_ctrl.h` | Updated | `ddsm_set_heartbeat()` declaration |
| `platformio.ini` | Updated | experimental env added, lib_deps for ACAN2040 |
| `picoControl_README.md` | Updated | DDSM, experimental, BetaFPV, desklight sections |
---

## main.cpp cleanup — dead ROBOTIS/CAN_DRIVER/CUBEMARS code removed [all vehicles]

### What was removed and why

Checked every `#ifdef`-guarded block in `main.cpp` against every `#define` in `platformio.ini` (`build_flags`) and `config.h` (per-vehicle blocks) to confirm which conditional blocks are actually reachable by any build. Three macros — `ROBOTIS`, `CAN_DRIVER`, `CUBEMARS` — are referenced by `#ifdef` in `main.cpp` but are **never defined anywhere in the entire codebase**, for any environment. That code has been permanently dead since at least the start of this session (predates the AK60 work) — removed:

- **`ROBOTIS`** (3 blocks in `main.cpp`): `Dynamixel2Arduino` include/globals, the `dxl.begin()`/torque/position-mode setup sequence, and the per-tick `dxl.setGoalPosition()` call. This was leftover from an early Dynamixel servo experiment that was never wired into any vehicle's `config.h`. The `robotis-git/Dynamixel2Arduino` lib_dep in `platformio.ini`'s `env_base` was also removed — it was being pulled into and compiled for every single vehicle's binary for nothing, since nothing referenced it anywhere else in the tree.
- **`CAN_DRIVER`** / **`CUBEMARS`** (3 blocks in `main.cpp`): the original MCP2515 + `TMotor_ServoConnection` (VESC-style CAN protocol) AK60 integration attempt, predating this session's ACAN2040-based work. Fully superseded — the current AK60 driver lives in the shared `include/ak60.h` / `src/ak60.cpp` module (used by `EXPERIMENTAL` and `WASHMACHINE`), which uses ACAN2040 (RP2040 PIO software CAN) instead of a physical MCP2515 SPI CAN controller.

**Checked and deliberately left alone:** `USE_STS` was raised in the same request as a candidate for removal, but it is **not** dead code — it's actively defined in `config.h`'s `DESKLIGHT` block and used by both `main.cpp` (STSport init) and `platform_desklight.cpp` (per-tick `WritePosEx` calls, per the "Desklight platform migration" section above). Left untouched.

### How to restore ROBOTIS, if ever needed again

The removed blocks (verbatim, for reference — not currently in the tree):

```cpp
// Near top of main.cpp, after the USE_RS485 include block:
#ifdef ROBOTIS
#include <Dynamixel2Arduino.h>
#define DXL_SERIAL   Serial1
#define DEBUG_SERIAL Serial
const int DXL_DIR_PIN = 2;
const uint8_t DXL_ID = 1;
const float DXL_PROTOCOL_VERSION = 2.0;
Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
using namespace ControlTableItem;
#endif

// In setup(), before the USE_CRSF block:
#ifdef ROBOTIS
    Serial1.setTX(0);
    Serial1.setRX(1);
    dxl.begin(1000000);
    dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
    dxl.ping(DXL_ID);
    dxl.torqueOff(DXL_ID);
    dxl.setOperatingMode(DXL_ID, OP_POSITION);
    dxl.torqueOn(DXL_ID);
    dxl.writeControlTableItem(PROFILE_VELOCITY, DXL_ID, 0);
#endif

// In loop(), before the "Timeout / mode management" comment:
#ifdef ROBOTIS
    dxl.setGoalPosition(DXL_ID, map(channels[2], 0, 255, 1024, 3072));
#endif
```

To reactivate: paste the three blocks back into `main.cpp` at the locations noted, add `robotis-git/Dynamixel2Arduino@^0.8.1` back to `env_base`'s `lib_deps` in `platformio.ini` (or to a specific vehicle's env if it shouldn't apply globally), and `#define ROBOTIS (1)` in that vehicle's `config.h` block. Note this was never wired into any real vehicle — pin assignments (`DXL_DIR_PIN`, Serial1 TX/RX) and the position-mode setup would need re-verifying against whatever hardware it's meant for.

### How to restore CAN_DRIVER/CUBEMARS, if ever needed again

The removed blocks (verbatim):

```cpp
// Near top of main.cpp, after the USE_M5_SERVOS block:
#ifdef CAN_DRIVER
#include "mcp_can.h"
#include "TMotor_ServoConnection.h"
#define CAN0_INT 2
#endif

#ifdef CUBEMARS
#define ID_MOTOR_1 104
MCP_CAN CAN0(&SPI1, 17);
TMotor_ServoConnection servo_conn(CAN0);
#endif

// In setup(), before the USE_CRSF block:
#ifdef CAN_DRIVER
    while (CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) != CAN_OK) {
        Serial.println("Error Initializing MCP2515...");
        delay(100);
    }
    Serial.println("MCP2515 Initialized Successfully!");
    CAN0.setMode(MCP_NORMAL);
#endif

#ifdef CUBEMARS
    servo_conn.set_origin(ID_MOTOR_1, 0);
#endif

// In loop(), before the "Timeout / mode management" comment:
#ifdef CUBEMARS
    servo_conn.set_pos_spd(ID_MOTOR_1, 180, 1000, 1000);
    if (!digitalRead(CAN0_INT)) {
        while (CAN_MSGAVAIL == CAN0.checkReceive()) servo_conn.can_receive();
    }
    servo_conn.print_motor_vars(ID_MOTOR_1);
#endif
```

`src/mcp_can.cpp` / `include/mcp_can.h` and `src/TMotor_ServoConnection.cpp` / `include/TMotor_ServoConnection.h` are still present in the repo — nothing was deleted, just unreferenced. If a future vehicle genuinely needs MCP2515 (physical SPI CAN controller board) instead of ACAN2040 (RP2040 PIO software CAN, no extra CAN controller chip needed), these files are ready to wire back in: paste the blocks back, `#define CAN_DRIVER` and/or `CUBEMARS` in that vehicle's `config.h`, and add `mcp_can_lib` (or whichever MCP2515 Arduino library it depends on — check the original `#include` resolution) to that environment's `lib_deps`.

**Strongly consider using `ak60.h`/`ak60.cpp` instead** unless there's a specific reason to need MCP2515 hardware — it's the actively-maintained, tested path (confirmed working on real AK60 hardware in Servo mode, ported from `TMotor_ServoConnection.cpp`'s own math), shared across `EXPERIMENTAL` and `WASHMACHINE`, and doesn't require a separate physical CAN controller chip.
# TODO:

- Change the defined variable names in header file to match the documentation
- Change the fundamental logic of some functions
- Removed redundant conditions

# Commands for control word

- Shutdown: 110
- Switch on: 111
- Disable voltage: 100
- Quick stop: 010
- Disable operation: 0111
- Enable operation: 1111
- Fault reset: 10000000

- Note: bit 2 (quick stop)
  - 1 -> Stay active
  - 0 -> quick stop command
- bit 1 (disable voltage)
  - 1 -> enable voltage
  - 0 -> disable voltage

## Transition

- 0 -> Fault reset (Not ready to switch on)
- 1 -> Ignore (Switch on disabled)
- 2 -> 110 (Ready to switch on)
- 3 -> 111
- 4 -> 1111
- 5 -> 0111
- 6 -> 0110
- 7 -> 100/010
- 8 -> 110 (from operation -> ready to switch on)
- 9 -> 100
- 10 ->
- 11 -> 1011 (ignore maybe)
- 12 -> 1001
- 13 ->

This is the crt0 I've originally shipped.

It's Jeff Frohwein's crt0 v1.28 which is configured to permit multiple interrupts.

It's changed in a way so a Timer1-interrupt is never interrupted by any other interrupt:
This is necessary because Timer1 is used to reset the audio-DMA-position,
if execution of kradInterrupt() is delayed too long (>= 60 us), audible clicks will be output.

See 2.4 in the Developer's handbook for details.


Now, the examples use the crt0/interrupt-handling that comes with devkitPro, which doesn't
know about this special case. Thus, a patch for devkitPro for that case should be made.


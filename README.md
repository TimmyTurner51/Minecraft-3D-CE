# Minecraft-3D-CE
Here be dragons!


Features: 
- Generation using noise. 0 returns superflat, the input is broken though. That will be fixed later.
- Semi-working backface culling. Still trying to work out the kinks.
- FPS counter in place. 
- FOV can be altered from source code. I haven't added any options screen for this though.
- Blinking cursor partially works (still testing with dot projection values to fix that).
- World Size increased to a single chunk, 16x16x16 blocks.
- Each face is textured with 2 colors per face per cube, modifiable in the color sheet.

The program currently is 14985 bytes. Quite small for such a world size, thanks to the compression lol

Future implementations/experiments:

- Realtime generation of a level appvar, it will generate more chunks and store in there and dynamically load from the appvar into the current world array. 
- Placing and destroying blocks when the cursor is stable.

I wrote this with ChatGPT, going back and forth trying optimization after optimization to the rendering. I kind of understand 3D Projection now after doing this so much

Since this was wrote basically entirely by AI, there is no license holder as I think this is instead public domain.

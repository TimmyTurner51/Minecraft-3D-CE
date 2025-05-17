# Minecraft-3D-CE
Here be dragons!

![Image](https://github.com/user-attachments/assets/7b771613-e12f-461b-bca6-7884089e681e)

Features: 
- Generation using noise. 0 returns superflat, the input is broken though. That will be fixed later.
- Semi-working backface culling. Still trying to work out the kinks.
- FPS counter in place. 
- FOV can be altered from source code. I haven't added any options screen for this though.
- Blinking cursor works.
- World Size increased to a single chunk, 16x16x16 blocks.
- Each face is textured with 2 colors per face per cube, modifiable in the color sheet.
- Pause menu in place


Future implementations/experiments:

- Realtime generation of a level appvar, it will generate more chunks and store in there and dynamically load from the appvar into the current world array. 
- Placing and destroying blocks.
- Inventory
- Finished pause menu, fixed bugs.

I wrote this with ChatGPT, going back and forth trying optimization after optimization to the rendering. I kind of understand 3D Projection now after doing this so much

Since this was wrote basically entirely by AI, there is no license holder as I think this is instead public domain.

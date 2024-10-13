![display](Images/SistemTreceri.jpg)
# Copyright Notice
### Licence
Copyright (C) 2024 Nedelcu Bogdan Sebastian  
This code is free software: you can redistribute it and/or modify it under the following conditions:  
1. The use, distribution, and modification of this file are permitted for any purpose,  
   provided that the following conditions are met:  
2. Any redistribution or modification of this file must retain the original copyright notice,  
   this list of conditions, and the following attribution:  
      **"Original work by Nedelcu Bogdan Sebastian."**   
3. The original author provides no warranty regarding the functionality or fitness of this software  
   for any particular purpose. Use it at your own risk.  
By using this software, you agree to retain the name of the original author in any derivative works or distributions.  
This code is provided as-is, without any express or implied warranties.

# treceri
This is the main project used to acquire leakage currents signals, 
and compute **RMS voltage** and **phase** for each of the six channels.

The acquisition is starting after a **zero-cross detection**, and the length
of data is **2048 samples**.

The limits of the signals are between **2.678 mA** and **100 mA**.

More explanations of the algorithms are in the **main.c** file.

*(Note: The code runs on BluePill (STM32F103CBT6) and the IDE is the CooCox 1.7.8.
        The compiler used is the  gcc-arm-none-eabi-10.3-2021.10-win32.exe.
        Use external 23K256 SRAM memory to store all 6 signals at once!)*

# TestFFTPhaseComputation
This project also runs on the **STM32F103CBT6**.

Here we generate sine waves with phases from 0 to 359 degrees, and add noise to 
each signal and compute the phase angle from FFT.

The BluePill will show on the PC side as a **USB SERIAL** device, and we use this method
to print all values over **SERIAL**, the initial **signal phase** and the **computed phase** 
to further analyze them with Python if needed.

**Termite serial** can be used to receive data.

*(Note: The values are scaled by a factor of 10000 !)*

# treceriTestPhaseComputing

This project is made in **Visual Studio Code**. The compiler used is **tdm64-gcc-10.3.0-2**.

In this example we have the following:

    - we generate signals from 0 degrees to 359 degrees,
      of specific phase and noise, and write the values to files named `sine_wave_x.x.txt`,
      with 1.0 degree step
      
    - we generate the Flattop window and write coeffs to a file `flattop_window.txt`
    
    - we apply the window to the signal real side
    
    - we compute FFT
    
    - we compute the phase from bin 9 which is the nearest frequency from 50 Hz (51.4984130859375 Hz)

    - print computed data and repeat until the phase is >= 360.0 degrees
    
We also have some **Python** scripts we can use to see the signals from the files:

    - the `animate_plot_files.py` loads text files, one after another, 
      and plot the signal and the FFT
      
    - the `plot_sine_wave.py` plot only one file
    
    - the `gen_sig_hanning.py` generate a signal and apply hanning window. 
      Also compute phase. 


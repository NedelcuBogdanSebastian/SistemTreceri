# treceri

This is the main project used to acquire leakage currents signals, 
and compute **RMS voltage** and **phase** for each of the six channels.

The acquisition is starting after a **zero-cross detection**, and the length
of data is **2048 samples**.

The limit of the signals are between **2.678 mA** and **100 mA**.

More explanations of the algorithms are in the **main.c** file.

*(Note: The code runs on STM32F103CBT6 and the IDE is the good ol' CooCox. 
        The compiler used is the  gcc-arm-none-eabi-5_4-2016q3-20160926-win32.exe)*

# 

2019.01.06

- This project was build using STM32CubeMX, selecting the nucleo-f429zi board from the library and enabling the clocks, ethernet, rtos 'middlewares' and 2 uarts
- Then the ioc file was opened using Atollic studio and compiled
- Project Properties->Run/Debug Settings->nucleo.elf file was modified to select the ST-Link debugger associated with the dev board
- Note that the STM32CubeMX libraries were included by reference, not copied into the BACnet source tree. This means that the referenced files may have to be reinstalled,
  and or the version paths updated.
- I copied then modified debug config to create 'autorun' to permanently write to flash per: https://forum.atollic.com/viewtopic.php?t=60
  You need to switch moded for debug/autorun sessions in debug config: Run -> Debug Conrigurations -> choose between nucleo.elf and nucleo.autorun.elf


For help:
edward@bac-test.com



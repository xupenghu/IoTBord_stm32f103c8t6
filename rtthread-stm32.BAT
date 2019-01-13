SET PATH=S:\Keil_v5\ARM\ARMCC\Bin;C:\Program Files\VanDyke Software\Clients\;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files (x86)\NVIDIA Corporation\PhysX\Common;C:\Program Files\dotnet\;S:\Program Files\Git\cmd;S:\Program Files\nodejs\;S:\Program Files\python3\Scripts\;S:\Program Files\python3\;C:\Users\Administrator\AppData\Local\Microsoft\WindowsApps;C:\Users\Administrator\AppData\Roaming\npm;C:\Users\Administrator\AppData\Local\Programs\Fiddler
SET CPU_TYPE=STM32F103C8
SET CPU_VENDOR=STMicroelectronics
SET UV2_TARGET=rtthread-stm32
SET CPU_CLOCK=0x00B71B00
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\clock.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\components.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\cpu.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\device.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\idle.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\ipc.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\irq.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\kservice.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\mem.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\memheap.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\mempool.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\object.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\scheduler.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\signal.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\thread.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\timer.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\main.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\board.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via ".\build\stm32f1xx_it.__i"
"S:\Keil_v5\ARM\ARMCC\Bin\ArmLink" --Via ".\build\rtthread-stm32.lnp"

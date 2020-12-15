# Introduction

This is an application that demostrates the use of a Finite State Machine (FSM)
to control the LEDs on a Nordic nRF52840 devkit.

## Monitoring Logs

Using the SEGGER
[J-Link](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack)
tools will allow the monitoring of the log output from the firmware.

Download and install the **J-Link Software and Documentation Pack** for your OS.

The Nordic [command line
tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs)
are also useful to have installed.

### Command Line

Start two terminal windows.

In the first window, create a connection to the DK running the firmware with J-Link:

```
JLinkExe -device NRF52832_XXAA -if SWD -speed 4000 -autoconnect 1
```

If you have multiple DKs attached, you may need to specify the serial number
of the DK to create the correct connection.  The serial number is printed on a
white lable on the large IC on the board.  You can also read the serial numbers
of all attached DKs with the following command:

```
nrfjprog -i
```

The output from the serial number list will look similar to:

```
682117555
682900888
```

The JLink command for a specific DK would look similar to:

```
JLinkExe -device NRF52832_XXAA -selectemubysn 682900888 -if SWD -speed 4000 -autoconnect 1
```

The output from the `JLinkExe` should look similar to:

```
SEGGER J-Link Commander V6.48b (Compiled Aug  2 2019 10:20:30)
DLL version V6.48b, compiled Aug  2 2019 10:20:19

Connecting to J-Link via USB...O.K.
Firmware: J-Link OB-SAM3U128-V2-NordicSemi compiled Jan  7 2019 14:07:15
Hardware version: V1.00
S/N: 682900888
VTref=3.300V
Device "NRF52832_XXAA" selected.


Connecting to target via SWD
Found SW-DP with ID 0x2BA01477
Found SW-DP with ID 0x2BA01477
Scanning AP map to find all available APs
AP[2]: Stopped AP scan as end of AP map has been reached
AP[0]: AHB-AP (IDR: 0x24770011)
AP[1]: JTAG-AP (IDR: 0x02880000)
Iterating through AP map to find AHB-AP to use
AP[0]: Core found
AP[0]: AHB-AP ROM base: 0xE00FF000
CPUID register: 0x410FC241. Implementer code: 0x41 (ARM)
Found Cortex-M4 r0p1, Little endian.
FPUnit: 6 code (BP) slots and 2 literal slots
CoreSight components:
ROMTbl[0] @ E00FF000
ROMTbl[0][0]: E000E000, CID: B105E00D, PID: 000BB00C SCS-M7
ROMTbl[0][1]: E0001000, CID: B105E00D, PID: 003BB002 DWT
ROMTbl[0][2]: E0002000, CID: B105E00D, PID: 002BB003 FPB
ROMTbl[0][3]: E0000000, CID: B105E00D, PID: 003BB001 ITM
ROMTbl[0][4]: E0040000, CID: B105900D, PID: 000BB9A1 TPIU
ROMTbl[0][5]: E0041000, CID: B105900D, PID: 000BB925 ETM
Cortex-M4 identified.
J-Link>
```

In the second window, start the SEGGER Real Time Transfer (RTT) viewer:

```
JLinkRTTClient
```

If the `JLinkExe` process is running in the other window, the output will look similar to:

```
###RTT Client: ************************************************************
###RTT Client: *               SEGGER Microcontroller GmbH                *
###RTT Client: *   Solutions for real time microcontroller applications   *
###RTT Client: ************************************************************
###RTT Client: *                                                          *
###RTT Client: *       (c) 2012 - 2016  SEGGER Microcontroller GmbH       *
###RTT Client: *                                                          *
###RTT Client: *     www.segger.com     Support: support@segger.com       *
###RTT Client: *                                                          *
###RTT Client: ************************************************************
###RTT Client: *                                                          *
###RTT Client: * SEGGER J-Link RTT Client   Compiled Aug  2 2019 10:20:40 *
###RTT Client: *                                                          *
###RTT Client: ************************************************************

###RTT Client: -----------------------------------------------
###RTT Client: Connecting to J-Link RTT Server via localhost:19021 ...
###RTT Client: Connected.

SEGGER J-Link V6.48b - Real time terminal output
J-Link OB-SAM3U128-V2-NordicSemi compiled Jan  7 2019 14:07:15 V1.0, SN=682900888
Process: JLinkExe
```

If the `JLinkExe` process is not running, the output will look similar to:

```
###RTT Client: ************************************************************
###RTT Client: *               SEGGER Microcontroller GmbH                *
###RTT Client: *   Solutions for real time microcontroller applications   *
###RTT Client: ************************************************************
###RTT Client: *                                                          *
###RTT Client: *       (c) 2012 - 2016  SEGGER Microcontroller GmbH       *
###RTT Client: *                                                          *
###RTT Client: *     www.segger.com     Support: support@segger.com       *
###RTT Client: *                                                          *
###RTT Client: ************************************************************
###RTT Client: *                                                          *
###RTT Client: * SEGGER J-Link RTT Client   Compiled Aug  2 2019 10:20:40 *
###RTT Client: *                                                          *
###RTT Client: ************************************************************

###RTT Client: -----------------------------------------------
###RTT Client: Connecting to J-Link RTT Server via localhost:19021 .......
```

The RTT Client will keep printing periods while it waits for a connection to
the `JLinkExe` process.  When the process starts, the followin output will
appear on the client.

```
###RTT Client: Connected.

SEGGER J-Link V6.48b - Real time terminal output
J-Link OB-SAM3U128-V2-NordicSemi compiled Jan  7 2019 14:07:15 V1.0, SN=682900888
Process: JLinkExe
```

If the DK is reset via a button (if configured to have button that can perform
a reset) or via a command:

Specific DK:
```
nrfjprog --reset -s 682900888
```

Single DK:
```
nrfjprog --reset
```

Output simular to the following should appear in the RTT Client terminal window:
```
FSM Demo v0.1.0
<debug> app: nRF52840 Variant: AAE0
<debug> app: RAM: 64KB Flash: 512KB
<debug> app: Device ID: 9489F305EEA8A721
<info> app: Platform: nRF52840-DK
```

If the above output is seen, you can successfully view log output from the
application.

If the DK is powered down, the JLink connection will be lost.  In that case,
switch to the JLinkExe terminal window and type `q` to quit.  Then restart the
process to restablish a connection.

### GUI

TBD


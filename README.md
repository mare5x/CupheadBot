# CupheadBot

CupheadBot is a GUI based trainer for the game Cuphead.

## Usage

Inject the _dll_ with (must run as _admin_):  

```
CupheadBot.exe <absolute path to dll>
```

or copy the _dll_ to _Cuphead.exe_ directory and run:  
```
CupheadBot.exe
```

Once CupheadBot.dll has been successfully injected, you will see a GUI show up right in the game. Use the GUI to enable/disable various hacks. Press **F1** to show/hide the GUI. To uninject the _dll_, press the _exit_ button.

Note: some hacks won't work until you're in a game. For example to enable _invincibility_, you must first get hit. That is because Cuphead uses _just-in-time_ compilation (_JIT_). (This can be fixed by using mono functions, such as _mono_compile_method_.)

## Demo

Available hacks:
  * map wallhack,
  * money hack,
  * invincibility,
  * HP and Super Meter hacks,
  * no cost super,
  * infinite jumping and dashing (and parrying),
  * one hit kill enemies,
  * change loadout at any time (primary weapon, secondary weapon, super and charm),
  * show in-game debug console.

[Demo webm.](./demo.webm)

## Setup

  1. ```git clone https://github.com/mare5x/CupheadBot.git```
  2. Open _CupheadBot.sln_ with _Microsoft Visual Studio_.
  3. Build CupheadBotCL and CupheadBotDLL to get _CupheadBot.exe_ and _CupheadBot.dll_.

## About

CupheadBot is a set of hacks for the game **Cuphead**.  
The purpose of the project was to learn about various _hacking_ and _reverse engineering_ techniques. As such, the code is not ideal and the implementation of many hacks could have been simplified.   

_CupheadBotCL_ is the first version which features only basic hacks implemented using _ReadProcessMemory_, _WriteProcessMemory_ and other _process_ memory functions.  

_**CupheadBotDLL**_ is the improved version which features more hacks as well as an in-game _GUI_ using _**ImGui**_ (using _d3d11_ hooking). CupheadBotDLL is a _dll_ project, so once the _dll_ is injected, controlling program flow is simpler, since the _dll_ lives in the target process' memory space. 

### Techniques used  
  * dll injection,
  * vtable hooks,
  * jump hooks,
  * detour hooks,
  * nop fills,
  * memory signature scanning,
  * writing asm code,
  * trampoline functions,
  * input hooking,
  * **mono.dll** hooking,
  * **d3d11** hooking.
  
Function signatures were found by memory scanning using _Cheat Engine_.   
The _x86_ project was compiled using _Microsoft Visual Studio Community 2017_ on _Windows 10_.

### Extras

While reverse engineering the game I found a left over developer console which can be enabled through the GUI. I also found unused weapons, which can be equipped using the GUI weapon selector.
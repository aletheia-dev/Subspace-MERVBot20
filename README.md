# Subspace-MERVBot20
An expendable bot core for the multiplayer online game Subspace/Continuum

## About
This Subspace/Continuum bot core is based on MERVBot v6.8 by Catid. The original 32 bit code has been vastly refactored for 64 bit with the use of the latest coding paradigms of C++ 20.

## Setup

- Edit the configuration file `Spawns.txt`:
    - Read the directions and delete the example spawns at the very top.
    - Add your own spawn(s) in the proper format. To be able to operate on a subgame2 server, the bot needs at least smod privilegues, so you have to either provide the smod server password in the format `<bot passwd>*<smod passwd>` or add the bot's name to the server configuration file `smod.txt`.

- Edit the configuration file `Operators.txt`:
    - Read the directions and delete the example operators at the very top.
    - Add your player name in the proper format.

- Edit the configuration file `MERVBot20.INI`:
    - In Continuum, right-click the zone you want to have the bot enter and write down its IP address and port. 
    - Replace the default IP address and port in `Zone=127.0.0.1:2000` of the `[Login]` section.
    - Note: If the zone is running on a subgame2 server that is configured as Continuum-only, add the bot's name to the server configuration file `vip.txt`.

## Build

- Open the Microsoft Visual Studio 2022 solution `MERVBot20.sln`.
- Make sure the configuration `Release` and the platform `x64` are selected.
- Build the example bot plugin `ExampleBot`. Related projects within the solution will be built automatically.

## Run

- Run the bot either from Visual Studio or by double-clicking `_out\Release\MERVBot20.exe`.
- In the Subspace/Continuum chat, type /!login <your operator password> to gain access to the bot. If you didn't set a password for your name in `Operators.txt`, you will be logged in automatically.
- When you are done using the bot, select the console window and press a key to close it. CAUTION: Do not close the console window to shut down the bot.

See [Frequently Asked Questions](./doc/faq.md) for additional infos on troubleshooting.

## Thanks
This work has been done out of appreciation for Catid's remarkable effort two decades ago to provide us with a bot core for this wonderful game. Also thanks to all those coders and hackers who contributed to the original project, like Snrrrub, OmegaFirebolt, SOS, Coconut Emulator, Ave-iator, Cyan~Fire and BaK. 

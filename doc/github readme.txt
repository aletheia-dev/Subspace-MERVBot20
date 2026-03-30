An expendable bot core for the multiplayer online game Subspace/Continuum.

## About
This Subspace/Continuum bot core is based on *MERVBot v6.8 by Catid*. The original 32 bit code has been vastly refactored for 64 bit with the use of the latest coding paradigms of the C++ 20 standard.

- See [Frequently Asked Questions](./doc/faq.md) for additional infos on troubleshooting.

## Setup

- Edit the configuration file `Spawns.txt`:
    - Read the directions and delete the example spawns at the very top.
    - Add your own spawn(s) in the proper format.

- Edit the configuration file `Operators.txt`:
    - Read the directions and delete the example operators at the very top.
    - Add yourself in the proper format.

- Edit the configuration file `MERVBot20.INI`:
    - In Cointinuum, right-click the zone you want to have the bot enter and write down its IP address and port. 
    - Replace the default IP address and port in the definition `Zone=127.0.0.1:2000` of the `[Login]` section.
 
- If the zone is Continuum-only, add the bot's name to the server configuration file `VIP.txt`.

## Build

- Open the Microsoft Visual Studio 2022 solution `MERVBot20.sln`.
- Make sure the configuration 'Release' and the platform `x64` is selected.
- Build the example bot plugin `ExampleBot`.

## Run

- Run the bot either from Visual Studio or by double clicking `_out\Release\MERVBot20.exe`.
- Type /!login <your operator password> to gain access to the bot. If you didn't set a password for your name in `Operators.txt`, you will be logged in automatically.
- When you are done using the bot, make sure the console window is selected and press a key to close it.
- CAUTION: Do not close the console window to shut down the bot.

## Thanks
This work has been done out of appreciation for Catid's remarkable effort two decades ago to provide us with a bot core for this wonderful game. Also thanks to all those coders and hackers who contributed to the original project and who's names shall never be forgotten, like Snrrrub, OmegaFirebolt, Cyan~Fire and BaK.

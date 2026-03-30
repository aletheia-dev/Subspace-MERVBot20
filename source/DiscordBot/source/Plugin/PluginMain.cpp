import Spawn;

import <string>;


SpawnList<Spawn, Observable> spawns;


/// <summary>
/// Create a spawn for a given m_host. An instance of this class is created for each bot that is spawned to an arena.
/// </summary>
/// <param name="hostName">Host name.</param>
/// <param name="pluginName">Name of the plugin DLL.</param>
/// <param name="m_host">Observable m_host.</param>
extern "C" _declspec(dllexport)
Observable& __stdcall createSpawn(std::string hostName, std::string pluginName, Observable& obHost)
{
    return spawns.create(hostName, pluginName, obHost);
}


/// <summary>
/// Set up a spawn. The setup process is a separate step to enable the creator of the spawn to receive events that 
/// are issued during the setup process.
/// </summary>
/// <param name="hostName">Host name.</param>
/// <param name="pluginName">Name of the plugin DLL.</param>
extern "C" _declspec(dllexport)
void __stdcall setupSpawn(std::string hostName, std::string pluginName)
{
    spawns.setup(hostName, pluginName);
}

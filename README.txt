 ________   __                               ___       __________        __   
 \______ \ |__| ______ ____  ___________  __| _/       \______   \ _____/  |_ 
  |    |  \|  |/  ___// ___\/  _ \_  __ \/ __ |  ______ |    |  _//  _ \   __\
  |    |   \  |\___ \\  \__(  <_> )  | \/ /_/ | /_____/ |    |   (  <_> )  |  
 /_______  /__/____  >\___  >____/|__|  \____ |         |______  /\____/|__|  
         \/        \/     \/                 \/                \/             


To run DiscordBot, perform the following setup:

Discord Developer Portal:
- create a new bot with the name DiscordBot
- in Applications > DiscordBot > OAuth2 > URL Generator, authenticate the bot for the scopes 
  "bot" and "application.commands" and "Administrator" permissions
- in Applications > DiscordBot > Bot, grant all three "Privileged Gateway 
  Intents"
- in Applications > DiscordBot > OAuth2 > URL Generator, copy the "GENERATED URL" and enter it into a new browser tab.

Discord Server:
- create a new channel
- in the channel permissions, grant "Create Public Threads" and "Use 
  Application Commands"

DiscordBot.ini:
- set BotToken to the bot token from Discord Developer Portal > Reset Token
- set ServerId to the value of menu item *Discord Server* > Copy ID
- set ChatChannelId to the value of menu item *Channel* > Copy ID

# Libra

## How to Use

> This project was developed using Visual Studio, and it is recommended that Visual Studio and the MSVC toolchain be used for compiling from source. For best results, use VS2022.

The project requires my [custom C++ game engine](https://github.com/shreyasnisal/GameEngine) and a specific directory structure:

```
Engine
└── Code
Libra
├──Code
└──Run
```

The game contains multiple maps that are semi-procedurally generated using definitions in MapDefnitions.xml and use tiles defined in TileDefinitions.xml. Defined maps must be added to the game by adding their names in the list of maps in GameConfig.xml and the same type of map can be added multiple times.

Other game design values can also be updated in GameConfig.xml and maps can also use .png images that specify tiles to be used at specific coordinates on the map. To get a tile to show up with a probability, use a lower alpha in the map image (alpha = 127 => 0.5 probability of the tile being of the type specified in the image).

Hot reloading the game using the F11 key (see Developer Cheats section below) only reloads certain values in GameConfig.xml that are fetched on each frame. There is currently no option to completely reload all XML files and reset the game.

Enemies taking shortcuts is not very refined yet, but can be enabled through GameConfig.xml.

The following table gives the key-bindings for different screens in the game:

### Attract Screen

| Key           | Action                            |
|---------------|-----------------------------------|
| P             | Start game                        |
| Xbox Start    | Start game                        |
| M             | Play a test sound                 |
| Escape        | Exit game                         |
| Xbox Back     | Exit game

### Game

| Key                   | Action                                                                                                |
|-----------------------|-------------------------------------------------------------------------------------------------------|
| E                     | Set the goal direction for player tank to Vec2::NORTH (0.f, 1.f)                                      |
| S                     | Set the goal direction for player tank to Vec2::WEST(-1.f, 0.f)                                       |
| F                     | Set the goal direction for player tank to Vec2::EAST(1.f, 0.f)                                        |
| D                     | Set the goal direction for player tank to Vec2::SOUTH(0.f, -1.f)                                      |
| I                     | Set the goal direction for turret to Vec2::NORTH (0.f, 1.f)                                           |
| J                     | Set the goal direction for turret to Vec2::WEST(-1.f, 0.f)                                            |
| K                     | Set the goal direction for turret to Vec2::EAST(1.f, 0.f)                                             |
| L                     | Set the goal direction for turret to Vec2::SOUTH(0.f, -1.f)                                           |
| H                     | Toggle the selected weapon between bullet and flamethrower                                            |
| Escape                | Pause the game (if unpaused), Exit to attract screen                                                  |
| P                     | Toggle the game's paused state                                                                        |
| Xbox Left Joystick    | Set the goal direction for player tank to joystick direction (magnitude is used to affect speed)      |
| Xbox Right Joystick   | Set the goal direction for turret to right joystick direction                                         |
| Xbox Start            | Toggle the game's paused state                                                                        |
| Xbox Back             | Pause the game (if unpaused), Exit to attract screen (if paused)                                      |

**Note** : When the "Goal Direction" for the tank or turret is set, the tank rotates towards this direction and moves forward (on its forward direcction on any given frame) or the turret rotates to the goal direction as long as the button is held down. When the button is released, the tank/turret stops rotating and translating.

In addition to the above controls available to the player, the game also offers a number of *developer cheats* that can be used for effective in-game information and debugging. The below table lists the available developer cheats. These cheats are only available in-game (not available on the attract screen, defeat screen or victory screen).

| Key               | Action                                                                                        |
|-------------------|-----------------------------------------------------------------------------------------------|
| T                 | Slows the game down to 1/10th of the normal speed (music playback speed is decreased to 0.5x  |
| Y                 | Speeds the game up to 4x the normal speed (music playback speed is increased to 2x)           |
| O                 | Runs a single frame of the game and then pauses the game                                      |
| F1                | Toggles the *Debug Draw* mode which shows additional debug information for all entities       |
| F2                | Toggles invincibility mode                                                                    |
| F3                | Toggles the *No-Clip* mode which allows the player tank to move through stone tiles           |
| F4                | Toggles a Zoomed-out camera that shows the entire map                                         |
| F5                | Switches to previous map                                                                      |
| F6                | Toggles heat map modes                                                                        |
| Right Arrow Key   | Change selected enemy for enemy heatmap to goal position                                      |
| F8                | Completely resets the game (Destroys the instance of Game class and initializes it again)     |
| F9                | Advances game to next map                                                                     |
| F11               | Hot reloads GameConfig.xml                                                                    |
| `                 | Opens the Dev Console                                                                         |

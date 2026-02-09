# Compiling On Linux (Debian 10 Buster)

1. Install dependencies (Commands on MX 19.4_x64 patito feo / Debian 10 buster): `sudo apt update && sudo apt install libxfce4panel-2.0-dev libxfce4ui-2-dev libgtk-3-dev curl libnotify-dev`
2. Clone this repo
3. Change into this repo directory, then run: `make`
4. Run: `sudo make install`
5. Run: `xfce4-panel -r`

# Usage

You should now see the Compass navigator icon on your panel. Right-click to choose "**Refresh IP Now**" to update it with your public IP, then you can  hover or left-click to copy it to your clipboard.

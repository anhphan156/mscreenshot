# Mscreenshot
A tool that overlay a meme on top of your screenshot. It uses slurp and grim to take a screenshot, then compose an image with imagemagick. And, it only works on wayland.

![Imgur](video.gif)

# Build
You can grab the dependencies listed in the flake.nix file then do the meson ninja thingy, or you can just hit nix build I guess.

# Usage
Feed a config file to the program via the `MEME_SCREENSHOT_CONFIG` env var. Run the program with `/path/to/mscreenshot -x template -o /path/to/output`.
An example of a config file is below. Anchor is the corner where the image is supposed to go. Pivot doesn't do anything at the moment. Scale is scale. Path is, well, path.
```json
{
  "anya": {
    "stickers": [
      {
        "anchor": 3,
        "path": "/nix/store/blw49d3snaj29z0bbzb6hlrk9riykgrn-Wallpapers/stickers/anya.png",
        "pivot": 3,
        "scale": 0.4
      }
    ]
  },
  "sparkle": {
    "stickers": [
      {
        "anchor": 3,
        "path": "/nix/store/blw49d3snaj29z0bbzb6hlrk9riykgrn-Wallpapers/stickers/sparkle.png",
        "pivot": 3,
        "scale": 1
      }
    ]
  },
  "yae": {
    "stickers": [
      {
        "anchor": 3,
        "path": "/nix/store/blw49d3snaj29z0bbzb6hlrk9riykgrn-Wallpapers/stickers/twosoyjaklumine.png",
        "pivot": 3,
        "scale": 0.4
      },
      {
        "anchor": 2,
        "path": "/nix/store/blw49d3snaj29z0bbzb6hlrk9riykgrn-Wallpapers/stickers/twosoyjakyae.png",
        "pivot": 2,
        "scale": 0.4
      }
    ]
  }
}
```

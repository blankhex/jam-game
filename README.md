# Game

Game should be available here: https://blankhex.github.io/jam-game/eulamadness.html

# Building

## Common

Build data archive

```sh
python3 makepak.py Data data.pak
```

## Linux/MacOS
 
```sh
make clean all
```

## Emscripten

```sh
emmake make -f Makefile.emcc clean all
```
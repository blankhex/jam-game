# Building

## Common

Build data archive

```sh
python3 makepak.py Data data.pak
```

## Linux/MacOS
 
```sh
makefile clean all
```

## Emscripten

```sh
emmake make -f Makefile.emcc clean all
```
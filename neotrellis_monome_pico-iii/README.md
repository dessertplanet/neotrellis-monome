# Neotrellis Grid Code for Raspberry Pi Pico/RP2040 boards  


This version is rewritten to use the Raspberry Pi Pico VS Code Extension and includes the monome iii scripting 

Install the Raspberry Pi Pico VS Code Extension from the Extensions tab in VSCode.  
![alt text](PicoVSCodeExtn.png)

### Configuration

Look at the `# UPDATE HERE FOR YOUR BOARD AND BUILD` sections of `CMakeLists.txt` to configure for your specific board. This *should* be the only configuration changes needed.

`# SET BOARD TYPE`

pico  (included in Pico SDK)  
adafruit_kb2040  (included in Pico SDK)  
board_dinkii  (included in neotrellis repo)  
adafruit_feather_rp2040  (included in Pico SDK)   

`# UPDATE HERE FOR YOUR BOARD AND BUILD`
`# Build defines`

Set these according to your specific build  
```
    BOARDTYPE=2  # must be number - options are:  1=PICO, 2=KB2040QT, 3=DINKII, 4=FEATHER2040QT
    GRIDCOUNT=3   # must be number - options are:  1=4X4, 2=8x8, 3=16x8, 4=16x16 
```

### i2c address configuration

If you're not using the default i2c address configuration, you might need to change i2c addresses in `config.h` to match your boards


### iii  

iii is an interactive scripting environment that runs on the device itself.  With grid, this can turn the device into a user-scriptable midi controller/sequencer.

See https://github.com/monome/iii for documentation.  

The `diii` REPL tool is hosted at https://monome.org/diii


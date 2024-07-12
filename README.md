EXPERIMENTAL prores_encoder (FFMPEG backend) for Davinci Resolve Studio (based on Black Magic's x264_encoder_plugin sample)

[WINDOWS]

To compile this under WINDOWS, you'll need the following tools installed

1) Visual Studio 2022
2) MSYS2

Instructions:

[Pre-Req]

Create Directory C:\VideoEditingUtils\prores_plugin_build

[Download ffmpeg]

1) run "MSYS2 MSYS" - type it in the search bar in the Windows Task Bar
2) pacman -Sy pacman
3) pacman -Syu
4) pacman -Su
5) pacman -S nasm
6) pacman -S make
7) pacman -S git
8) cd /C/VideoEditingUtils/prores_plugin_build   
90) git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg

[Compile ffmpeg]

1) run the "x64 Native Tools Command Prompt for VS 2022" - type it in the seach bar in the Windows Task Bar
2) Inside the command prompt, type "cd /d C:\msys64"
3) Execute the command "msys2_shell.cmd -mingw64 -full-path"
4) This will open up another terminal, type in the following commands (in the new terminal)
5) cd /C/VideoEditingUtils/prores_plugin_build/ffmpeg
6) CC=cl ./configure --toolchain=msvc  --pkg-config-flags=--static --disable-programs --disable-shared --enable-static --extra-ldflags=-static --enable-encoder=prores --prefix=/C/VideoEditingUtils/prores_plugin_build/ffmpeg_pkg
7) make V=1 -j 8
Ad
[Download prores_encoder]

1) run "MSYS2 MSYS" - type it in the search bar in the Windows Task Bar
2) cd /C/VideoEditingUtils/prores_plugin_build
3) git clone https://github.com/UDaManFunks/prores_encoder

[Compile prores_encoder]

1) run the "x64 Native Tools Command Prompt for VS 2022" - type it in the seach bar in the Windows Task Bar
2) Inside the command prompt, type "cd /d C:\VideoEditingUtils\x265_plugin_build\prores_encoder"
3) nmake /f NMakefile
   
[Packaging / Installing]

1) Create folder called "IOPlugins" under %PROGRAMDATA%\Blackmagic Design\DaVinci Resolve\Support

  Note: you can open up %PROGRAMDATA% folder via explorer by typing it verbatim in a run window (Win + R) 

2) Create a folder named "prores_encoder.dvcp.bundle" under the IOPlugins folder
3) Create a folder named "Contents" under the "prores_encoder.dvcp.bundle" folder
4) Create a folder named "Win64" under the "Contents" folder
5) Copy the built plugin from C:\VideoEditingUtils\prores_plugin_build\prores_encoder\bin\prores_encoder.dvcp" and place it in the "Win64" folder (which you've created via Step 1-4)
6) Start Davinci Resolve Studio
   
You can export using PRORES if you pick "QUICKTIME" as your FORMAT in Davnci Resolve, then selecting the "ProRes" Codec option.
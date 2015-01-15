: The following file renaming is needed if one want to compile all files 
: using the same output directory, e.g. "Debug\" or "Release\". 
:
: Usage:
: copy all src WebP files into src\, then, run this script to rename files
:

: dec\

copy /Y dec\alpha.c dec\dec_alpha.c
copy /Y dec\frame.c dec\dec_frame.c
copy /Y dec\quant.c dec\dec_quant.c
copy /Y dec\tree.c dec\dec_tree.c
copy /Y dec\vp8l.c dec\dec_vp8l.c

del /Q dec\alpha.c
del /Q dec\frame.c
del /Q dec\quant.c
del /Q dec\tree.c
del /Q dec\vp8l.c

: enc\

copy /Y enc\alpha.c enc\enc_alpha.c
copy /Y enc\frame.c enc\enc_frame.c
copy /Y enc\quant.c enc\enc_quant.c
copy /Y enc\tree.c enc\enc_tree.c
copy /Y enc\vp8l.c enc\enc_vp8l.c

del /Q enc\alpha.c
del /Q enc\frame.c
del /Q enc\quant.c
del /Q enc\tree.c
del /Q enc\vp8l.c

: Makefiles

del /S /Q Makefile.am
del /S /Q *.pc.in

pause -1

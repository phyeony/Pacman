TARGETS= main
DEPS= ghost.c map.c gameManager.c ledDisplay.c utility.c joycon.c udp.c shutdown.c zenCapeJoystick.c types.h wave_player.c

OUTFILE = game
OUTDIR = $(HOME)/cmpt433/public/myApps

CROSS_COMPILE = arm-linux-gnueabihf
CC_C = $(CROSS_COMPILE)-gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -Wshadow

LFLAGS = -L$(HOME)/cmpt433/public/asound_lib_BBB

all: $(TARGETS)

$(TARGETS):
	$(CC_C) $(CFLAGS) $@.c $(DEPS) -o $(OUTDIR)/$@ -lpthread -lusb-1.0 -lasound $(LFLAGS)
clean:
	rm $(OUTDIR)/$(OUTFILE)

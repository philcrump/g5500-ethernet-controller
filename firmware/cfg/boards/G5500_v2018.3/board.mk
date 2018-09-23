# List of all the board related files.
BOARDSRC = $(CONFDIR)/boards/G5500_v2018.3/board.c

# Required include directories
BOARDINC = $(CONFDIR)/boards/G5500_v2018.3

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)

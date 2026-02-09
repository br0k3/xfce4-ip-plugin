# Plugin details
PLUGIN_NAME = ip_plugin
SOURCES     = ip-panel-plugin.c
DESKTOP_FILE = ip-plugin.desktop

# Tools and Flags
CC      = gcc
# Standard production flags
CFLAGS  = -fPIC -Wall `pkg-config --cflags libxfce4panel-2.0 libxfce4ui-2-0 libxfce4util-1.0 gtk+-3.0 libnotify`
LIBS    = `pkg-config --libs libxfce4panel-2.0 libxfce4ui-2-0 libxfce4util-1.0 gtk+-3.0 libnotify`

# System Paths
PREFIX       = /usr
LIBDIR       = $(PREFIX)/lib/x86_64-linux-gnu/xfce4/panel/plugins
DATADIR      = $(PREFIX)/share/xfce4/panel/plugins
TARGET_SO    = lib$(PLUGIN_NAME).so

.PHONY: all clean install uninstall debug

# Default target (Production build)
all: $(TARGET_SO)

# Compile the shared library
$(TARGET_SO): $(SOURCES)
	$(CC) -shared $(CFLAGS) -o $@ $^ $(LIBS)

# --- DEBUGGING TARGET ---
# Appends debug symbols (-g) and disables optimization (-O0)
# Then forces a recompile of the library.
debug: CFLAGS += -g -O0
debug: clean $(TARGET_SO)
	@echo "Debug build complete with symbols (-g)."

# --- INSTALLATION TARGETS ---
install: all
	@echo "Installing to $(LIBDIR) and $(DATADIR)..."
	install -d $(DESTDIR)$(LIBDIR)
	install -m 755 $(TARGET_SO) $(DESTDIR)$(LIBDIR)
	install -d $(DATADIR)
	install -m 644 $(DESKTOP_FILE) $(DATADIR)
	@echo "Installation complete. Restart your panel with: xfce4-panel -r"

uninstall:
	rm -f $(DESTDIR)$(LIBDIR)/$(TARGET_SO)
	rm -f $(DATADIR)/$(DESKTOP_FILE)
	@echo "Uninstalled."

clean:
	rm -f $(TARGET_SO)

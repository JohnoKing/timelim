# Copyright (c) 2018-2019 Johnothan King. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Variables (each one may be overridden)
CC      := cc
STRIP   := strip
CFLAGS  := -O2 -ffast-math -fomit-frame-pointer -fPIC -pipe
WFLAGS  := -Wall -Wextra -Wpedantic
LDFLAGS := -Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now

# Compile
all:
	@$(CC) $(CFLAGS) $(WFLAGS) -o timelim timelim.c $(LDFLAGS)
	@$(STRIP) --strip-unneeded -R .comment -R .gnu.version -R .GCC.command.line -R .note.gnu.gold-version timelim
	@echo "Successfully built timelim!"

# Install
install: all
	@mkdir -p "$(DESTDIR)/usr/local/bin"
	@install -Dm0755 timelim "$(DESTDIR)/usr/local/bin"
	@echo "Successfully installed timelim!"

# Uninstall
uninstall:
	@rm -f "$(DESTDIR)/usr/local/bin/timelim"
	@echo "Successfully uninstalled timelim!"

# Cleanup
clean:
	@rm -rf timelim

# Calls clean, then resets the git repo
clobber: clean
	git reset --hard

# Alias for clobber
mrproper: clobber

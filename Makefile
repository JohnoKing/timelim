# Copyright © 2018-2020 Johnothan King. All rights reserved.
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
CC       := cc
STRIP    := strip
CFLAGS   := -O2 -ffast-math -fomit-frame-pointer -fpic -fno-plt -pipe
CPPFLAGS := -D_FORTIFY_SOURCE=2
WFLAGS   := -Wall -Wextra -Wno-unused-result
LDFLAGS  := -Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now
prefix   := /usr/local

# Compile timelim
all:
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(WFLAGS) -o timelim timelim.c $(LDFLAGS)
	@$(STRIP) --strip-unneeded -R .comment -R .gnu.version -R .GCC.command.line -R .note.gnu.gold-version timelim || true
	@echo "Successfully built timelim!"

# Install
install: all
	@mkdir -p "$(DESTDIR)/$(prefix)/bin" "$(DESTDIR)/$(prefix)/share/man/man1" "$(DESTDIR)/$(prefix)/share/licenses/timelim"
	@install -Dm0755 timelim "$(DESTDIR)/$(prefix)/bin"
	@install -Dm0644 timelim.1 "$(DESTDIR)/$(prefix)/share/man/man1"
	@install -Dm0644 LICENSE "$(DESTDIR)/$(prefix)/share/licenses/timelim"
	@echo "Successfully installed timelim!"

# Uninstall
uninstall:
	@rm -rf "$(DESTDIR)/$(prefix)/bin/timelim" "$(DESTDIR)/$(prefix)/share/man/man1/timelim.1" "$(DESTDIR)/$(prefix)/share/licenses/timelim"
	@echo "Successfully uninstalled timelim!"

# Cleanup
clean:
	@rm -rf timelim
	@git gc 2> /dev/null
	@git repack >> /dev/null 2> /dev/null

# Calls clean, then resets the git repo
clobber: clean
	git reset --hard

# Alias for clobber
mrproper: clobber

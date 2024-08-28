include common.mk
TD:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
CHANGELOG_VERSION=$(shell grep '^\#\# \[[0-9]' CHANGELOG.md | sed 's/\#\# \[\([^]]\{1,\}\)].*/\1/' | head -n1)
GIT_VERSION_TAG=$(shell git tag --points-at HEAD 2>/dev/null | grep "v[0-9]" | sed -e 's/^v//')

ifdef HOME
ZIG_LOCAL_CACHE_DIR ?= $(HOME)/.cache/acton/zig-local-cache
else
# TODO: Windows?
ZIG_LOCAL_CACHE_DIR ?= $(TD)/zig-cache
endif
export ZIG_LOCAL_CACHE_DIR

ACTON=$(TD)/dist/bin/acton
ACTONC=dist/bin/actonc
ACTC=$(TD)/dist/bin/actonc
ZIG_VERSION:=0.13.0
ZIG=$(TD)/dist/zig/zig
AR=$(ZIG) ar
CC=$(ZIG) cc
CXX=$(ZIG) c++
export CC
export CXX

# Determine which xargs we have. BSD xargs does not have --no-run-if-empty,
# rather, it is the default behavior so the argument is superfluous. We check if
# we are using GNU xargs by trying to run xargs --version and grep for 'GNU', if
# that returns 0 we are on GNU and will use 'xargs --no-run-if-empty', otherwise
# we are on BSD and will use 'xargs' straight up.
XARGS_CHECK := $(shell xargs --version 2>&1 | grep GNU >/dev/null 2>&1; echo $$?)
ifeq ($(XARGS_CHECK),0)
	XARGS := xargs --no-run-if-empty
else
	XARGS := xargs
endif

# This is the version we will stamp into actonc
BUILD_TIME=$(shell date "+%Y%m%d.%-H.%-M.%-S")
ifdef BUILD_RELEASE
export VERSION_INFO?=$(VERSION)
export DEB_DIST=stable
else
export VERSION_INFO?=$(VERSION).$(BUILD_TIME)
export DEB_DIST=tip
endif

ifdef CPEDANTIC
CPEDANTIC=--cpedantic
endif

# rewrite arm64 to aarch64
ifeq ($(shell uname -m),arm64)
ARCH:=aarch64
else
ARCH:=$(shell uname -m)
endif

# -- Apple Mac OS X ------------------------------------------------------------
ifeq ($(shell uname -s),Darwin)
OS:=macos
ifeq ($(shell uname -m),arm64)
ZIG_CPU := -Dcpu=apple_a15
ZIG_TARGET := -Dtarget=aarch64-macos
else
ZIG_TARGET := -Dtarget=x86_64-macos
endif
endif

# -- Linux ---------------------------------------------------------------------
ifeq ($(shell uname -s),Linux)
OS:=linux
ifeq ($(shell uname -m),x86_64)
ZIG_TARGET := -Dtarget=x86_64-linux-gnu.2.27
ZIG_CPU := -Dcpu=skylake
ACTONC_TARGET := --target x86_64-linux-gnu.2.27
else
$(error "Unsupported architecture for Linux?")
endif
endif # -- END: Linux ----------------------------------------------------------

.PHONY: all
all: version-check
	$(MAKE) $(DIST_ZIG)
	$(MAKE) distribution

.PHONY: help
help:
	@echo "Available make targets:"
	@echo "  all     - build everything"
	@echo "  test    - run the test suite"
	@echo ""
	@echo "  clean   - /normal/ clean repo"
	@echo "  clean-all - thorough cleaning"
	@echo "  clean-downloads - remove downloaded deps, normally never needed?"


.PHONY: version-check
version-check:
ifneq ($(VERSION), $(CHANGELOG_VERSION))
	$(error Version in common.mk ($(VERSION)) differs from last version in CHANGELOG.md ($(CHANGELOG_VERSION)))
endif
ifneq ($(GIT_VERSION_TAG),) # if we are on a git tag..
ifneq ($(VERSION),$(GIT_VERSION_TAG)) # ..ensure the git tag is same as version in common.mk
	$(error Current git tag ($(GIT_VERSION_TAG)) differs from version in common.mk ($(VERSION)))
endif
endif

BUILTIN_HFILES=$(wildcard base/builtin/*.h)

DIST_BINS=$(ACTONC) dist/bin/actondb dist/bin/runacton
DIST_ZIG=dist/zig

.PHONY: test-backend
test-backend: $(BACKEND_TESTS)
	@echo DISABLED TEST: backend/failure_detector/db_messages_test
	./backend/test/actor_ring_tests_local
	./backend/test/actor_ring_tests_remote
	./backend/test/db_unit_tests
	@echo DISABLED test: ./backend/test/queue_unit_tests
	./backend/test/skiplist_test

# /compiler ----------------------------------------------
ACTONC_ALL_HS=$(wildcard compiler/*.hs compiler/**/*.hs)
ACTONC_TEST_HS=$(wildcard compiler/tests/*.hs)
ACTONC_HS=$(filter-out $(ACTONC_TEST_HS),$(ACTONC_ALL_HS))
# NOTE: we're unsetting CC & CXX to avoid using zig cc & zig c++ for stack /
# ghc, which doesn't seem to work properly
compiler/actonc: compiler/package.yaml.in compiler/stack.yaml dist/builder $(ACTONC_HS) compiler/Acton/Builder.hs
	cd compiler && unset CC && unset CXX && unset CFLAGS && stack build --dry-run 2>&1 | grep "Nothing to build" || \
		(sed 's,^version:.*,version:      "$(VERSION_INFO)",' < package.yaml.in > package.yaml \
		&& stack build $(STACK_OPTS) --ghc-options='-j4 $(ACTC_GHC_OPTS)' \
		&& stack --local-bin-path=. install 2>/dev/null)

compiler/Acton/Builder.hs: builder/build.zig builder/build.zig.zon
# We need to generate a Haskell file from the zig file, so we can include it in the compiler
# Make sure to escape the double quotes in the zig file and replace them with \" in the Haskell file. We also need to handle newlines since Haskell strings what newlines to be escaped.
	(echo 'module Acton.Builder where'; \
		echo '\nbuildzig :: String'; /bin/echo -n 'buildzig = "'; cat builder/build.zig | sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/$$/\\n/' | tr -d '\n'; echo '"'; \
		echo '\nbuildzigzon :: String'; /bin/echo -n 'buildzigzon = "'; cat builder/build.zig.zon | sed -e 's/"/\\"/g' -e 's/$$/\\n/' | tr -d '\n'; echo '"') > compiler/Acton/Builder.hs

.PHONY: clean-compiler
clean-compiler:
	cd compiler && stack clean >/dev/null 2>&1 || true
	rm -f compiler/actonc compiler/package.yaml compiler/acton.cabal

# /deps --------------------------------------------------
DEPS += dist/deps/mbedtls
DEPS += dist/deps/libargp
DEPS += dist/deps/libbsdnt
DEPS += dist/deps/libgc
DEPS += dist/deps/libnetstring
DEPS += dist/deps/pcre2
DEPS += dist/deps/libprotobuf_c
DEPS += dist/deps/tlsuv
DEPS += dist/deps/libutf8proc
DEPS += dist/deps/libuuid
DEPS += dist/deps/libuv
DEPS += dist/deps/libxml2
DEPS += dist/deps/libyyjson
DEPS += dist/deps/libsnappy_c

.PHONE: clean-downloads
clean-downloads:
	rm -rf deps-download


# Explanation of the horrible hack found below:
# The symlink .build/sys that we create in each deps directory is to make zig
# build work. It normally uses absolute paths and so when it is in say deps/a
# and wants to go to deps/b, it'll just cd into the b dir, but since we use a
# relative dir (that is really meant to be relative to the build root, not each
# individual dependency), this breaks. We work around it by making all deps
# directory look like the build root in this sense, i.e. .build/sys points to
# syspath, the root of the system distribution.
# /deps/libargp --------------------------------------------
LIBARGP_REF=a30e99cda3fabc591727a8df3aee5524c2392e15
deps-download/$(LIBARGP_REF).tar.gz:
	mkdir -p deps-download
	curl -f -L -o $@ https://github.com/actonlang/argp-standalone/archive/$(LIBARGP_REF).tar.gz

dist/deps/libargp: deps-download/$(LIBARGP_REF).tar.gz
	mkdir -p $@
	cd $@ && tar zx --strip-components=1 -f $(TD)/$<
	rm -rf $@/testsuite
	touch $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

# /deps/libbsdnt --------------------------------------------
LIBBSDNT_REF=282f774e1e664ea7c23cc0bb9f313c1054874a97
deps-download/$(LIBBSDNT_REF).tar.gz:
	mkdir -p deps-download
	curl -f -L -o $@ https://github.com/actonlang/bsdnt/archive/$(LIBBSDNT_REF).tar.gz

dist/deps/libbsdnt: deps-download/$(LIBBSDNT_REF).tar.gz
	mkdir -p $@
	cd $@ && tar zx --strip-components=1 -f $(TD)/$<
	touch $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

# /deps/libgc --------------------------------------------
LIBGC_REF=0a23b211b558137de7ee654c5527a54113142517
deps-download/$(LIBGC_REF).tar.gz:
	mkdir -p deps-download
	curl -f -L -o $@ https://github.com/actonlang/bdwgc/archive/$(LIBGC_REF).tar.gz

dist/deps/libgc: deps-download/$(LIBGC_REF).tar.gz
	mkdir -p $@
	cd $@ && tar zx --strip-components=1 -f $(TD)/$<
	rm -rf $@/tests $@/tools
	touch $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

# /deps/libmbedtls --------------------------------------------
LIBMBEDTLS_REF=e72756f2312f04b659fdeaba2fbba7b1f5fd3927
deps-download/$(LIBMBEDTLS_REF).tar.gz:
	mkdir -p deps-download
	curl -f -L -o $@ https://github.com/actonlang/mbedtls/archive/$(LIBMBEDTLS_REF).tar.gz

dist/deps/mbedtls: deps-download/$(LIBMBEDTLS_REF).tar.gz
	mkdir -p $@
	cd $@ && tar zx --strip-components=1 -f $(TD)/$<
	touch $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

# /deps/libprotobuf_c --------------------------------------------
LIBPROTOBUF_C_REF=4e4bfc7ec44e6ac746b05f3251f59610822bc95c
deps-download/$(LIBPROTOBUF_C_REF).tar.gz:
	mkdir -p deps-download
	curl -f -L -o $@ https://github.com/actonlang/protobuf-c/archive/$(LIBPROTOBUF_C_REF).tar.gz

dist/deps/libprotobuf_c: deps-download/$(LIBPROTOBUF_C_REF).tar.gz
	mkdir -p $@
	cd $@ && tar zx --strip-components=1 -f $(TD)/$<
	touch $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

# /deps/tlsuv ---------------------------------------------
TLSUV_REF=5a811cce1efc360a2bd4fe11ada581ceb2a1c764
deps-download/$(TLSUV_REF).tar.gz:
	mkdir -p deps-download
	curl -f -L -o $@ https://github.com/actonlang/tlsuv/archive/$(TLSUV_REF).tar.gz

dist/deps/tlsuv: deps-download/$(TLSUV_REF).tar.gz dist/deps/libuv dist/deps/mbedtls
	mkdir -p $@
	cd $@ && tar zx --strip-components=1 -f $(TD)/$<
	touch $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

# /deps/libutf8proc --------------------------------------
LIBUTF8PROC_REF=e914c63b43d5f283090a63a307fccd25acbe37f0
deps-download/$(LIBUTF8PROC_REF).tar.gz:
	mkdir -p deps-download
	curl -f -L -o $@ https://github.com/actonlang/utf8proc/archive/$(LIBUTF8PROC_REF).tar.gz

dist/deps/libutf8proc: deps-download/$(LIBUTF8PROC_REF).tar.gz
	mkdir -p $@
	cd $@ && tar zx --strip-components=1 -f $(TD)/$<
	touch $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

# /deps/libuuid ------------------------------------------
dist/deps/libuuid: deps/libuuid
	mkdir -p $(TD)/$@
	cp -a $</* $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

# /deps/libuv --------------------------------------------
LIBUV_REF=f20620733fb8fb5fb261699bbb858887ac6ec0bb
deps-download/$(LIBUV_REF).tar.gz:
	mkdir -p deps-download
	curl -f -L -o $@ https://github.com/actonlang/libuv/archive/$(LIBUV_REF).tar.gz

dist/deps/libuv: deps-download/$(LIBUV_REF).tar.gz
	mkdir -p $@
	cd $@ && tar zx --strip-components=1 -f $(TD)/$< libuv-$(LIBUV_REF)/build.zig libuv-$(LIBUV_REF)/include libuv-$(LIBUV_REF)/src
	touch $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

# /deps/libxml2 ------------------------------------------
LIBXML2_REF=56e4e62c077b2c5285b0eec4d6d4497f9b2e6e8f
deps-download/$(LIBXML2_REF).tar.gz:
	mkdir -p deps-download
	curl -f -L -o $@ https://github.com/actonlang/libxml2/archive/$(LIBXML2_REF).tar.gz

dist/deps/libxml2: deps-download/$(LIBXML2_REF).tar.gz
	mkdir -p $@
	cd $@ && tar zx --strip-components=1 -f $(TD)/$<
	rm -rf $@/doc $@/example $@/fuzz $@/os400 $@/python $@/test*
	touch $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

# /deps/pcre2 --------------------------------------------
LIBPCRE2_REF=2afc8e2c87e53204e08e5e1333a8e14ecbf5e3a2
deps-download/$(LIBPCRE2_REF).tar.gz:
	mkdir -p deps-download
	curl -f -L -o $@ https://github.com/actonlang/pcre2/archive/$(LIBPCRE2_REF).tar.gz

dist/deps/pcre2: deps-download/$(LIBPCRE2_REF).tar.gz
	mkdir -p $@
	cd $@ && tar zx --strip-components=1 -f $(TD)/$<
	touch $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

# /deps/libsnappy_c --------------------------------------------
LIBSNAPPY_C_REF=3f5b95957558a35c2becbe6b628c8219477dd5a4
deps-download/$(LIBSNAPPY_C_REF).tar.gz:
	mkdir -p deps-download
	curl -f -L -o $@ https://github.com/actonlang/snappy/archive/$(LIBSNAPPY_C_REF).tar.gz

dist/deps/libsnappy_c: deps-download/$(LIBSNAPPY_C_REF).tar.gz
	mkdir -p $@
	cd $@ && tar zx --strip-components=1 -f $(TD)/$<
	touch $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

dist/deps/libnetstring: deps/libnetstring $(DIST_ZIG)
	mkdir -p $(TD)/$@
	cp -a $</* $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work

dist/deps/libyyjson: deps/libyyjson $(DIST_ZIG)
	mkdir -p $(TD)/$@
	cp -a $</* $(TD)/$@
	mkdir -p $@/.build
	ln -s ../../../ $@/.build/sys # horrible hack to make zig build hack work


ifeq ($(ARCH),x86_64)
ZIG_ARCH_ARG=-mcpu=x86_64
endif
builder/builder: builder/build.zig builder/build.zig.zon builder/dependencies.zig backend/build.zig base/build.zig $(ZIG_DEP) $(DEPS) dist/base $(DIST_ZIG)
	rm -rf builder/zig-cache builder/zig-out
	cd builder && $(ZIG) build-exe -femit-bin=builder $(ZIG_ARCH_ARG) --dep @build --dep @dependencies --mod root ../dist/zig/lib/compiler/build_runner.zig --mod @build ./build.zig --mod @dependencies ./dependencies.zig

.PHONY: base/out/types/__builtin__.ty
base/out/types/__builtin__.ty: $(ACTONC) $(DEPS)
	cd base && ../dist/bin/actonc build --auto-stub --skip-build $(CPEDANTIC)

# top level targets

.PHONY: test test-builtins test-compiler test-db test-examples test-lang test-regressions test-rts test-stdlib
test:
	cd compiler && stack test
	$(MAKE) test-stdlib
	$(MAKE) -C backend test
	$(MAKE) test-rts-db

test-builtins:
	cd compiler && stack test --ta '-p "Builtins"'

test-compiler:
	cd compiler && stack test --ta '-p "compiler"'

test-cross-compile:
	cd compiler && stack test --ta '-p "cross-compilation"'

test-typeerrors:
	cd compiler && stack test --ta '-p "type errors"'

test-typeerrors-accept:
	cd compiler && stack runghc -- test.hs -p "type errors" --accept

test-db:
	cd compiler && stack test --ta '-p "DB"'

test-examples:
	cd compiler && stack test --ta '-p "Examples"'

test-lang:
	cd compiler && stack test --ta '-p "Core language"'

test-regressions:
	cd compiler && stack test --ta '-p "Regression"'

test-rts:
	cd compiler && stack test --ta '-p "RTS"'

test-rts-db:
	$(MAKE) -C test

test-stdlib: dist/bin/acton
	cd compiler && stack test --ta '-p "stdlib"'
	cd test/stdlib_tests && $(ACTON) test


.PHONY: clean clean-all clean-base
clean: clean-cli clean-distribution clean-base
	rm -rf compiler/Acton/Builder.hs

clean-cli:
	rm -rf cli/out

clean-all: clean clean-compiler
	rm -rf $(ZIG_LOCAL_CACHE_DIR)

clean-base:
	rm -rf base/out builder/builder* builder/zig-cache builder/zig-out

bin/acton: cli/out/bin/acton
	cp -a $< $@

cli/out/bin/acton: distribution1
	cd cli && $(ACTC) build $(ACTONC_TARGET)

# == DIST ==
#

BACKEND_FILES = backend/build.zig backend/build.zig.zon $(wildcard backend/*.c backend/*.h backend/failure_detector/*.c backend/failure_detector/*.h)
DIST_BACKEND_FILES = $(addprefix dist/,$(BACKEND_FILES)) dist/backend/deps dist/bin/actondb
dist/backend%: backend/%
	mkdir -p $(dir $@)
	cp -a $< $@
	mkdir -p dist/backend/.build
	ln -sf ../../ dist/backend/.build/sys || true

# We depend on __builtin__.ty because the base/out directory will be populated
# as a result of building it, and we want to copy those files!
.PHONY: dist/base
dist/base: base base/.build base/build.zig base/build.zig.zon dist/base/out/types/__builtin__.ty
	mkdir -p $@ $@/.build $@/out
	ln -sf ../../ dist/base/.build/sys || true
	cp -a base/Acton.toml base/build.zig base/build.zig.zon base/builtin base/rts base/src base/stdlib dist/base/
	cp -a base/out/types dist/base/out/

dist/bin/acton: bin/acton
	@mkdir -p $(dir $@)
	cp -a $< $@.tmp
	mv $@.tmp $@

# This does a little hack, first copying and then moving the file in place. This
# is to avoid an error if the executable is currently running. cp tries to open
# the file and modify it, which the Linux kernel (and perhaps others?) will
# prevent if the file to be modified is an executable program that is currently
# running.  We work around it by moving / renaming the file in place instead!
dist/bin/actonc: compiler/actonc $(DIST_ZIG)
	@mkdir -p $(dir $@)
	cp $< $@.tmp
	mv $@.tmp $@

#
dist/bin/actondb: $(DIST_ZIG) $(DEPS)
	@mkdir -p $(dir $@)
	cd dist/backend && $(ZIG) build -Donly_actondb --prefix $(TD)/dist

dist/bin/runacton: bin/runacton
	@mkdir -p $(dir $@)
	cp $< $@.tmp
	mv $@.tmp $@

dist/builder: builder/builder
	@mkdir -p $@
	cp -a builder/builder builder/*.zig builder/build.zig.zon $@/

DIST_DEPS=$(addprefix dist/deps/,libargp libbsdnt libgc libnetstring libprotobuf_c libutf8proc libuuid libuv libxml2 libyyjson pcre2 libsnappy_c)
dist/deps/%: deps/% $(DEPS)
	@mkdir -p $(dir $@)
	cp -a $< $@

dist/base/out/types/__builtin__.ty: base/out/types/__builtin__.ty
	@mkdir -p $(dir $@)
	cp -a $< $@

dist/completion/acton.bash-completion: completion/acton.bash-completion
	mkdir -p $(dir $@)
	cp $< $@

dist/zig: deps-download/zig-$(OS)-$(ARCH)-$(ZIG_VERSION).tar.xz
	mkdir -p $@
	cd $@ && tar Jx --strip-components=1 -f ../../$^
	rm -rf $@/doc
	cp -a deps/zig-extras/* $@


# Check if ZIG_VERSION contains -dev, in which case we pull down a nightly,
# otherwise its a release
deps-download/zig-$(OS)-$(ARCH)-$(ZIG_VERSION).tar.xz:
	mkdir -p deps-download
ifeq ($(findstring -dev,$(ZIG_VERSION)),-dev)
	curl -L -o $@ https://github.com/actonlang/zigballs/raw/main/zig-$(OS)-$(ARCH)-$(ZIG_VERSION).tar.xz
#	curl -o $@ https://ziglang.org/builds/zig-$(OS)-$(ARCH)-$(ZIG_VERSION).tar.xz
else
	curl -o $@ https://ziglang.org/download/$(ZIG_VERSION)/zig-$(OS)-$(ARCH)-$(ZIG_VERSION).tar.xz
endif

.PHONY: distribution1 distribution clean-distribution
distribution1: dist/base $(DIST_BACKEND_FILES) dist/builder $(DIST_BINS) $(DIST_ZIG)
	$(MAKE) $(DIST_DEPS)

distribution: dist/bin/acton

clean-distribution:
	rm -rf dist

# == release ==
# This is where we take our distribution and turn it into a release tar ball
GNU_TAR := $(shell sed --version 2>&1 | grep GNU >/dev/null 2>&1; echo $$?)
ifeq ($(GNU_TAR),0)
TAR_TRANSFORM_OPT=--transform 's,^dist,acton,'
else
TAR_TRANSFORM_OPT=-s ,^dist,acton,
endif

# Do grep to only get a version number. If there's an error, we get an empty
# string which is better than getting the error message itself.
ACTONC_VERSION=$(shell $(ACTONC) --numeric-version 2>/dev/null | grep -E "^[0-9.]+$$")
.PHONY: acton-$(OS)-$(ARCH)-$(ACTONC_VERSION).tar.xz
acton-$(OS)-$(ARCH)-$(ACTONC_VERSION).tar.xz:
	tar cv $(TAR_TRANSFORM_OPT) --exclude .gitignore dist | xz -z -0 --threads=0 > $@

.PHONY: release
release: distribution
	$(MAKE) acton-$(OS)-$(ARCH)-$(ACTONC_VERSION).tar.xz

# This target is used by the debian packaging
.PHONY: install
install:
	mkdir -p $(DESTDIR)/usr/bin $(DESTDIR)/usr/lib/acton
	cp -a dist/. $(DESTDIR)/usr/lib/acton/
	cd $(DESTDIR)/usr/bin && ln -s ../lib/acton/bin/acton
	cd $(DESTDIR)/usr/bin && ln -s ../lib/acton/bin/actonc
	cd $(DESTDIR)/usr/bin && ln -s ../lib/acton/bin/actondb
	cd $(DESTDIR)/usr/bin && ln -s ../lib/acton/bin/runacton

.PHONY: debian/changelog
debian/changelog: debian/changelog.in CHANGELOG.md
	cat $< | sed -e 's/VERSION/$(VERSION_INFO)/' -e 's/DEB_DIST/$(DEB_DIST)/' > $@

.PHONY: debs
debs: debian/changelog
	debuild --preserve-envvar VERSION_INFO -i -us -uc -b

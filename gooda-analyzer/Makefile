#    Gooda Makefile
#    Gooda Makefile
#
# Copyright 2012 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
PREFIX=/usr/local
GOODA_DIR=$(PREFIX)/libexec/gooda
gooda_path = $(subst /,\/,$(GOODA_DIR))
# must match gooda.spec version
VERSION=1.0.0

CC = gcc
INSTALL = install

CFLAGS=-O2 -g -I. -DGOODA_DIR=\"$(GOODA_DIR)\"

gooda :	perf_gooda_read.o gooda_create.o perf_gooda_create.o gooda_util.o analyzer.o asm2src.o column_align_intel.o column_align_def.o column_align.o load_addr.o
	${CC} $(CFLAGS) -o $@ perf_gooda_read.o gooda_create.o perf_gooda_create.o load_addr.o gooda_util.o analyzer.o asm2src.o column_align.o column_align_intel.o column_align_def.o -lbfd -liberty -lz -ldl

gooda_create.o :	gooda_create.c gooda.h perf_gooda.h gooda_util.h perf_event.h
	${CC} $(CFLAGS) -c gooda_create.c

perf_gooda_create.o :	perf_gooda_create.c gooda.h perf_gooda.h gooda_util.h perf_event.h
	${CC} $(CFLAGS) -c perf_gooda_create.c

load_addr.o :	load_addr.c gooda.h perf_gooda.h gooda_util.h perf_event.h
	${CC} $(CFLAGS) -c load_addr.c

gooda_util.o :	gooda_util.c gooda.h perf_gooda.h gooda_util.h perf_event.h
	${CC} $(CFLAGS) -c gooda_util.c

column_align_intel.o :	column_align_intel.c gooda.h perf_gooda.h gooda_util.h perf_event.h
	${CC} $(CFLAGS) -c column_align_intel.c

column_align_def.o :	column_align_def.c gooda.h perf_gooda.h gooda_util.h perf_event.h
	${CC} $(CFLAGS) -c column_align_def.c

column_align.o :	column_align.c gooda.h perf_gooda.h gooda_util.h perf_event.h
	${CC} $(CFLAGS) -c column_align.c

analyzer.o :	analyzer.c gooda.h perf_gooda.h gooda_util.h perf_event.h asm_2_src.h
	${CC} $(CFLAGS) -c analyzer.c

asm2src.o :	asm2src.c gooda.h perf_gooda.h gooda_util.h perf_event.h asm_2_src.h
	${CC} $(CFLAGS) -c asm2src.c

perf_gooda_read.o :	perf_gooda_read.c gooda.h perf_gooda.h gooda_util.h perf_event.h
	${CC} $(CFLAGS) -c -DANALYZE perf_gooda_read.c

reader: ${objs}
	${CC} $(CFLAGS) -DDBUG -DDBUGA -static perf_gooda_read.c -o $@ ${objs}


clean:
	rm -f *.o gooda


install: gooda
	-mkdir -p $(DESTDIR)$(GOODA_DIR)/scripts
	-mkdir -p $(DESTDIR)$(GOODA_DIR)/report_files
	-mkdir -p $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 755 gooda $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 644 scripts/*.txt $(DESTDIR)$(GOODA_DIR)/scripts
	$(INSTALL) -m 644 report_files/*.csv $(DESTDIR)$(GOODA_DIR)/report_files
	for i in scripts/*.sh; \
	do \
		sed -e "s/GOODA_DIR/$(gooda_path)/" $$i >$(DESTDIR)$(PREFIX)/bin/`basename $$i`; \
		chmod 755 $(DESTDIR)$(PREFIX)/bin/`basename $$i`; \
	done

rpm: 
	(R=`mktemp -d /var/tmp/rpmbuild.perf.XXXXXX`; \
	 mkdir -p $$R/SOURCES/gooda-$(VERSION); \
	 tar cpf - * | tar -C $$R/SOURCES/gooda-$(VERSION) -xpf -;\
	 cd $$R/SOURCES; \
	 tar czf gooda-$(VERSION).tar.gz gooda-$(VERSION); \
	 rm -rf gooda-$(VERSION); \
	 cd -; \
	 rpmbuild --clean --rmsource -bb gooda.spec --define "_topdir $$R"; \
	 cp $$R/RPMS/*/gooda*.rpm .;\
	 rm -rf $$R)

debian:
	debuild -i -us -uc -b

.PHONY: debian

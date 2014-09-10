#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <inttypes.h>

#define EI_NIDENT 16
#define ELFMAG0		0x7f		/* Magic number byte 0 */
#define ELFMAG1		'E'		/* Magic number byte 1 */
#define ELFMAG2		'L'		/* Magic number byte 2 */
#define ELFMAG3		'F'		/* Magic number byte 3 */

typedef struct {
	unsigned char e_ident[EI_NIDENT];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
  unsigned char	e_ident[EI_NIDENT];	/* Magic number and other info */
  uint16_t e_type;			/* Object file type */
  uint16_t e_machine;		/* Architecture */
  uint32_t e_version;		/* Object file version */
  uint64_t e_entry;		/* Entry point virtual address */
  uint64_t e_phoff;		/* Program header table file offset */
  uint64_t e_shoff;		/* Section header table file offset */
  uint32_t e_flags;		/* Processor-specific flags */
  uint16_t e_ehsize;		/* ELF header size in bytes */
  uint16_t e_phentsize;		/* Program header table entry size */
  uint16_t e_phnum;		/* Program header table entry count */
  uint16_t e_shentsize;		/* Section header table entry size */
  uint16_t e_shnum;		/* Section header table entry count */
  uint16_t e_shstrndx;		/* Section header string table index */
} Elf64_Ehdr;

typedef struct {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
} Elf32_Phdr;

typedef struct {
  uint32_t p_type;			/* Segment type */
  uint32_t p_flags;		/* Segment flags */
  uint64_t p_offset;		/* Segment file offset */
  uint64_t p_vaddr;		/* Segment virtual address */
  uint64_t p_paddr;		/* Segment physical address */
  uint64_t p_filesz;		/* Segment size in file */
  uint64_t p_memsz;		/* Segment size in memory */
  uint64_t p_align;		/* Segment alignment */
} Elf64_Phdr;

#define	ELFMAG		"\177ELF"
#define EI_CLASS	4	/* File class byte index in e_ident[] */
#define ELFCLASS32	1	/* 32-bit objects */
#define ELFCLASS64	2	/* 64-bit objects */

#define PT_LOAD		1	/* Loadable program segment */

#define PF_X		(1 << 0)	/* Segment is executable */
#define PF_R		(1 << 2)	/* Segment is readable */
#define PF_RX		(PF_X|PF_R)

static uint64_t
get_load_addr_32(int fd)
{
	Elf32_Ehdr hdr;
	Elf32_Phdr phdr;
	ssize_t s;
	size_t sz;
	off_t o;
	int i;

	s = read(fd, &hdr, sizeof(hdr));
	if (s != (ssize_t)sizeof(hdr)) {
		warnx("cannot read ehdr");
		return -1;
	}

	o = lseek(fd, (off_t)hdr.e_phoff, SEEK_SET);
	if (o != (off_t)hdr.e_phoff) {
		warnx("cannot get to program header start");
		return -1;
	}
	for (i = 0 ; i < hdr.e_phnum; i++) {
		s = read(fd, &phdr, sizeof(phdr));
		if (s != (ssize_t)sizeof(phdr)) {
			warnx("cannot read phdr %d", i);
			return -1;
		}

		if (phdr.p_type != PT_LOAD)
			continue;

		if ((phdr.p_flags & PF_RX) != PF_RX)
			continue;

		/* return first match */
		return (uint64_t)phdr.p_vaddr;
	}
	return -1;
}

static uint64_t
get_load_addr_64(int fd)
{
	Elf64_Ehdr hdr;
	Elf64_Phdr phdr;
	ssize_t s;
	size_t sz;
	off_t o;
	int i;

	s = read(fd, &hdr, sizeof(hdr));
	if (s != (ssize_t)sizeof(hdr)) {
		warnx("cannot read ehdr");
		return -1;
	}

	o = lseek(fd, (off_t)hdr.e_phoff, SEEK_SET);
	if (o != (off_t)hdr.e_phoff) {
		warnx("cannot get to program header start");
		return -1;
	}
	for (i = 0 ; i < hdr.e_phnum; i++) {
		s = read(fd, &phdr, sizeof(phdr));
		if (s != (ssize_t)sizeof(phdr)) {
			warnx("cannot read phdr %d", i);
			return -1;
		}

		if (phdr.p_type != PT_LOAD)
			continue;

		if ((phdr.p_flags & PF_RX) != PF_RX)
			continue;

		/* return first match */
		return (uint64_t)phdr.p_vaddr;
	}
	return -1;
}

static uint64_t
parse_header(int fd)
{
  	unsigned char ident[EI_NIDENT];	
	ssize_t s;
	off_t o;
	uint64_t addr;

	s = read(fd, ident, sizeof(ident));
	if (s != (ssize_t)sizeof(ident)) {
		warnx("cannot read ELF ident");
		return -1;
	}

	if (   ident[0] != ELFMAG0
	    || ident[1] != ELFMAG1
	    || ident[2] != ELFMAG2
	    || ident[3] != ELFMAG3) {
		warnx("not an ELF file");
		return -1;
	}
	o = lseek(fd, 0, SEEK_SET);
	if (o != 0) {
		warnx("cannot reset to ELF header start");
		return -1;
	}
	switch (ident[EI_CLASS]) {
	case ELFCLASS32:
			addr = get_load_addr_32(fd);
			break;
	case ELFCLASS64:
			addr = get_load_addr_64(fd);
			break;
	default:
		warnx("unknown ELF class %d", ident[EI_CLASS]);
		addr = -1;
	}
	return addr;
}


int main(int argc, char **argv)
{
	char *filename;
	uint64_t addr;
	int fd;

	if (argc < 1) {
		err(1, "usage: load_addr elf_filename");
		return -1;
	}
	filename = argv[1];

	fd = open(filename, O_RDONLY);
	if (fd == -1)
		err(1, "cannot open %s", filename);

	addr = parse_header(fd);
	close(fd);

	if (addr == (uint64_t)-1) {
		warnx("cannot determine load address for %s", filename);
		return -1;
	}
	printf("load addr for %s: %#"PRIx64"\n", filename, addr);
	return 0;
}

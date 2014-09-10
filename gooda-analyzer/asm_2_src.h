#ifndef A2LCG_H_
#define A2LCG_H_

int asm_2_src_init(const char *file_name);
int asm_2_src_inline(const char **file, unsigned *line_nr);
int asm_2_src(unsigned long addr, const char **file, unsigned *line_nr);
void asm_2_src_close(void);

#endif

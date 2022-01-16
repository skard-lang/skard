#ifndef SKARD_ERROR_H
#define SKARD_ERROR_H

void error_not_enough_memory(void);
void error_too_many_constants_in_chunk(void);
void error_could_not_open_file(const char *filename);
void error_not_enough_memory_to_read(const char *filename);
void error_could_not_read(const char *filename);

#endif //SKARD_ERROR_H

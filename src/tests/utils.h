void print_next_row(void);
void print_next_col(void);
void print_position_row(void);
void print_position_column(void);
void print_next_line(void);

#define PRINT_LINE(...) \
    do { \
        print_position_column(); \
        printf(__VA_ARGS__); \
        print_next_line(); \
    } while (0)

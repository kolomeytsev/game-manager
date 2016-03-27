#ifndef LEXIS_H
#define LEXIS_H

enum type_of_lex 
	{ keyword, function, function_with_param, function_without_param, 
		label, variable, number, string, assignment, separator, 
		error_funct, error_keyword, error };
enum type_of_state { H, N, F, L, V, K, A, S, ERR };

struct Lexeme_list {
	int num_str;
	type_of_lex type;
	char* lex;
	struct Lexeme_list* next;
};

class Automat {
	char buffer[64];
	int buf_len;
	int full;
	type_of_state state;
	Lexeme_list *list;
	type_of_lex lexeme;
	char sym;
	int nstr;
	bool err;
public:
	int use_prev;
	Automat();
	~Automat();
	void feed_char(int c);
	void run();
	void print_list();
	int full_lex();
	bool isnot_param(char *str);
	bool is_param(char *str);
	bool is_keyword(char *str);
	void add_new_elem();
	void state_home();
	void state_number();
	void state_function();
	void state_variable();
	void state_label();
	void state_keyword();
	void state_assignment();
	void state_string();
	void state_error();
	void inc_nstr();
	void free_list(Lexeme_list *lst);
	bool is_separator(int c);
	bool is_alfa(int c);
	bool is_digit(int c);
	bool is_any_errors();
	bool is_long();
	Lexeme_list *get_list();
	void print_str(const char *s);
};
#endif

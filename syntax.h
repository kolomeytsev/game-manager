#ifndef SYNTAX_H
#define SYNTAX_H

class SyntaxAnalyzer {
	Lexeme_list *cur;
	Lexeme_list *list;
	Lexeme_list *pointer;
	void S();
	void assignment_statement();
	void goto_statement();
	void if_statement();
	void while_statement();
	void game_statement();
	void print_statement();
	void expression();
	void op1();
	void ending1();
	void op2();
	void ending2();
	void op3();
	void ending3();
	void funct();
	void var();
	void index();
	void element();
	void add_poliz_list(PolizElem* element);
	void add_label_list(PolizItem* p, char* s);
	void add_var_list(char *s);
	void next();
	PolizItem* search_address(char *name);
	bool is_first_label(char *name);
	void add_address(PolizItem *p, char *name);
	void add_address_poliz(PolizItem *p, char *name);
	void make_poliz_var();
	void make_poliz_label();
	void add_nul_index();
	void add_index();
public:
	void set_list(Lexeme_list *lst);
	void analyze();
	PolizItem* poliz_list;
	LabelTable* label_list;
	VarTable* var_list;
};

class Exeption {
	char *comment;
	int nstr;
	char *lexeme;
public:
	Exeption(const char *cmt, int num, const char *lex);
	Exeption(const Exeption& other);
	~Exeption();
	const char *GetComment() const {
		return comment;
	}
	int GetNstr() const {
		return nstr;
	}
	const char *GetLexeme() const {
		return lexeme;
	}
private:
	static char *strdup(const char *str);
};

#endif

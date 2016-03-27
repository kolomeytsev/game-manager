#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <typeinfo>
#include "lexis.h"

Automat::Automat()
{
	sym = 0;
	buf_len = 0;
	list = 0;
	full = 0;
	use_prev = 0;
	state = H;
	nstr = 1;
	err = false;
}

Automat::~Automat()
{
	free_list(list);
}

void Automat::free_list(Lexeme_list *lst)
{
	if (lst!=0) {
		free_list(lst->next);
		delete lst->lex;
		delete lst;
	}
}

void Automat::feed_char(int c)
{
	sym = c;
}

void Automat::run()
{
	if (use_prev)
		use_prev = 0;
	switch (state) {
	case H:
		state_home();
		break;
	case N:
		state_number();
		break;
	case F:
		state_function();
		break;
	case V:
		state_variable();
		break;
	case L:
		state_label();
		break;
	case K:
		state_keyword();
		break;
	case A:
		state_assignment();
		break;
	case S:
		state_string();
		break;
	case ERR:
		state_error();
		break;
	}
}

void Automat::state_home()
{
	if ((sym==' ')||(sym=='\t')||(sym=='\n')) {
		return;
	}
	if (is_digit(sym)) {
		buffer[buf_len] = sym;
		buf_len++;
		state = N;
	}
	if (sym=='?') {
		state = F;
	}
	if (sym=='@') {
		state = L;
	}
	if (sym=='$') {
		state = V;
	}
	if (is_alfa(sym)) {
		buffer[buf_len] = sym;
		buf_len++;
		state = K;
	}
	if (sym==':') {
		buffer[buf_len] = sym;
		buf_len++;
		state = A;
		return;
	}
	if (sym=='"') {
		state = S;
	}
	if (is_separator(sym)) {
		lexeme = separator;
		buffer[buf_len] = sym;
		buf_len++;
		full = 1;
	}
}

void Automat::state_number()
{
	if (is_digit(sym)) {
		buffer[buf_len] = sym;
		buf_len++;
		state = N;
	} else {
		if (is_separator(sym)) {
			lexeme = number;
			full = 1;
			use_prev = 1;
			state = H;
		} else {
			buffer[buf_len] = sym;
			buf_len++;
			state = ERR;
			err = true;
		}
	}
}

void Automat::state_function()
{
	if (is_alfa(sym)||is_digit(sym)||(sym=='_')) {
		buffer[buf_len] = sym;
		buf_len++;
		state = F;
	} else {
		if (is_separator(sym)){
			lexeme = function;
			full = 1;
			use_prev = 1;
			state = H;
		} else {
			buffer[buf_len] = sym;
			buf_len++;
			state = ERR;
			err = true;
		}
	}
}

void Automat::state_variable()
{
	if (is_alfa(sym)||is_digit(sym)||(sym=='_')) {
		buffer[buf_len] = sym;
		buf_len++;
		state = V;
	} else {
		if (is_separator(sym)){
			lexeme = variable;
			full = 1;
			use_prev = 1;
			state = H;
		} else {
			buffer[buf_len] = sym;
			buf_len++;
			state = ERR;
			err = true;
		}
	}
}

void Automat::state_label()
{
	if (is_alfa(sym)||is_digit(sym)||(sym=='_')) {
		buffer[buf_len] = sym;
		buf_len++;
		state = L;
	} else {
		if (is_separator(sym)){
			lexeme = label;
			full = 1;
			use_prev = 1;
			state = H;
		} else {
			buffer[buf_len] = sym;
			buf_len++;
			state = ERR;
			err = true;
		}
	}
}

void Automat::state_keyword()
{
	if (is_alfa(sym)) {
		buffer[buf_len] = sym;
		buf_len++;
		state = K;
	} else {
		if (is_separator(sym)) {
			lexeme = keyword;
			full = 1;
			use_prev = 1;
			state = H;
		} else {
			buffer[buf_len] = sym;
			buf_len++;
			state = ERR;
			err = true;
		}
	}
}

void Automat::state_assignment()
{
	if (sym=='=') {
		lexeme = assignment;
		buffer[buf_len] = sym;
		buf_len++;
		full = 1;
		state = H;
	} else {
		lexeme = error;
		err = true;
		full = 1;
		use_prev = 1;
		state = H;
	}
}

void Automat::state_string()
{
	if (sym!='"') {
		buffer[buf_len] = sym;
		buf_len++;
	} else {
		full = 1;
		lexeme = string;
		state = H;
	}
}

void Automat::state_error()
{
	if (is_separator(sym)) {
		lexeme = error;
		full = 1;
		use_prev = 1;
		state = H;
	} else {
		buffer[buf_len] = sym;
		buf_len++;
		state = ERR;
	}
}

int Automat::full_lex()
{
	if (full)
		return 1;
	else
		return 0;
}

void Automat::inc_nstr()
{
	nstr++;
}

bool Automat::isnot_param(char *str)
{
	if (strcmp(str, "my_id")==0) return 1;
	if (strcmp(str, "turn")==0) return 1;
	if (strcmp(str, "players")==0) return 1;
	if (strcmp(str, "active_players")==0) return 1;
	if (strcmp(str, "supply")==0) return 1;
	if (strcmp(str, "raw_price")==0) return 1;
	if (strcmp(str, "demand")==0) return 1;
	if (strcmp(str, "production_price")==0) return 1;
	if (strcmp(str, "factories_being_build")==0) return 1;
	if (strcmp(str, "expected_raw_sold")==0) return 1;
	if (strcmp(str, "expected_prod_bought")==0) return 1;
	if (strcmp(str, "expected_raw_price")==0) return 1;
	if (strcmp(str, "expected_prod_price")==0) return 1;
	if (strcmp(str, "prev_raw_price")==0) return 1;
	if (strcmp(str, "prev_prod_price")==0) return 1;
	return 0;
}

bool Automat::is_param(char *str)
{
	if (strcmp(str, "money")==0) return 1;
	if (strcmp(str, "raw")==0) return 1;
	if (strcmp(str, "production")==0) return 1;
	if (strcmp(str, "factories")==0) return 1;
	if (strcmp(str, "auto_factories")==0) return 1;
	if (strcmp(str, "manufactured")==0) return 1;
	if (strcmp(str, "result_raw_sold")==0) return 1;
	if (strcmp(str, "result_raw_price")==0) return 1;
	if (strcmp(str, "result_prod_bought")==0) return 1;
	if (strcmp(str, "result_prod_price")==0) return 1;
	return 0;
}

bool Automat::is_keyword(char *str)
{
	if (strcmp(str, "if")==0) return 1;
	if (strcmp(str, "then")==0) return 1;
	if (strcmp(str, "goto")==0) return 1;
	if (strcmp(str, "print")==0) return 1;
	if (strcmp(str, "buy")==0) return 1;
	if (strcmp(str, "sell")==0) return 1;
	if (strcmp(str, "prod")==0) return 1;
	if (strcmp(str, "build")==0) return 1;
	if (strcmp(str, "endturn")==0) return 1;
	if (strcmp(str, "end")==0) return 1;
	if (strcmp(str, "while")==0) return 1;
	if (strcmp(str, "do")==0) return 1;
	return 0;
}

bool Automat::is_any_errors()
{
	return err;
}

bool Automat::is_long()
{
	if (buf_len >= 64)
		return true;
	return false;
}

void Automat::add_new_elem()
{
	char *str = new char [buf_len];
	Lexeme_list *lst_tmp, *tmp;
	int i;
	for (i=0; i<buf_len; i++) {
		str[i] = buffer[i];
	}
	buf_len = 0;
	full = 0;
	tmp = list;
	lst_tmp = new Lexeme_list;
	lst_tmp->num_str = nstr;
	if (lexeme == function) {
		if (isnot_param(str))
			lexeme = function_without_param;
		else
			if (is_param(str))
				lexeme = function_with_param;
			else {
				lexeme = error_funct;
				err = true;
			}
	}
	if (lexeme == keyword)
		if (is_keyword(str)==0) {
			lexeme = error_keyword;
			err = true;
		}
	lst_tmp->type = lexeme;
	lst_tmp->lex = str;
	lst_tmp->next = NULL;
	if (list == 0)
		list = lst_tmp;
	else {
		while (tmp->next!=0)
			tmp = tmp->next;
		tmp->next = lst_tmp;
	}
}

void Automat::print_list()
{
	Lexeme_list *tmp;
	tmp = list;
	while (tmp!=0) {
		printf("%3d,   ", tmp->num_str);
		switch (tmp->type) {
		case keyword:
			print_str("keyword");
			break;
		case number:
			print_str("number");
			break;
		case separator:
			print_str("separator");
			break;
		case string:
			print_str("string");
			break;
		case function:
			print_str("function");
			break;
		case function_with_param:
			print_str("func with par");
			break;
		case function_without_param:
			print_str("func without par");
			break;
		case error_funct:
			print_str("no such function!!!!!!!!!!!");
			break;
		case error_keyword:
			print_str("no such keyword!!!!!!!!!!");
			break;
		case variable:
			print_str("variable");
			break;
		case label:
			print_str("label");
			break;
		case assignment:
			print_str("assignment");
			break;
		case error:
			print_str("error!!!!!!!!!!!!!!");
		}
		printf(" <%s>\n", tmp->lex);
		tmp = tmp->next;
	}
}

void Automat::print_str(const char *s)
{
	printf("%16s", s);
}

bool Automat::is_separator(int c)
{
	switch (c) {
	case '+': return true;
	case '-': return true;
	case '*': return true;
	case '/': return true;
	case '%': return true;
	case '<': return true;
	case '>': return true;
	case '=': return true;
	case ':': return true;
	case '(': return true;
	case ')': return true;
	case '[': return true;
	case ']': return true;
	case ';': return true;
	case ',': return true;
	case ' ': return true;
	case '\t': return true;
	case '\n': return true;
	case '.': return true;
	default: return false;
	}
}

bool Automat::is_alfa(int c)
{
	return (((sym>='A')&&(sym<='Z'))||((sym>='a')&&(sym<='z')));
}

bool Automat::is_digit(int c)
{
	return ((sym>='0')&&(sym<='9'));
}

Lexeme_list *Automat::get_list()
{
	return list;
}

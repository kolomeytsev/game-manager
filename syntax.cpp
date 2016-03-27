#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <typeinfo>
#include "lexis.h"
#include "poliz.h"
#include "syntax.h"

Exeption::Exeption(const char *cmt, int num, const char *lex) {
	comment = strdup(cmt);
	nstr = num;
	lexeme = strdup(lex);
}

Exeption::Exeption(const Exeption& other) {
	comment = strdup(other.comment);
	nstr = other.nstr;
	lexeme = strdup(other.lexeme);
}

Exeption::~Exeption() {
	delete[] comment;
}

char* Exeption::strdup(const char *str) {
	char *res;
	res = new char [strlen(str)+1];
	strcpy(res, str);
	return res;
}

void SyntaxAnalyzer::next()
{
	cur = pointer;
	if ((pointer->next!=0)&&(strcmp(pointer->lex, ".")!=0))
		pointer = pointer->next;
}

void SyntaxAnalyzer::set_list(Lexeme_list *lst)
{
	list = lst;
	pointer = list;
	poliz_list = 0;
	label_list = 0;
	var_list = 0;
}

void SyntaxAnalyzer::analyze()
{
	next();
	S();
	if (strcmp(cur->lex, ".")!=0)
		throw Exeption("dot expected", cur->num_str, cur->lex);
}

void SyntaxAnalyzer::S()
{
	if ((cur->type == keyword)&&
		(strcmp(cur->lex, "end")!=0)) {
		if (strcmp(cur->lex, "if")==0) {
			next();
			if_statement();
			S();
		}
		else
		if (strcmp(cur->lex, "while")==0) {
			next();
			while_statement();
			S();
		}
		else
		if (strcmp(cur->lex, "goto")==0) {
			next();
			goto_statement();
			PolizOpGo* tmp = new PolizOpGo;
			add_poliz_list(tmp);
			S();
		}
		else
		if (strcmp(cur->lex, "print")==0) {
			next();
			print_statement();
			PolizPrint* tmp = new PolizPrint;
			add_poliz_list(tmp);
			S();
		}
		else
		if (strcmp(cur->lex, "then")!=0) {
			game_statement();
			S();
		}
		else
			throw Exeption("unexpected then", cur->num_str, cur->lex);
	}
	else
	if (cur->type == variable) {
		make_poliz_var();
		next();
		assignment_statement();
		PolizFunAssign* tmp = new PolizFunAssign;
		add_poliz_list(tmp);
		S();
	}
	else
	if (cur->type == label) {
		make_poliz_label();
		next();
		if (strcmp(cur->lex, ">")!=0)
			throw Exeption("'>' expected", cur->num_str, cur->lex);
		next();
		S();
	}
	else {
	if (strcmp(cur->lex, "end")!=0)
		throw Exeption("end expected", cur->num_str, cur->lex);
	else
		next();
	}
}

void SyntaxAnalyzer::if_statement()
{
	expression();
	if (strcmp(cur->lex, "then") != 0)
		throw Exeption("then expexted", cur->num_str, cur->lex);
	PolizLabel* tmp1 = new PolizLabel(0, 0);
	add_poliz_list(tmp1);
	PolizItem* p1 = poliz_list;
	while (p1->next!=0)
		p1 = p1->next;
	PolizOpGoFalse* tmp2 = new PolizOpGoFalse;
	add_poliz_list(tmp2);
	next();
	S();
	if (strcmp(cur->lex, ";") != 0)
		throw Exeption("';' expected", cur->num_str, cur->lex);
	next();
	PolizFunNOP* tmp5 = new PolizFunNOP;
	add_poliz_list(tmp5);
	PolizItem* p2 = poliz_list;
	while (p2->next!=0)
		p2 = p2->next;
	PolizLabel *ptr = dynamic_cast<PolizLabel*>(p1->elem);
	ptr->set_addr(p2);
}

void SyntaxAnalyzer::while_statement()
{
	PolizFunNOP* tmp = new PolizFunNOP;
	add_poliz_list(tmp);
	PolizItem* p = poliz_list;
	while (p->next!=0)
		p = p->next;
	expression();
	if (strcmp(cur->lex, "do") != 0)
		throw Exeption("do expexted", cur->num_str, cur->lex);
	PolizLabel* tmp1 = new PolizLabel(0, 0);
	add_poliz_list(tmp1);
	PolizItem* p1 = poliz_list;
	while (p1->next!=0)
		p1 = p1->next;
	PolizOpGoFalse* tmp2 = new PolizOpGoFalse;
	add_poliz_list(tmp2);
	next();
	S();
	if (strcmp(cur->lex, ";") != 0)
		throw Exeption("';' expected", cur->num_str, cur->lex);
	next();
	PolizLabel* tmp3 = new PolizLabel(0, p);
	add_poliz_list(tmp3);
	PolizOpGo* tmp4 = new PolizOpGo;
	add_poliz_list(tmp4);
	PolizFunNOP* tmp5 = new PolizFunNOP;
	add_poliz_list(tmp5);
	PolizItem* p2 = poliz_list;
	while (p2->next!=0)
		p2 = p2->next;
	PolizLabel *ptr = dynamic_cast<PolizLabel*>(p1->elem);
	ptr->set_addr(p2);
}

void SyntaxAnalyzer::goto_statement()
{
	if (cur->type != label)
		throw Exeption("label expected", cur->num_str, cur->lex);
	PolizItem* item = search_address(cur->lex); // in label list
	PolizLabel* tmp = new PolizLabel(cur->lex, item);
	add_poliz_list(tmp);
	if (item == 0)
		add_label_list(0, cur->lex);
	next();
	if (strcmp(cur->lex, ";") !=0 )
		throw Exeption("';' expected", cur->num_str, cur->lex);
	next();
}

void SyntaxAnalyzer::print_statement()
{
	if (cur->type == string) {
		PolizString* tmp = new PolizString(cur->lex);
		add_poliz_list(tmp);
		next();
	}
	else
		expression();
	if (strcmp(cur->lex, ";") !=0 )
		throw Exeption("';' expected", cur->num_str, cur->lex);
	next();
}

void SyntaxAnalyzer::game_statement()
{
	if (strcmp(cur->lex, "endturn")==0) {
		PolizEndTurn* tmp = new PolizEndTurn();
		add_poliz_list(tmp);
		next();
	} else
	if ((strcmp(cur->lex,"prod")==0)||(strcmp(cur->lex,"build")==0)) {
		bool is_prod = false;
		if (strcmp(cur->lex, "prod")==0) is_prod = true;
		next();
		expression();
		if (is_prod) {
			PolizProd* tmp = new PolizProd();
			add_poliz_list(tmp);
		} else {
			PolizBuild* tmp = new PolizBuild();
			add_poliz_list(tmp);
		}
	}
	else
	if ((strcmp(cur->lex,"buy")==0)||(strcmp(cur->lex,"sell")==0)) {
		bool is_buy = false;
		if (strcmp(cur->lex, "buy")==0) is_buy = true;
		next();
		expression();
		expression();
		if (is_buy) {
			PolizBuy* tmp = new PolizBuy();
			add_poliz_list(tmp);
		} else {
			PolizSell* tmp = new PolizSell();
			add_poliz_list(tmp);
		}
	}
	if (strcmp(cur->lex,";")!=0)
		throw Exeption("';' expected", cur->num_str, cur->lex);
	next();
}

void SyntaxAnalyzer::assignment_statement()
{
	if (cur->type == assignment) {
		add_nul_index();
		next();
	} else
	if (strcmp(cur->lex, "[")==0) {
		next();
		expression();
		if (strcmp(cur->lex, "]")!=0)
			throw Exeption("']' expected", cur->num_str, cur->lex);
		next();
		if (cur->type != assignment)
			throw Exeption("':=' expected", cur->num_str, cur->lex);
		next();
		add_index();
	}
	else
		throw Exeption("':=' or index expected",cur->num_str, cur->lex);
	expression();
	if (strcmp(cur->lex,";")!=0)
		throw Exeption("';' expected", cur->num_str, cur->lex);
	next();
}

void SyntaxAnalyzer::expression()
{
	op1();
	ending1();
}

void SyntaxAnalyzer::op1()
{
	op2();
	ending2();
}

void SyntaxAnalyzer::op2()
{
	op3();
	ending3();
}

void SyntaxAnalyzer::op3()
{
	if (cur->type==number) {
		PolizInt* tmp = new PolizInt(atoi(cur->lex));
		add_poliz_list(tmp);
		next();
	} else
	if (cur->type==variable) {
		PolizString* tmp = new PolizString(cur->lex);
		add_poliz_list(tmp);
		next();
		if (strcmp(cur->lex, "[")==0) {
			next();
			expression();
			if (strcmp(cur->lex, "]")!=0)
				throw Exeption("']' expected", cur->num_str, cur->lex);
			next();
			add_index();
		} else 
			add_nul_index();
		PolizVar* tmp2 = new PolizVar;
		add_poliz_list(tmp2);
	}
	else
	if (cur->type==function_without_param) {
		PolizFunNoParam* tmp = new PolizFunNoParam(cur->lex);
		add_poliz_list(tmp);
		next();
	}
	else
	if (cur->type==function_with_param) {
		PolizFunParam* tmp = new PolizFunParam(cur->lex);
		next();
		if (strcmp(cur->lex, "(")!=0)
			throw Exeption("'(' expected in funct",
				cur->num_str, cur->lex);
		next();
		expression();
		if (strcmp(cur->lex, ")")!=0)
			throw Exeption("')' expected in funct",
				cur->num_str, cur->lex);
		next();
		add_poliz_list(tmp);
	}
	else
	{
		if (strcmp(cur->lex, "(")!=0)
			throw Exeption("'(' expected", cur->num_str, cur->lex);
		next();
		expression();
		if (strcmp(cur->lex, ")")!=0)
			throw Exeption("')' expected", cur->num_str, cur->lex);
		next();
	}
}

void SyntaxAnalyzer::ending3()
{
	if ((strcmp(cur->lex, "*")==0)||
		(strcmp(cur->lex, "/")==0)||
		(strcmp(cur->lex, "%")==0)) {
		bool is_mult = false;
		bool is_div = false;
		bool is_mod = false;
		if (strcmp(cur->lex, "*")==0) is_mult = true;
		if (strcmp(cur->lex, "/")==0) is_div = true;
		if (strcmp(cur->lex, "%")==0) is_mod = true;
		next();
		op2();
		if (is_mult) {
			PolizFunMult* tmp = new PolizFunMult();
			add_poliz_list(tmp);
		}
		if (is_div) {
			PolizFunDiv* tmp = new PolizFunDiv();
			add_poliz_list(tmp);
		} 
		if (is_mod) {
			PolizFunMod* tmp = new PolizFunMod();
			add_poliz_list(tmp);
		}
	}
}

void SyntaxAnalyzer::ending2()
{
	if ((strcmp(cur->lex, "+")==0)||
		(strcmp(cur->lex, "-")==0)) {
		bool is_plus = false;
		if (strcmp(cur->lex, "+")==0) is_plus = true;
		next();
		op1();
		if (is_plus) {
			PolizFunPlus* tmp = new PolizFunPlus();
			add_poliz_list(tmp);
		} else {
			PolizFunMinus* tmp = new PolizFunMinus();
			add_poliz_list(tmp);
		}
	}
}

void SyntaxAnalyzer::ending1()
{
	if ((strcmp(cur->lex, "=")==0)||
		(strcmp(cur->lex, ">")==0)||
		(strcmp(cur->lex, "<")==0)) {
		bool is_greater = false;
		bool is_less = false;
		bool is_equal = false;
		if (strcmp(cur->lex, ">")==0) is_greater = true;
		if (strcmp(cur->lex, "<")==0) is_less = true;
		if (strcmp(cur->lex, "=")==0) is_equal = true;
		next();
		expression();
		if (is_greater) {
			PolizFunGreater* tmp = new PolizFunGreater();
			add_poliz_list(tmp);
		}
		if (is_less) {
			PolizFunLess* tmp = new PolizFunLess();
			add_poliz_list(tmp);
		}
		if (is_equal) {
			PolizFunEqual* tmp = new PolizFunEqual();
			add_poliz_list(tmp);
		}
	}
}

PolizItem* SyntaxAnalyzer::search_address(char *name)
{
	LabelTable* tmp = label_list;
	while (tmp!=0) {
		if (strcmp(tmp->name, name)==0) {
			return tmp->pointer;
		}
		tmp = tmp->next;
	}
	return 0;
}

bool SyntaxAnalyzer::is_first_label(char *name)
{
	LabelTable* tmp = label_list;
	while (tmp!=0) {
		if (strcmp(tmp->name, name)==0) {
			return false;
		}
		tmp = tmp->next;
	}
	return true;
}

void SyntaxAnalyzer::add_address(PolizItem *p, char *name)
{
	LabelTable* tmp = label_list;
	while (tmp!=0) {
		if (strcmp(tmp->name, name)==0) {
			tmp->pointer = p;
			return;
		}
		tmp = tmp->next;
	}
}

void SyntaxAnalyzer::add_address_poliz(PolizItem *p, char *name)
{
	PolizItem *tmp = poliz_list;
	while (tmp!=0) {
		PolizLabel *ptr = dynamic_cast<PolizLabel*>(tmp->elem);
		if (ptr) {
			if (ptr->Get_name()!=0) {
				if (strcmp(ptr->Get_name(), name)==0)
					ptr->set_addr(p);
			}
		}
		tmp = tmp->next;
	}
}

void SyntaxAnalyzer::add_poliz_list(PolizElem* element)
{
	PolizItem *t = new PolizItem;
	PolizItem *temp;
	temp = poliz_list;
	t->elem = element;
	t->next = 0;
	if (poliz_list == 0)
		poliz_list = t;
	else {
		while (temp->next!=0)
			temp = temp->next;
		temp->next = t;
	}
}

void SyntaxAnalyzer::add_label_list(PolizItem* p, char* s)
{
	LabelTable* t = new LabelTable;
	LabelTable* temp;
	temp = label_list;
	t->name = s;
	t->pointer = p;
	t->next = 0;
	if (label_list==0)
		label_list = t;
	else {
		while (temp->next!=0)
			temp = temp->next;
		temp->next = t;
	}
}

void SyntaxAnalyzer::add_var_list(char *s) {
	VarTable* t = new VarTable;
	VarTable* temp = var_list;
	t->elem = new StructVar;
	t->elem->name = s;
	t->next = 0;
	if (var_list==0)
		var_list = t;
	else {
		while (temp->next!=0)
			temp = temp->next;
		temp->next = t;
	}
}

void SyntaxAnalyzer::make_poliz_var()
{
	PolizString* tmp = new PolizString(cur->lex);
	add_poliz_list(tmp);
}

void SyntaxAnalyzer::make_poliz_label()
{
	PolizFunNOP* tmp = new PolizFunNOP;
	add_poliz_list(tmp);
	PolizItem* p = poliz_list;
	while (p->next!=0)
		p = p->next;
	if (is_first_label(cur->lex))
		add_label_list(p, cur->lex);
	else {
		add_address(p, cur->lex);
		add_address_poliz(p, cur->lex);
	}
}

void SyntaxAnalyzer::add_nul_index()
{
	PolizInt* tmp = new PolizInt(0);
	add_poliz_list(tmp);
	PolizFunIndex* tmp2 = new PolizFunIndex;
	add_poliz_list(tmp2);
}

void SyntaxAnalyzer::add_index()
{
	PolizFunIndex* tmp2 = new PolizFunIndex;
	add_poliz_list(tmp2);
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <typeinfo>

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

class PolizElem;

struct PolizItem {
	PolizElem* elem;
	struct PolizItem* next;
};

struct StructVar {
	char *name;
	int value;
};

struct VarTable { // table->elem->name
	StructVar* elem;
	struct VarTable* next;
};

struct LabelTable {
	char* name;
	PolizItem* pointer;
	struct LabelTable* next;
};

class PolizEx {
	
};

class PolizExNotInt: public PolizEx {
public:
	PolizExNotInt(PolizElem *elem) {}
};

class PolizExNotLabel: public PolizEx {
public:
	PolizExNotLabel(PolizElem *elem) {}
};

class PolizExNotConst: public PolizEx {
public:
	PolizExNotConst(PolizElem *elem) {}
};

class PolizElem {
public:
	virtual ~PolizElem() {}
	virtual void Evaluate(PolizItem **stack,
						PolizItem **cur_cmd) const = 0;
protected:
	static void Push(PolizItem **stack, PolizElem *elem);
	static PolizElem* Pop(PolizItem **stack);
};

void PolizElem::Push(PolizItem **stack, PolizElem *elem)
{
	//printf("push\n");
	PolizItem* tmp = new PolizItem;
	tmp->elem = elem;
	tmp->next = *stack;
	*stack = tmp;
}

PolizElem* PolizElem::Pop(PolizItem **stack)
{
	//printf("pop\n");
	PolizItem* tmp = *stack;
	PolizElem* tmp_elem = tmp->elem;
	*stack = (*stack)->next;
	delete tmp;
	return tmp_elem;
}

class PolizConst: public PolizElem {
public:
	virtual void Evaluate(PolizItem **stack, PolizItem **cur_cmd) const
	{
		Push(stack, Clone());
		*cur_cmd = (*cur_cmd)->next;
	}
	virtual PolizElem* Clone() const = 0;
};

class PolizInt: public PolizConst {
	int value;
public:
	PolizInt(int a) { value = a; }
	virtual ~PolizInt() {}
	virtual PolizElem* Clone()const {
		return new PolizInt(value);
	}
	int Get() const {
		return value;
	}
};

class PolizString: public PolizConst {
	char *str;
public:
	PolizString(char *s) { str = s; }
	virtual ~PolizString() { delete str;}
	virtual PolizElem* Clone() const {
		int len;
		len = strlen(str);
		char *s = new char[len+1];
		for (int i = 0; i < len + 1; i++)
			s[i] = str[i];
		return new PolizString(s);
	}
	char *Get() const {
		return str;
	}	
};

class PolizVarAddr: public PolizConst {
	VarTable* addr;
public:
	PolizVarAddr(VarTable* a) { addr = a; }
	virtual ~PolizVarAddr() {}
	virtual PolizElem* Clone() const {
		return new PolizVarAddr(addr);
	}
	VarTable* Get() const {
		return addr;
	}	
};

class PolizLabel: public PolizConst {
	char *name;
	PolizItem* value;
public:
	PolizLabel(char *s, PolizItem* a) {
		name = s;
		value = a;
	}
	virtual ~PolizLabel() {}
	virtual PolizElem* Clone() const {
		return new PolizLabel(name, value);
	}
	PolizItem* Get() const {
		return value;
	}
	char* Get_name() const {
		return name;
	}
	void set_addr(PolizItem *ptr) {
		value = ptr;
	}
};

class PolizFunction: public PolizElem {
public:
	virtual void Evaluate(PolizItem **stack, PolizItem **cur_cmd) const
	{
		PolizElem *res = EvaluateFun(stack);
		if (res)
			Push(stack, res);
		*cur_cmd = (*cur_cmd)->next;
	}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const = 0;
};

class PolizVar: public PolizFunction {
public:
};

class PolizSell: public PolizFunction {
public:
	PolizSell() {}
	virtual ~PolizSell() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const {
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		PolizElem *operand2 = Pop(stack);
		PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
		if (!i2) throw PolizExNotInt(operand2);
		printf("sell %d, %d\n", i2->Get(), i1->Get());
		delete operand1;
		delete operand2;
		return 0;
	}
};

class PolizBuy: public PolizFunction {
public:
	PolizBuy() {}
	virtual ~PolizBuy() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const {
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		PolizElem *operand2 = Pop(stack);
		PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
		if (!i2) throw PolizExNotInt(operand2);
		printf("buy %d, %d\n", i2->Get(), i1->Get());
		delete operand1;
		delete operand2;
		return 0;
	}
};

class PolizProd: public PolizFunction {
public:
	PolizProd() {}
	virtual ~PolizProd() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const {
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		printf("prod %d\n", i1->Get());
		delete operand1;
		return 0;
	}
};

class PolizBuild: public PolizFunction {
public:
	PolizBuild() {}
	virtual ~PolizBuild() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const {
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		printf("build %d\n", i1->Get());
		delete operand1;
		return 0;
	}
};

class PolizEndTurn: public PolizFunction {
public:
	PolizEndTurn() {}
	virtual ~PolizEndTurn() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const {
		printf("end turn\n");
		return 0;
	}
};

class PolizPrint: public PolizFunction {
public:
	PolizPrint() {}
	virtual ~PolizPrint() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const
	{
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) {
			PolizString *i1 = dynamic_cast<PolizString*>(operand1);
			if (!i1) throw PolizExNotConst(operand1);
			printf("<%s>\n", i1->Get());
		} else {
			printf("<%d>\n", i1->Get());
		}
		delete operand1;
		return 0;
	}
};

class PolizFunNOP: public PolizFunction {
	int a;
public:
	PolizFunNOP() { a = 505; }
	virtual ~PolizFunNOP() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const {
		return 0;
	} 
};

class PolizFunPlus: public PolizFunction {
public:
	PolizFunPlus() {}
	virtual ~PolizFunPlus() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const
	{
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		PolizElem *operand2 = Pop(stack);
		PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
		if (!i2) throw PolizExNotInt(operand2);
		int res = i1->Get() + i2->Get();
		delete operand1;
		delete operand2;
		return new PolizInt(res);
	}
};

class PolizFunMinus: public PolizFunction {
public:
	PolizFunMinus() {}
	virtual ~PolizFunMinus() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const
	{
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		PolizElem *operand2 = Pop(stack);
		PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
		if (!i2) throw PolizExNotInt(operand2);
		int res = i2->Get() - i1->Get();
		delete operand1;
		delete operand2;
		return new PolizInt(res);
	}
};

class PolizFunMult: public PolizFunction {
public:
	PolizFunMult() {}
	virtual ~PolizFunMult() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const
	{
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		PolizElem *operand2 = Pop(stack);
		PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
		if (!i2) throw PolizExNotInt(operand2);
		int res = i1->Get() * i2->Get();
		delete operand1;
		delete operand2;
		return new PolizInt(res);
	}
};

class PolizFunDiv: public PolizFunction {
public:
	PolizFunDiv() {}
	virtual ~PolizFunDiv() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const
	{
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		PolizElem *operand2 = Pop(stack);
		PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
		if (!i2) throw PolizExNotInt(operand2);
		int res = i2->Get()/i1->Get();
		delete operand1;
		delete operand2;
		return new PolizInt(res);
	}
};

class PolizFunMod: public PolizFunction {
public:
	PolizFunMod() {}
	virtual ~PolizFunMod() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const
	{
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		PolizElem *operand2 = Pop(stack);
		PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
		if (!i2) throw PolizExNotInt(operand2);
		int res = i2->Get()%i1->Get();
		delete operand1;
		delete operand2;
		return new PolizInt(res);
	}
};

class PolizFunGreater: public PolizFunction {
public:
	PolizFunGreater() {}
	virtual ~PolizFunGreater() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const
	{
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		PolizElem *operand2 = Pop(stack);
		PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
		if (!i2) throw PolizExNotInt(operand2);
		bool t = i2->Get() > i1->Get();
		int res;
		if (t) res = 1;
		else res = 0;
		delete operand1;
		delete operand2;
		return new PolizInt(res);
	}
};

class PolizFunLess: public PolizFunction {
public:
	PolizFunLess() {}
	virtual ~PolizFunLess() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const
	{
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		PolizElem *operand2 = Pop(stack);
		PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
		if (!i2) throw PolizExNotInt(operand2);
		bool t = i2->Get() < i1->Get();
		int res;
		if (t) res = 1;
		else res = 0;
		delete operand1;
		delete operand2;
		return new PolizInt(res);
	}
};

class PolizFunEqual: public PolizFunction {
public:
	PolizFunEqual() {}
	virtual ~PolizFunEqual() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const
	{
		PolizElem *operand1 = Pop(stack);
		PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
		if (!i1) throw PolizExNotInt(operand1);
		PolizElem *operand2 = Pop(stack);
		PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
		if (!i2) throw PolizExNotInt(operand2);
		bool t = i2->Get() == i1->Get();
		int res;
		if (t) res = 1;
		else res = 0;
		delete operand1;
		delete operand2;
		return new PolizInt(res);
	}
};

void lprint(PolizItem* lst);

class PolizOpGo: public PolizElem {
public:
	PolizOpGo() {}
	virtual ~PolizOpGo() {}
	virtual void Evaluate(PolizItem **stack, PolizItem **cur_cmd) const
	{
		printf("OpGO!\n");
		PolizElem *operand1 = Pop(stack);
		PolizLabel *lab = dynamic_cast<PolizLabel*>(operand1);
		if (!lab) throw PolizExNotLabel(operand1);
		PolizItem *addr = lab->Get();
		//lprint(addr->next); //print string
		*cur_cmd = addr;
		delete operand1;
	}
};

class PolizOpGoFalse: public PolizElem {
public:
	PolizOpGoFalse() {}
	virtual ~PolizOpGoFalse() {}
	virtual void Evaluate(PolizItem **stack, PolizItem **cur_cmd) const
	{
		PolizElem *operand1 = Pop(stack);
		PolizLabel *lab = dynamic_cast<PolizLabel*>(operand1);
		if (!lab) throw PolizExNotLabel(operand1);
		PolizItem *addr = lab->Get();
		PolizElem *operand2 = Pop(stack);
		PolizInt *val = dynamic_cast<PolizInt*>(operand2);
		if (!val) throw PolizExNotInt(operand2);
		if (val==0)
			*cur_cmd = addr;
		else
			*cur_cmd = (*cur_cmd)->next;
		delete operand1;
		delete operand2;
	}	
};

class SyntaxAnalizer {
	Lexeme_list *cur;
	Lexeme_list *list;
	Lexeme_list *pointer;
	void S();
	void assignment_statement();
	void goto_statement();
	void if_statement();
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
	void add_poliz_list(PolizElem* element) {
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
	void add_label_list(PolizItem* p, char* s) {
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
	void next()
	{
		cur = pointer;
		if ((pointer->next!=0)&&(strcmp(pointer->lex, ".")!=0))
			pointer = pointer->next;
	}
	PolizItem* search_address(char *name);
	bool is_first_label(char *name);
	void add_address(PolizItem *p, char *name);
	void add_address_poliz(PolizItem *p, char *name);
public:
	void set_list(Lexeme_list *lst)
	{
		list = lst;
		pointer = list;
		poliz_list = 0;
		label_list = 0;
		var_list = 0;
	}
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
	const char *GetComment() const
	{
		return comment;
	}
	int GetNstr() const
	{
		return nstr;
	}
	const char *GetLexeme() const
	{
		return lexeme;
	}
private:
	static char *strdup(const char *str);
};

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

void SyntaxAnalizer::analyze()
{
	next();
	S();
	if (strcmp(cur->lex, ".")!=0)
		throw Exeption("dot expected", cur->num_str, cur->lex);
}

void SyntaxAnalizer::S()
{
	if ((cur->type == keyword)&&
		(strcmp(cur->lex, "end")!=0)) {
		if (strcmp(cur->lex, "if")==0) {
			next();
			if_statement();
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
		next();
		assignment_statement();
		S();
	}
	else
	if (cur->type == label) {
		PolizFunNOP* tmp = new PolizFunNOP;
		add_poliz_list(tmp);
		PolizItem* p = poliz_list;
		while (p->next!=0)
			p = p->next;
		if (is_first_label(cur->lex))
			add_label_list(p, cur->lex);
		else {
			add_address(p, cur->lex);
			add_address_poliz(p, cur->lex); //doesn't work FUCK!!!!!!!
		}
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

void SyntaxAnalizer::if_statement()
{
	expression();
	if (strcmp(cur->lex, "then") != 0)
		throw Exeption("then expected", cur->num_str, cur->lex);
	next();
	S();
	if (strcmp(cur->lex, ";") != 0)
		throw Exeption("';' expected", cur->num_str, cur->lex);
	next();
}

void SyntaxAnalizer::goto_statement()
{
	if (cur->type != label)
		throw Exeption("label expected", cur->num_str, cur->lex);
	PolizItem* item = search_address(cur->lex);
	PolizLabel* tmp = new PolizLabel(cur->lex, item); // 0 if goto @m...@m>
	add_poliz_list(tmp);
	if (item == 0)
		add_label_list(0, cur->lex);
	next();
	if (strcmp(cur->lex, ";") !=0 )
		throw Exeption("';' expected", cur->num_str, cur->lex);
	next();
}

void SyntaxAnalizer::print_statement()
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

void SyntaxAnalizer::game_statement()
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

void SyntaxAnalizer::assignment_statement()
{
	if (cur->type == assignment)
		next();
	else
	if (strcmp(cur->lex, "[")==0) {	//index
		next();
		expression();
		if (strcmp(cur->lex, "]")!=0)
			throw Exeption("']' expected", cur->num_str, cur->lex);
		next();
		if (cur->type != assignment)
			throw Exeption("':=' expected", cur->num_str, cur->lex);
		next();
	}
	else
		throw Exeption("':=' or index expected", cur->num_str, cur->lex);
	expression();
	if (strcmp(cur->lex,";")!=0)
		throw Exeption("';' expected", cur->num_str, cur->lex);
	next();
}

void SyntaxAnalizer::expression()
{
	op1();
	ending1();
}

void SyntaxAnalizer::op1()
{
	op2();
	ending2();
}

void SyntaxAnalizer::op2()
{
	op3();
	ending3();
}

void SyntaxAnalizer::op3()
{
	if (cur->type==number) {
		PolizInt* tmp = new PolizInt(atoi(cur->lex));
		add_poliz_list(tmp);
		next();
	} else
	if (cur->type==variable) {
		next();
		if (strcmp(cur->lex, "[")==0) {	//index
			next();
			expression();
			if (strcmp(cur->lex, "]")!=0)
				throw Exeption("']' expected", cur->num_str, cur->lex);
			next();
		}
	}
	else
	if (cur->type==function_without_param)
		next();
	else
	if (cur->type==function_with_param) {
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

void SyntaxAnalizer::ending3()
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

void SyntaxAnalizer::ending2()
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

void SyntaxAnalizer::ending1()
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

PolizItem* SyntaxAnalizer::search_address(char *name)
{
	LabelTable* tmp = label_list;
	while (tmp!=0) {
		if (strcmp(tmp->name, name)==0) {
			printf("found1 %s\n", tmp->name);
			return tmp->pointer;
		}
		tmp = tmp->next;
	}
	return 0;
}

bool SyntaxAnalizer::is_first_label(char *name)
{
	LabelTable* tmp = label_list;
	while (tmp!=0) {
		if (strcmp(tmp->name, name)==0) {
			printf("found2 %s\n", tmp->name);
			return false;
		}
		tmp = tmp->next;
	}
	printf("name first\n");
	return true;
}

void SyntaxAnalizer::add_address(PolizItem *p, char *name)
{
	LabelTable* tmp = label_list;
	while (tmp!=0) {
		if (strcmp(tmp->name, name)==0) {
			printf("name was found\n");
			tmp->pointer = p;
			lprint(tmp->pointer);
			return;
		}
		tmp = tmp->next;
	}
}

void SyntaxAnalizer::add_address_poliz(PolizItem *p, char *name)
{
	PolizItem *tmp;
	while (tmp!=0) {
		printf("xui\n");
		PolizLabel *ptr = dynamic_cast<PolizLabel*>(tmp->elem);
		if (ptr) {
			printf("xui2\n");
			if (strcmp(ptr->Get_name(), name)==0)
				ptr->set_addr(p);
		}
		tmp = tmp->next;
	}
}

class Executer {
	PolizItem* stack;
	PolizItem* prog;
public:
	Executer(PolizItem *lst) {
		prog = lst;
		stack = 0;
	}
	void execute();
};

void lprint(PolizItem* lst)
{
	while (lst!=0) {
		PolizPrint* p2 = dynamic_cast<PolizPrint*>(lst->elem);
		if (p2) printf("print\n");
		PolizString* p1 = dynamic_cast<PolizString*>(lst->elem);
		if (p1) printf("string: %s\n", p1->Get());
		PolizInt* p3 = dynamic_cast<PolizInt*>(lst->elem);
		if (p3) printf("int: %d\n", p3->Get());
		PolizEndTurn* p4 = dynamic_cast<PolizEndTurn*>(lst->elem);
		if (p4) printf("end_turn\n");
		PolizBuild* p5 = dynamic_cast<PolizBuild*>(lst->elem);
		if (p5) printf("build\n");
		PolizProd* p6 = dynamic_cast<PolizProd*>(lst->elem);
		if (p6) printf("prod\n");
		PolizBuy* p7 = dynamic_cast<PolizBuy*>(lst->elem);
		if (p7) printf("buy\n");
		PolizSell* p8 = dynamic_cast<PolizSell*>(lst->elem);
		if (p8) printf("sell\n");
		PolizFunPlus* p9 = dynamic_cast<PolizFunPlus*>(lst->elem);
		if (p9) printf("+\n");
		PolizFunMinus* p10 = dynamic_cast<PolizFunMinus*>(lst->elem);
		if (p10) printf("-\n");
		PolizFunMult* p11 = dynamic_cast<PolizFunMult*>(lst->elem);
		if (p11) printf("*\n");
		PolizFunDiv* p12 = dynamic_cast<PolizFunDiv*>(lst->elem);
		if (p12) printf("/\n");
		PolizFunMod* p13 = dynamic_cast<PolizFunMod*>(lst->elem);
		if (p13) printf("mod\n");
		PolizFunGreater* p14= dynamic_cast<PolizFunGreater*>(lst->elem);
		if (p14) printf(">\n");
		PolizFunLess* p15= dynamic_cast<PolizFunLess*>(lst->elem);
		if (p15) printf("<\n");
		PolizFunEqual* p16= dynamic_cast<PolizFunEqual*>(lst->elem);
		if (p16) printf("=\n");
		PolizFunNOP* p17 = dynamic_cast<PolizFunNOP*>(lst->elem);
		if (p17) printf("NOP\n");
		PolizOpGo* p18 = dynamic_cast<PolizOpGo*>(lst->elem);
		if (p18) printf("goto\n");
		PolizLabel* p19 = dynamic_cast<PolizLabel*>(lst->elem);
		if (p19) printf("label: %s\n", p19->Get_name());
		lst = lst->next;
	}
	printf("\n");
}

void delete_poliz_list(PolizItem* lst)
{
	if (lst!=0) {
		delete_poliz_list(lst->next);
		delete lst->elem;
		delete lst;
	}
}

void print_table_list(LabelTable *lst)
{
	int t =0;
	while (lst!=0) {
		printf("label: %s\n", lst->name);
		if (t==0) {
			PolizInt* p5 = dynamic_cast<PolizInt*>(lst->pointer->next->elem);
			if (p5) printf("label->next -> int\n");
		}
		t = 1;
		lst = lst->next;
	}
}

void Executer::execute() {
	//lprint(prog);
	while (prog!=0) {
		prog->elem->Evaluate(&stack, &prog);
	}
}

int main()
{
	Automat a;
	int c;
	Lexeme_list *list;
	c = getchar();
	try {
		for (;;) {
			if (c==EOF) break;
			a.feed_char(c);
			a.run();
			if (a.full_lex())
				a.add_new_elem();
			if (a.use_prev==0) {
				if (c=='\n')
					a.inc_nstr();
				c = getchar();
			}
			if (a.is_long())
				throw "too long name";
		}
	}
	catch (const char *s) {
		printf("Error: %s\n", s);
		return 1;
	}
	a.print_list();
	if (a.is_any_errors()) {
		printf("Some lexical errors detected!\n");
		return 1;
	}
	list = a.get_list();
	PolizItem *lst;			// PolizItem *lst
	LabelTable *table_lst;
	SyntaxAnalizer s;
	s.set_list(list);
	try {
		s.analyze();
		printf("SUCCESS!!!\n");
		lst = s.poliz_list;
		table_lst = s.label_list;
		lprint(lst);			// print poliz list:
	}
	catch (const Exeption &ex) {
		printf("Error: %s\nbut found %s\nstr# %d\n",
			ex.GetComment(), ex.GetLexeme(), ex.GetNstr());
		return 1;
	}
	Executer e(lst);
	e.execute();
	printf("\n");
	print_table_list(table_lst);
	delete_poliz_list(lst);
	return 0;
}

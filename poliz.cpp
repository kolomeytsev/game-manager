#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <typeinfo>
#include "lexis.h"
#include "poliz.h"
#include "brain.h"

extern RobotsBrain robot;
extern Communication server;

void PolizExNotInt::print_ex() {
	printf("error: poliz is not int\n");
}

void PolizExNotLabel::print_ex() {
	printf("error: poliz is not label\n");
}

void PolizExNotConst::print_ex() {
	printf("poliz is not const\n");
}

void PolizExNotVarAddr::print_ex() {
	printf("poliz is not var address\n");
}

void PolizExNoVar::print_ex() {
	printf("undefined variable\n");
}

VarTable* PolizElem::Get_var_table() {
	return var_table;
}

bool PolizElem::is_first_var(char *name) {
	VarTable* tmp = var_table;
	while (tmp!=0) {
		if (strcmp(tmp->elem->name, name)==0) {
			return false;
		}
		tmp = tmp->next;
	}
	return true;
}

bool PolizElem::search_var(char* name, int* val) {
	VarTable* tmp = var_table;
	while (tmp!=0) {
		if (strcmp(tmp->elem->name, name)==0) {
			*val = tmp->elem->value;
			return true;
		}
		tmp = tmp->next;
	}
	return false;
}

void PolizElem::add_value(char* name, int x) {
	VarTable* tmp = var_table;
	while (tmp!=0) {
		if (strcmp(tmp->elem->name, name)==0) {
			tmp->elem->value = x;
			return;
		}
		tmp = tmp->next;
	}
}

void PolizElem::add_var(char* name, int x) {
	VarTable* tmp = var_table;
	StructVar* tmp_var = new StructVar;
	tmp_var->name = name;
	tmp_var->value = x;
	VarTable* tmp_new = new VarTable;
	tmp_new->elem = tmp_var;
	tmp_new->next = 0;
	if (tmp==0)
		var_table = tmp_new;
	else {
		while (tmp->next!=0)
			tmp = tmp->next;
		tmp->next = tmp_new;
	}
}

void PolizElem::print_var_table() {
	VarTable* tmp = var_table;
	while (tmp!=0) {
		printf("name: %8s, value: %8d\n",
				tmp->elem->name, tmp->elem->value);
		tmp = tmp->next;
	}
}

void PolizElem::Push(PolizItem **stack, PolizElem *elem) {
	PolizItem* tmp = new PolizItem;
	tmp->elem = elem;
	tmp->next = *stack;
	*stack = tmp;
}

PolizElem* PolizElem::Pop(PolizItem **stack) {
	PolizItem* tmp = *stack;
	PolizElem* tmp_elem = tmp->elem;
	*stack = (*stack)->next;
	delete tmp;
	return tmp_elem;
}

void PolizConst::Evaluate(PolizItem **stack,PolizItem **cur_cmd) const {
	Push(stack, Clone());
	*cur_cmd = (*cur_cmd)->next;
}

PolizInt::PolizInt(int a) {
	value = a;
}
PolizElem* PolizInt::Clone()const {
	return new PolizInt(value);
}
int PolizInt::Get() const {
	return value;
}

PolizString::PolizString(char *s) {
	str = s;
}
PolizElem* PolizString::Clone() const {
	int len;
	len = strlen(str);
	char *s = new char[len+1];
	for (int i = 0; i < len + 1; i++)
		s[i] = str[i];
	return new PolizString(s);
}
char* PolizString::Get() const {
	return str;
}

PolizVarAddr::PolizVarAddr(StructVar* a) {
	addr = a;
}
PolizVarAddr::~PolizVarAddr() {
	delete addr->name;
}
PolizElem* PolizVarAddr::Clone() const {
	return new PolizVarAddr(addr);
}
StructVar* PolizVarAddr::Get() const {
	return addr;
}
void PolizVarAddr::set_value(int x) {
	addr->value = x;
}

PolizLabel::PolizLabel(char *s, PolizItem* a) {
	name = s;
	value = a;
}
PolizElem* PolizLabel::Clone() const {
	return new PolizLabel(name, value);
}
PolizItem* PolizLabel::Get() const {
	return value;
}
char* PolizLabel::Get_name() const {
	return name;
}
void PolizLabel::set_addr(PolizItem *ptr) {
	value = ptr;
}

void PolizFunction::
				Evaluate(PolizItem **stack, PolizItem **cur_cmd) const
{
	PolizElem *res = EvaluateFun(stack);
	if (res)
		Push(stack, res);
	*cur_cmd = (*cur_cmd)->next;
}

PolizElem* PolizVar::EvaluateFun(PolizItem **stack) const {
	PolizElem *operand1 = Pop(stack);
	PolizVarAddr *i1 = dynamic_cast<PolizVarAddr*>(operand1);
	if (!i1) throw PolizExNotVarAddr(operand1);
	int value = 0;
	bool t = search_var(i1->Get()->name, &value);
	if (!t) {
		printf("s= %s\n", i1->Get()->name);
		throw PolizExNoVar(operand1);
	}
	return new PolizInt(value);
}

PolizElem* PolizSell::EvaluateFun(PolizItem **stack) const {
	int amount, price;
	PolizElem *operand1 = Pop(stack);
	PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
	if (!i1) throw PolizExNotInt(operand1);
	PolizElem *operand2 = Pop(stack);
	PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
	if (!i2) throw PolizExNotInt(operand2);
	price = i1->Get();
	amount = i2->Get();
	robot.expected_prod_bought = amount;
	robot.expected_prod_price = price;
	char cmd[32] = "sell ";
	char str2[9], str3[9];
	sprintf(str3, "%d ", amount);
	sprintf(str2, "%d\n", price);
	strcat(cmd, str3);
	strcat(cmd, str2);
	printf("cmd_sell = %s", cmd);
	server.send_message(cmd);
	delete operand1;
	delete operand2;
	return 0;
}

PolizElem* PolizBuy::EvaluateFun(PolizItem **stack) const {
	int amount, price;
	PolizElem *operand1 = Pop(stack);
	PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
	if (!i1) throw PolizExNotInt(operand1);
	PolizElem *operand2 = Pop(stack);
	PolizInt *i2 = dynamic_cast<PolizInt*>(operand2);
	if (!i2) throw PolizExNotInt(operand2);
	price = i1->Get();
	amount = i2->Get();
	robot.expected_raw_sold = amount;
	robot.expected_raw_price = price;
	char cmd[32] = "buy ";
	char str1[9], str2[9];
	sprintf(str1, "%d\n", price);
	sprintf(str2, "%d ", amount);
	strcat(cmd, str2);
	strcat(cmd, str1);
	printf("cmd_buy = %s", cmd);
	server.send_message(cmd);
	delete operand1;
	delete operand2;
	return 0;
}

PolizElem* PolizProd::EvaluateFun(PolizItem **stack) const {
	int amount;
	PolizElem *operand1 = Pop(stack);
	PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
	if (!i1) throw PolizExNotInt(operand1);
	amount = i1->Get();
	char cmd[16] = "prod ", str1[9];
	sprintf(str1, "%d\n", amount);
	strcat(cmd, str1);
	printf("cmd_prod = %s", cmd);
	server.send_message(cmd);
	delete operand1;
	return 0;
}

PolizElem* PolizBuild::EvaluateFun(PolizItem **stack) const {
	char cmd[8] = "build\n";
	printf("cmd_build\n");
	server.send_message(cmd);
	robot.begin_constructing();
	return 0;
}

PolizElem* PolizEndTurn::EvaluateFun(PolizItem **stack) const {
	char cmd0[32] = "# Trading results";
	char cmd[16] = "& ENDTURN";
	char cmd2[16] = "turn\n";
	char *str;
	int t = 0;
	robot.set_auct();
	server.send_message(cmd2);
	for (;;) {
		str = server.read_message();
		if (strncmp(str, cmd0, 16)==0) { //in auction table
			for (;;) {
				delete str;
				str = server.read_message();
				if ((strncmp(str, "& BOUGHT", 8)==0)||
					(strncmp(str, "& SOLD", 6)==0)){
					t = 1;
					robot.add_new_elem(str);
				}
				else {
					if (t==1) {
						break;
					}
				}
			}
		} // out auction table
		if (strncmp(str, "# You are a bankrupt, sorry.", 28)==0) {
			printf("%s", str);
			delete str;
			robot.free_auct_list();
			exit(0);
		}
		if (strncmp(str, cmd, 9)==0) {
			delete str;
			break;
		}
		delete str;
	}
	return 0;
}

PolizElem* PolizPrint::EvaluateFun(PolizItem **stack) const {
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

PolizElem* PolizFunPlus::EvaluateFun(PolizItem **stack) const {
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

PolizElem* PolizFunMinus::EvaluateFun(PolizItem **stack) const {
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

PolizElem* PolizFunMult::EvaluateFun(PolizItem **stack) const {
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

PolizElem* PolizFunDiv::EvaluateFun(PolizItem **stack) const {
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

PolizElem* PolizFunMod::EvaluateFun(PolizItem **stack) const {
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

PolizElem* PolizFunGreater::EvaluateFun(PolizItem **stack) const {
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

PolizElem* PolizFunLess::EvaluateFun(PolizItem **stack) const {
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

PolizElem* PolizFunEqual::EvaluateFun(PolizItem **stack) const {
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

PolizElem* PolizFunIndex::EvaluateFun(PolizItem **stack) const {
	PolizElem *operand1 = Pop(stack);
	PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
	if (!i1) throw PolizExNotInt(operand1);
	PolizElem *operand2 = Pop(stack);
	PolizString *i2 = dynamic_cast<PolizString*>(operand2);
	if (!i2) throw PolizExNotVarAddr(operand2);
	int len = strlen(i2->Get());
	char* str = new char[16];
	int len_int;
	len_int = sprintf(str, "%d", i1->Get());
	char* s = new char[len + len_int + 2 + 1];
	s[0] = 0;
	strcat(s, i2->Get());
	strcat(s, "[");
	strcat(s, str);
	strcat(s, "]");
	s[len + len_int + 2] = '\0';
	StructVar* tmp = new StructVar;
	tmp->name = s;
	delete operand1;
	delete operand2;
	return new PolizVarAddr(tmp);
}

PolizElem* PolizFunAssign::EvaluateFun(PolizItem **stack) const {
	PolizElem *operand1 = Pop(stack);
	PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
	if (!i1) throw PolizExNotInt(operand1);
	PolizElem *operand2 = Pop(stack);
	PolizVarAddr *i2 = dynamic_cast<PolizVarAddr*>(operand2);
	if (!i2) throw PolizExNotVarAddr(operand2);
	i2->set_value(i1->Get());
	if (is_first_var(i2->Get()->name))
		add_var(i2->Get()->name, i1->Get());
	else
		add_value(i2->Get()->name, i1->Get());
	return 0;
}

PolizFunNoParam::PolizFunNoParam(char* s)
{
	name = s;
}

PolizFunNoParam::~PolizFunNoParam()
{
	delete name;
}

char* PolizFunNoParam::Get_name()
{
	return name;
}

PolizElem* PolizFunNoParam::EvaluateFun(PolizItem **stack) const {
	int res = 0;
	if (strcmp(name, "my_id")==0)
		res = 1;
	if (strcmp(name, "turn")==0)
		res = robot.get_turn();
	if (strcmp(name, "active_players")==0)
		res = robot.get_active_players();
	if (strcmp(name, "supply")==0)
		res = robot.get_supply();
	if (strcmp(name, "raw_price")==0)
		res = robot.get_raw_price();
	if (strcmp(name, "demand")==0)
		res = robot.get_demand();
	if (strcmp(name, "production_price")==0)
		res = robot.get_prod_price();
	if (strcmp(name, "factories_being_build")==0) {
		res = robot.factories;
		if (res > 0)
			res = 1;
		else
			res = 0;
	}
	if (strcmp(name, "expected_raw_sold")==0)
		res = robot.expected_raw_sold;
	if (strcmp(name, "expected_prod_bought")==0)
		res = robot.expected_prod_bought;
	if (strcmp(name, "expected_raw_price")==0)
		res = robot.expected_raw_price;
	if (strcmp(name, "expected_prod_price")==0)
		res = robot.expected_prod_price;
	if (strcmp(name, "prev_raw_price")==0)
		res = robot.prev_raw_price;
	if (strcmp(name, "prev_prod_price")==0)
		res = robot.prev_prod_price;
	return new PolizInt(res);
}

PolizFunParam::PolizFunParam(char* s)
{
	name = s;
}

PolizFunParam::~PolizFunParam()
{
	delete name;
}

char* PolizFunParam::Get_name()
{
	return name;
}

PolizElem* PolizFunParam::EvaluateFun(PolizItem **stack) const {
	Info_player* tmp;
	int res = 0, i, j, max;
	PolizElem *operand1 = Pop(stack);
	PolizInt *i1 = dynamic_cast<PolizInt*>(operand1);
	if (!i1) throw PolizExNotInt(operand1);
	i = i1->Get();
	if (strcmp(name, "money")==0) {
		tmp = robot.get_info();
		max = robot.get_active_players();
		if (i > max) {
			printf("error algorithm1");
			exit(1);
		}
		for (j = 1; j < i; j++)
			tmp = tmp->next;
		res = tmp->money;
	}
	if (strcmp(name, "raw")==0) {
		tmp = robot.get_info();
		max = robot.get_active_players();
		if (i > max) {
			printf("error algorithm2");
			exit(1);
		}
		for (j = 1; j < i; j++)
			tmp = tmp->next;
		res = tmp->raw;
	}
	if (strcmp(name, "production")==0) {
		tmp = robot.get_info();
		max = robot.get_active_players();
		if (i > max) {
			printf("error algorithm3");
			exit(1);
		}
		for (j = 1; j < i; j++)
			tmp = tmp->next;
		res = tmp->prod;
	}
	if (strcmp(name, "factories")==0) {
		tmp = robot.get_info();
		max = robot.get_active_players();
		if (i > max) {
			printf("error algorithm4 i = %d\n", i);
			exit(1);
		}
		for (j = 1; j < i; j++)
			tmp = tmp->next;
		res = tmp->plant;
	}
	if (strcmp(name, "manufactured")==0) {
	}
	if (strcmp(name, "result_raw_sold")==0) {
		tmp = robot.get_info();
		max = robot.get_active_players();
		if (i > max) {
			printf("error algorithm5");
			exit(1);
		}
		for (j = 1; j < i; j++)
			tmp = tmp->next;
		res = robot.search_raw_sold(tmp->name);
	}
	if (strcmp(name, "result_raw_price")==0) {
		tmp = robot.get_info();
		max = robot.get_active_players();
		if (i > max) {
			printf("error algorithm6");
			exit(1);
		}
		for (j = 1; j < i; j++)
			tmp = tmp->next;
		res = robot.search_raw_price(tmp->name);
	}
	if (strcmp(name, "result_prod_bought")==0) {
		tmp = robot.get_info();
		max = robot.get_active_players();
		if (i > max) {
			printf("error algorithm7");
			exit(1);
		}
		for (j = 1; j < i; j++)
			tmp = tmp->next;
		res = robot.search_prod_sold(tmp->name);
	}
	if (strcmp(name, "result_prod_price")==0) {
		tmp = robot.get_info();
		max = robot.get_active_players();
		if (i > max) {
			printf("error algorithm8");
			exit(1);
		}
		for (j = 1; j < i; j++)
			tmp = tmp->next;
		res = robot.search_prod_price(tmp->name);
	}
	return new PolizInt(res);
}

void PolizOpGo::Evaluate(PolizItem **stack, PolizItem **cur_cmd) const {
	PolizElem *operand1 = Pop(stack);
	PolizLabel *lab = dynamic_cast<PolizLabel*>(operand1);
	if (!lab) throw PolizExNotLabel(operand1);
	PolizItem *addr = lab->Get();
	*cur_cmd = addr;
	delete operand1;
}

void PolizOpGoFalse::Evaluate(PolizItem **stack, PolizItem **cur_cmd) const {
	PolizElem *operand1 = Pop(stack);
	PolizLabel *lab = dynamic_cast<PolizLabel*>(operand1);
	if (!lab) throw PolizExNotLabel(operand1);
	PolizItem *addr = lab->Get();
	PolizElem *operand2 = Pop(stack);
	PolizInt *val = dynamic_cast<PolizInt*>(operand2);
	if (!val) throw PolizExNotInt(operand2);
	if (val->Get()==0)
		*cur_cmd = addr;
	else
		*cur_cmd = (*cur_cmd)->next;
	delete operand1;
	delete operand2;
}

Executer::Executer(PolizItem *lst) {
	prog = lst;
	stack = 0;
}
void Executer::set(PolizItem *lst) {
	prog = lst;
	stack = 0;
}
PolizItem* Executer::Get_stack() {
	return stack;
}
void Executer::execute() {
	while (prog!=0) {
		prog->elem->Evaluate(&stack, &prog);
	}
}

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
		if (p9) printf("op: +\n");
		PolizFunMinus* p10 = dynamic_cast<PolizFunMinus*>(lst->elem);
		if (p10) printf("op: -\n");
		PolizFunMult* p11 = dynamic_cast<PolizFunMult*>(lst->elem);
		if (p11) printf("op: *\n");
		PolizFunDiv* p12 = dynamic_cast<PolizFunDiv*>(lst->elem);
		if (p12) printf("op: /\n");
		PolizFunMod* p13 = dynamic_cast<PolizFunMod*>(lst->elem);
		if (p13) printf("mod\n");
		PolizFunGreater* p14= dynamic_cast<PolizFunGreater*>(lst->elem);
		if (p14) printf("op: >\n");
		PolizFunLess* p15= dynamic_cast<PolizFunLess*>(lst->elem);
		if (p15) printf("op: <\n");
		PolizFunEqual* p16= dynamic_cast<PolizFunEqual*>(lst->elem);
		if (p16) printf("op: =\n");
		PolizFunNOP* p17 = dynamic_cast<PolizFunNOP*>(lst->elem);
		if (p17) printf("NOP\n");
		PolizOpGo* p18 = dynamic_cast<PolizOpGo*>(lst->elem);
		if (p18) printf("goto\n");
		PolizLabel* p19 = dynamic_cast<PolizLabel*>(lst->elem);
		if (p19) printf("label: %s\n", p19->Get_name());
		PolizFunParam* p20 = dynamic_cast<PolizFunParam*>(lst->elem);
		if (p20) printf("fun with param: %s\n", p20->Get_name());
		PolizFunNoParam* p21 = dynamic_cast<PolizFunNoParam*>(lst->elem);
		if (p21) printf("fun without param: %s\n", p21->Get_name());
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

void delete_label_list(LabelTable* lst)
{
	if (lst!=0) {
		delete_label_list(lst->next);
		delete lst->name;
		delete lst;
	}
}

void delete_var_list(VarTable* lst)
{
	if (lst!=0) {
		delete_var_list(lst->next);
		delete lst->elem->name;
		delete lst->elem;
		delete lst;
	}
}	

void print_lable_list(LabelTable *lst)
{
	int t = 0;
	while (lst!=0) {
		printf("label: %s\n", lst->name);
		if (t==0) {
			PolizInt* p5 = dynamic_cast<PolizInt*>
							(lst->pointer->next->elem);
			if (p5) printf("label->next -> int\n");
		}
		t = 1;
		lst = lst->next;
	}
}

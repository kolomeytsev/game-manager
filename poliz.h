#ifndef POLIZ_H
#define POLIZ_H

class PolizElem;

struct PolizItem {
	PolizElem* elem;
	struct PolizItem* next;
};

struct StructVar {
	char *name;
	int value;
};

struct VarTable {
	StructVar* elem;
	struct VarTable* next;
};

struct LabelTable {
	char* name;
	PolizItem* pointer;
	struct LabelTable* next;
};

void lprint(PolizItem* lst);
void delete_poliz_list(PolizItem* lst);
void delete_label_list(LabelTable* lst);
void delete_var_list(VarTable* lst);
void print_lable_list(LabelTable *lst);

class PolizEx {
	char *comment;
public:
	virtual ~PolizEx() {}
	virtual void print_ex() = 0;
};

class PolizExNotInt: public PolizEx {
public:
	PolizExNotInt(PolizElem *elem) {}
	virtual void print_ex();
};

class PolizExNotLabel: public PolizEx {
public:
	PolizExNotLabel(PolizElem *elem) {}
	virtual void print_ex();
};

class PolizExNotConst: public PolizEx {
public:
	PolizExNotConst(PolizElem *elem) {}
	virtual void print_ex();
};

class PolizExNotVarAddr: public PolizEx {
public:
	PolizExNotVarAddr(PolizElem *elem) {}
	virtual void print_ex();
};

class PolizExNoVar: public PolizEx {
public:
	PolizExNoVar(PolizElem *elem) {}
	virtual void print_ex();
};

class PolizElem {
public:
	virtual ~PolizElem() {}
	virtual void Evaluate(PolizItem **stack,
							PolizItem **cur_cmd) const = 0;
	static VarTable* var_table;		//	variable table
	static bool is_first_var(char *name);
	static void add_value(char* name, int x);
	static void add_var(char* name, int x);
	static void print_var_table();
	static bool search_var(char* name, int *val);
	static VarTable* Get_var_table();
protected:
	static void Push(PolizItem **stack, PolizElem *elem);
	static PolizElem* Pop(PolizItem **stack);
};

class PolizConst: public PolizElem {
public:
	virtual void Evaluate(PolizItem **stack,PolizItem **cur_cmd) const;
	virtual PolizElem* Clone() const = 0;
};

class PolizInt: public PolizConst {
	int value;
public:
	PolizInt(int a);
	virtual ~PolizInt() {}
	virtual PolizElem* Clone()const;
	int Get() const;
};

class PolizString: public PolizConst {
	char *str;
public:
	PolizString(char *s);
	virtual ~PolizString() {}
	virtual PolizElem* Clone() const;
	char *Get() const;
};

class PolizVarAddr: public PolizConst {
	StructVar* addr;
public:
	PolizVarAddr(StructVar* a);
	virtual ~PolizVarAddr();
	virtual PolizElem* Clone() const;
	StructVar* Get() const;
	void set_value(int x);
};

class PolizLabel: public PolizConst {
	char *name;
	PolizItem* value;
public:
	PolizLabel(char *s, PolizItem* a);
	virtual ~PolizLabel() {}
	virtual PolizElem* Clone() const;
	PolizItem* Get() const;
	char* Get_name() const;
	void set_addr(PolizItem *ptr);
};

class PolizFunction: public PolizElem {
public:
	virtual void Evaluate(PolizItem **stack, PolizItem **cur_cmd) const;
	virtual PolizElem* EvaluateFun(PolizItem **stack) const = 0;
};

class PolizVar: public PolizFunction {
public:
	PolizVar() {}
	virtual ~PolizVar() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizSell: public PolizFunction {
public:
	PolizSell() {}
	virtual ~PolizSell() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizBuy: public PolizFunction {
public:
	PolizBuy() {}
	virtual ~PolizBuy() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizProd: public PolizFunction {
public:
	PolizProd() {}
	virtual ~PolizProd() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizBuild: public PolizFunction {
public:
	PolizBuild() {}
	virtual ~PolizBuild() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizEndTurn: public PolizFunction {
public:
	PolizEndTurn() {}
	virtual ~PolizEndTurn() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizPrint: public PolizFunction {
public:
	PolizPrint() {}
	virtual ~PolizPrint() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunNOP: public PolizFunction {
public:
	PolizFunNOP() {}
	virtual ~PolizFunNOP() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const {
		return 0;
	} 
};

class PolizFunPlus: public PolizFunction {
public:
	PolizFunPlus() {}
	virtual ~PolizFunPlus() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunMinus: public PolizFunction {
public:
	PolizFunMinus() {}
	virtual ~PolizFunMinus() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunMult: public PolizFunction {
public:
	PolizFunMult() {}
	virtual ~PolizFunMult() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunDiv: public PolizFunction {
public:
	PolizFunDiv() {}
	virtual ~PolizFunDiv() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunMod: public PolizFunction {
public:
	PolizFunMod() {}
	virtual ~PolizFunMod() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunGreater: public PolizFunction {
public:
	PolizFunGreater() {}
	virtual ~PolizFunGreater() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunLess: public PolizFunction {
public:
	PolizFunLess() {}
	virtual ~PolizFunLess() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunEqual: public PolizFunction {
public:
	PolizFunEqual() {}
	virtual ~PolizFunEqual() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunIndex: public PolizFunction {
public:
	PolizFunIndex() {}
	virtual ~PolizFunIndex() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunAssign: public PolizFunction {
public:
	PolizFunAssign() {}
	virtual ~PolizFunAssign() {}
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunNoParam: public PolizFunction {
	char* name;
public:
	PolizFunNoParam(char* s);
	char* Get_name();
	virtual ~PolizFunNoParam();
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizFunParam: public PolizFunction {
	char* name;
public:
	PolizFunParam(char* s);
	char* Get_name();
	virtual ~PolizFunParam();
	virtual PolizElem* EvaluateFun(PolizItem **stack) const;
};

class PolizOpGo: public PolizElem {
public:
	PolizOpGo() {}
	virtual ~PolizOpGo() {}
	virtual void Evaluate(PolizItem **stack, PolizItem **cur_cmd) const;
};

class PolizOpGoFalse: public PolizElem {
public:
	PolizOpGoFalse() {}
	virtual ~PolizOpGoFalse() {}
	virtual void Evaluate(PolizItem **stack, PolizItem **cur_cmd) const;
};

class Executer {
	PolizItem* stack;
	PolizItem* prog;
public:
	Executer(PolizItem *lst);
	void set(PolizItem *lst);
	PolizItem* Get_stack();
	void execute();
};

#endif

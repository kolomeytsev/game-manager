#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <typeinfo>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "lexis.h"
#include "poliz.h"
#include "syntax.h"
#include "brain.h"

VarTable* PolizElem::var_table = 0;
extern RobotsBrain robot;
extern Communication server;

void full_argc(int argc)
{
	if (argc != 6) {
		printf("wrong paramets!\n");
		exit(1);
	}
}

int main(int argc, char** argv)
{
	full_argc(argc);
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
			if (a.is_long()) throw "too long name";
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
	PolizItem *lst;
	LabelTable *lable_lst;
	VarTable *var_lst;
	SyntaxAnalyzer s;
	s.set_list(list);
	try {
		s.analyze();
		printf("SUCCESSFULL ANALYZATION!!!\n\n");
		lst = s.poliz_list;
		lable_lst = s.label_list;
	}
	catch (const Exeption &ex) {
		printf("Error: %s\nbut found %s\nstr# %d\n",
			ex.GetComment(), ex.GetLexeme(), ex.GetNstr());
		return 1;
	}
	lprint(lst);
	server.set(atoi(argv[2]), argv[1]);
	server.connection();
	robot.set(argv, server);
	robot.readnsend_name();
	robot.join_or_create();
	printf("start playing\n");
	robot.start_easy_game(lst);
	var_lst = PolizElem::Get_var_table();
	delete_var_list(var_lst);
	delete_label_list(lable_lst);
	delete_poliz_list(lst);
	return 0;
}

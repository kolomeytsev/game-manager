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

RobotsBrain robot;
Communication server;

Communication::Communication()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd==-1) {
		perror("socket");
		exit(1);
	}
	buf_len = 0;
}

void Communication::set(int prt, char *ip)
{
	port = prt;
	serv_ip = ip;
}

void Communication::reset_buf()
{
	buf_len = 0;
}

void Communication::connection()
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (!inet_aton(serv_ip, &(addr.sin_addr))) {
		perror("IP");
		exit(1);
	}
	if (0!=connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)))
	{
		perror("connect");
		exit(1);
	}
}

char* Communication::read_message()
{
	char *str1;
	char str[256];
	int rc, i, k = 0;
	while (full_string()==0) {
		rc = read(sockfd, str, sizeof(str));
		for (i=buf_len; i<buf_len + rc; i++) {
			buf[i] = str[k];
			k++;
		}
		buf_len = buf_len + rc;
	}
	str1 = make_full_string();
	return str1;
}

int Communication::full_string()
{
	int i;
	for (i = 0; i < buf_len; i++) {
		if (buf[i] == '\n')
			return 1;
	}
	return 0;
}

char* Communication::make_full_string()
{
	int i=0, j, k=0;
	char *str = new char[256];
	while (buf[i]!='\n') {	//copy buf to str
		str[i] = buf[i];
		i++;
	}
	str[i]='\n';
	i++;
	str[i] = '\0';
	for (j = i; j < buf_len; j++) {
		buf[k] = buf[j];	//shift buf
		k++;
	}
	buf_len = buf_len - i;
	return str;
}

void Communication::send_message(char *str)
{
	int len;
	len = strlen(str);
	write(sockfd, str, len);
}

void Communication::read_name()
{
	char str[32];
	int rc;
	rc = read(sockfd, str, sizeof(str));
	if (rc!=24)
		printf("ERROR\n");
}

void Communication::buf_print()
{
	write(1, buf, buf_len);
	printf("buf_printf = %d\n", buf_len);
}

void RobotsBrain::set(char **argv, Communication serv)
{
	server = serv;
	auct = 0;
	turn = 0;
	factories = 0;
	expected_raw_sold = 0;
	expected_prod_bought = 0;
	prev_raw_price = 0;
	prev_prod_price = 0;
	void set_prev_price();
	int len;
	char cmd1[] = "create";
	char cmd2[] = "join";
	len = strlen(argv[3]);
	name = new char[len+2];
	strcpy(name, argv[3]);
	name[len] = '\n';
	name[len+1] = '\0';
	if (strcmp(argv[4], cmd1)==0)
		mode = 0;
	else {
		if (strcmp(argv[4], cmd2)==0)
			mode = 1;
		else {
			printf("wrong 4th param!\n");
			exit(1);
		}
	}
	arg = argv[5];
}

RobotsBrain::~RobotsBrain()
{
	delete name;
}

void RobotsBrain::set_auct()
{
	auct = 0;
}

void RobotsBrain::begin_constructing()
{
	factories = 5;
}

void RobotsBrain::process_factories()
{
	if (factories > 0)
		factories--;
}

void RobotsBrain::join_or_create()
{
	if (mode)
		join();
	else
		create();
}

void RobotsBrain::join()
{
	char *str;
	char cmd[16] = ".join ";
	char cmd2[16] = "& START\n";
	strcat(cmd, arg);
	strcat(cmd, "\n");
	server.send_message(cmd);
	for (;;) {
		str = server.read_message();
		if (strncmp(str, "%- Couldn't join the game",
			strlen("%- Couldn't join the game"))==0)
			exit(1);
		if (strcmp(str, cmd2)==0) {
			delete str;
			break;
		}
		delete str;
	}
}

void RobotsBrain::create()
{
	char *str;
	int num_now = 0, num_all;
	num_all = atoi(arg);
	char cmd1[] = ".create\n";
	char start[] = "start\n";
	server.send_message(cmd1);
	for(;;) {
		str = server.read_message();
		if ((str[0]=='@')&&(str[1]=='+'))
			num_now++;
		if ((str[0]=='@')&&(str[1]=='-'))
			num_now--;
		if (num_now==num_all) {
			delete str;
			break;
		}
		delete str;
	}
	server.send_message(start);
}

void RobotsBrain::readnsend_name()
{
	server.read_name();
	server.send_message(name);
}

void RobotsBrain::process_info()
{
	char *str, *nm;
	char cmd[10] = "info\n";
	char cmd2[10] = "& INFO";
	int len, res, len2;
	players = 0;
	server.send_message(cmd);
	for (;;) {
		str = server.read_message();
		if (strncmp(str, cmd2, 6)==0) {
			nm = new char[16];
			sscanf(str + 8, "%s", nm);
			printf("nm = %s\n", nm);
			len = strlen(nm);
			len2 = strlen(name) - 1;
			if (len2 > len)
				len = len2;
			if (strncmp(name, nm, len)==0)
				add_new_info_to_beg(nm, str);
			else
				add_new_info_to_end(nm, str);
		}
		if (strncmp(str, "& PLAYERS", 9)==0) {
			sscanf(str + 10, "%d", &res);
			active_players = res;
			delete str;
			break;
		}
		delete str;
	}
}

int RobotsBrain::get_num(char *str, int *k)
{
	char num1[10] = "0";
	int i, len, ret;
	len = strlen(str);
	for (i=*k; i<len; i++) {
		if ((str[i]>='0')&&(str[i]<='9')) {
			strncat(num1, str+i, 1);
			i++;
			for(i=i; i<len; i++) {
				if (str[i]==' ')
					break;
				if ((str[i]>='0')&&(str[i]<='9'))
					strncat(num1, str+i,1);
			}
			break;
		}
	}
	*k = i;
	ret = atoi(num1);
	return ret;
}

void RobotsBrain::process_market()
{
	char *str;
	char cmd[10] = "market\n";
	char cmd2[10] = "& MARKET";
	int i = 0;
	server.send_message(cmd);
	for (;;) {
		str = server.read_message();
		if (strncmp(str, "&- The game is over\n",
			strlen("&- The game is over"))==0) {
			printf("YOU WIN!!!\n%s", str);
			free_list(auct);
			delete str;
			//delete name;
			exit(0);
		}
		if (strncmp(str, cmd2, 8)==0) {
			market.raw = get_num(str, &i);
			printf("raw = %d\n", market.raw);
			market.min_price = get_num(str, &i);
			printf("min_price = %d\n", market.min_price);
			market.prod = get_num(str, &i);
			printf("prod = %d\n", market.prod);
			market.max_price = get_num(str, &i);
			printf("max_price = %d\n", market.max_price);
			delete str;
			break;
		}
		delete str;
	}
}

void RobotsBrain::add_new_info_to_beg(char* nm, char* str)
{
	Info_player *tmp;
	int r, p, m, plant, a_plant;
	tmp = new Info_player;
	tmp->name = nm;
	sscanf(str + 25, "%d %d %d %d %d", &r, &p, &m, &plant, &a_plant);
	tmp->money = m;
	tmp->raw = r;
	tmp->prod = p;
	tmp->plant = plant;
	tmp->auto_plant = a_plant;
	tmp->next = players;
	players = tmp;
}

void RobotsBrain::add_new_info_to_end(char* nm, char* str)
{
	Info_player* lst = players, *tmp;
	int r, p, m, plant, a_plant;
	tmp = new Info_player;
	tmp->name = nm;
	sscanf(str + 25, "%d %d %d %d %d", &r, &p, &m, &plant, &a_plant);
	tmp->money = m;
	tmp->raw = r;
	tmp->prod = p;
	tmp->plant = plant;
	tmp->auto_plant = a_plant;
	tmp->next = 0;
	if (players == 0)
		players = tmp;
	else {
		while (lst->next != 0)
			lst = lst->next;
		lst->next = tmp;
	}
}

void RobotsBrain::add_new_elem(char* str)
{
	int i = 30;
	struct Info_auction *lst_tmp, *tmp;
	tmp = auct;
	lst_tmp = new struct Info_auction;
	if (strncmp(str, "& BOUGHT", 8)==0)
		lst_tmp->mode = 0;
	else
		lst_tmp->mode = 1;
	sscanf(str + 8, "%s", lst_tmp->name);
	lst_tmp->amount = get_num(str, &i);
	lst_tmp->price = get_num(str, &i);
	lst_tmp->next = NULL;
	if (auct == 0)
		auct = lst_tmp;
	else {
		while (tmp->next!=0)
			tmp = tmp->next;
		tmp->next = lst_tmp;
	}
}

void RobotsBrain::print_info() {
	Info_player* tmp = players;
	while (tmp != 0) {
		printf("%10s, %7d, %3d, %3d plants: %3d %3d\n", tmp->name,
			tmp->money, tmp->raw, tmp->prod, tmp->plant, 
			tmp->auto_plant);
		tmp = tmp->next;
	}
}

void RobotsBrain::print_auct_list(Info_auction *lst)
{
	int i = 1;
	printf("Results of auction:\n");
	while (lst!=0) {
		printf("%d)   %10s, %5d, %5d, mode = %d\n",
				i, lst->name, lst->amount, lst->price, lst->mode);
		i++;
		lst = lst->next;
	}
}

void RobotsBrain::free_list(Info_auction *lst)
{
	if (lst!=0) {
		free_list(lst->next);
		delete lst;
	}
}

void RobotsBrain::free_auct_list()
{
	free_list(auct);
}

void RobotsBrain::free_info_list(Info_player* lst)
{
	if (lst!=0) {
		free_info_list(lst->next);
		delete lst->name;
		delete lst;
	}
}

char* RobotsBrain::get_name()
{
	return name;
}

int RobotsBrain::get_turn()
{
	return turn;
}

int RobotsBrain::get_active_players()
{
	return active_players;
}

int RobotsBrain::get_supply()
{
	return market.raw;
}

int RobotsBrain::get_demand()
{
	return market.prod;
}

int RobotsBrain::get_raw_price()
{
	return market.min_price;
}

int RobotsBrain::get_prod_price()
{
	return market.max_price;
}

Info_player* RobotsBrain::get_info()
{
	return players;
}

Info_auction* RobotsBrain::get_auct()
{
	return auct;
}

int RobotsBrain::search_raw_sold(char* nm)
{
	Info_auction* tmp = auct;
	while (tmp != 0) {
		if ((tmp->mode == 0)&&(strcmp(tmp->name, nm)==0))
			return tmp->amount;
		tmp = tmp->next;
	}
	return 0;
}

int RobotsBrain::search_raw_price(char* nm)
{
	Info_auction* tmp = auct;
	while (tmp != 0) {
		if ((tmp->mode == 0)&&(strcmp(tmp->name, nm)==0))
			return tmp->price;
		tmp = tmp->next;
	}
	return 0;
}

int RobotsBrain::search_prod_sold(char* nm)
{
	Info_auction* tmp = auct;
	while (tmp != 0) {
		if ((tmp->mode == 1)&&(strcmp(tmp->name, nm)==0))
			return tmp->amount;
		tmp = tmp->next;
	}
	return 0;
}

int RobotsBrain::search_prod_price(char* nm)
{
	Info_auction* tmp = auct;
	while (tmp != 0) {
		if ((tmp->mode == 1)&&(strcmp(tmp->name, nm)==0))
			return tmp->price;
		tmp = tmp->next;
	}
	return 0;
}

void RobotsBrain::set_prev_price()
{
	prev_raw_price = market.min_price;
	prev_prod_price = market.max_price;
}

void RobotsBrain::start_easy_game(PolizItem *lst)
{
	Executer e(lst);
	for (;;) {
		turn++;
		process_factories();
		process_market();
		process_info();
		print_info();
		try {
			e.set(lst);
			e.execute();
		}
		catch(PolizEx &poliz_ex) {
			PolizEx* tmp = &poliz_ex;
			tmp->print_ex();
			exit(1);
		}
		set_prev_price();
		print_auct_list(auct);
		if (players!=0)
			free_info_list(players);
	}
}

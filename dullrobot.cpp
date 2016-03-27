#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

struct Info_market {
	int raw;
	int min_price;
	int prod;
	int max_price;
};

struct Info_player {
	char* name;
	int raw;
	int prod;
	int money;
	Info_player* next;
};

struct Info_auction {
	int mode; //0 - BOUGHT, 1 - SOLD
	char name[20];
	int amount;
	int price;
	struct Info_auction *next;
};

class Communication {
	int sockfd, port, buf_len;
	char *serv_ip;
	char buf[512];
public:
	Communication();
	void set(int prt, char *ip);
	void connection();
	char* read_message();
	int full_string();
	char* make_full_string();
	void send_message(char *str);
	void buf_print();
	void reset_buf();
	void read_name();
};

class RobotsBrain {
	char *name;
	int mode; // 0 - create, 1 - join
	char *arg;
	Info_market market;
	Info_player player;
	Info_auction* auct;
	Info_player* players;
	Communication server;
	int active_players;
public:
	void set(char **argv, Communication serv);
	~RobotsBrain();
	void readnsend_name();
	void join_or_create();
	void join();
	void create();
	void start_game();
	void process_market();
	void process_info();
	int get_num(char *str, int *k);
	void buy_and_sell();
	void wait4turn();
	void produce();
	void add_new_elem(char* str);
	void print_auct_list(Info_auction *lst);
	void free_list(Info_auction *lst);
	void add_new_info_to_beg(char* nm, char* str);
	void add_new_info_to_end(char* nm, char* str);
	void print_info();
	void free_info_list(Info_player* lst);
};

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

void Communication::buf_print() {
	write(1, buf, buf_len);
	printf("buf_printf = %d\n", buf_len);
}

void RobotsBrain::set(char **argv, Communication serv) {
	server = serv;
	auct = 0;
	players = 0;
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

void RobotsBrain::start_game()
{
	for (;;) {
		process_market();
		process_info();
		print_info();
		buy_and_sell();
		produce();
		if (auct!=0) {
			print_auct_list(auct);
			free_list(auct);
		}
		if (players!=0)
			free_info_list(players);
		wait4turn();
	}
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
			printf("str = %s", str);
			free_list(auct);
			delete str;
			delete name;
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

void RobotsBrain::process_info()
{
	char *str, *nm;
	char cmd[10] = "info\n";
	char cmd2[10] = "& INFO";
	int len, res;
	players = 0;
	server.send_message(cmd);
	for (;;) {
		str = server.read_message();
		if (strncmp(str, cmd2, 6)==0) {
			nm = new char[16];
			sscanf(str + 8, "%s", nm);
			printf("nm = %s\n", nm);
			len = strlen(nm);
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

void RobotsBrain::buy_and_sell()
{
	char *str;
	char cmd1[32] = "buy 2 ";
	char cmd2[32] = "sell ";
	char str1[9], str2[9], str3[9];
	sprintf(str1, "%d\n", market.min_price);
	sprintf(str2, "%d\n", market.max_price);
	sprintf(str3, "%d ", players->prod);
	strcat(cmd1, str1);
	strcat(cmd2, str3);
	strcat(cmd2, str2);
	printf("cmd1 = %s", cmd1);
	printf("cmd2 = %s", cmd2);
	server.send_message(cmd1);
	str = server.read_message();
	//printf("str = %s", str);
	delete str;
	server.send_message(cmd2);
	str = server.read_message();
	//printf("str = %s", str);
	delete str;
}

void RobotsBrain::produce()
{
	char cmd0[16] = "prod 0\n";
	char cmd1[16] = "prod 1\n";
	char cmd2[16] = "prod 2\n";
	if (players->raw == 0) {
		server.send_message(cmd0);
		printf("cmd3 = %s", cmd0);
	}
	if (players->raw == 1) {
		server.send_message(cmd1);
		printf("cmd3 = %s", cmd1);
	}
	if (players->raw >= 2) {
		server.send_message(cmd2);
		printf("cmd3 = %s", cmd2);
	}
}

void RobotsBrain::wait4turn()
{
	char cmd0[32] = "# Trading results";
	char cmd[16] = "& ENDTURN";
	char cmd2[16] = "turn\n";
	char *str;
	int t = 0;
	auct = 0;
	server.send_message(cmd2);
	for (;;) {
		str = server.read_message();
		//printf("str = %s", str);
		if (strncmp(str, cmd0, 16)==0) { //in auction table
			for (;;) {
				delete str;
				str = server.read_message();
				//printf("str = %s", str);
				if ((strncmp(str, "& BOUGHT", 8)==0)||
					(strncmp(str, "& SOLD", 6)==0)){
					t = 1;
					//printf("str auct = %s", str);
					add_new_elem(str);
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
			free_list(auct);
			delete name;
			exit(0);
		}
		if (strncmp(str, cmd, 9)==0) {
			delete str;
			break;
		}
		delete str;
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

void RobotsBrain::add_new_info_to_beg(char* nm, char* str) {
	Info_player *tmp;
	int r, p, m;
	tmp = new Info_player;
	tmp->name = nm;
	sscanf(str + 25, "%d %d %d", &r, &p, &m);
	tmp->money = m;
	tmp->raw = r;
	tmp->prod = p;
	tmp->next = players;
	players = tmp;
}

void RobotsBrain::add_new_info_to_end(char* nm, char* str) {
	Info_player* lst = players, *tmp;
	int r, p, m;
	tmp = new Info_player;
	tmp->name = nm;
	sscanf(str + 25, "%d %d %d", &r, &p, &m);
	tmp->money = m;
	tmp->raw = r;
	tmp->prod = p;
	tmp->next = 0;
	if (players == 0)
		players = tmp;
	else {
		while (lst->next != 0)
			lst = lst->next;
		lst->next = tmp;
	}
}

void RobotsBrain::print_info() {
	Info_player* tmp = players;
	while (tmp != 0) {
		printf("%10s, %7d, %5d, %5d\n", tmp->name, tmp->money,
				tmp->raw, tmp->prod);
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

void RobotsBrain::free_info_list(Info_player* lst)
{
	if (lst!=0) {
		free_info_list(lst->next);
		delete lst->name;
		delete lst;
	}
}
	
void full_argc(int argc)
{
	if (argc != 6) {
		printf("wrong paramets!\n");
		exit(1);
	}
}

int main(int argc, char **argv)
{
	Communication server;
	RobotsBrain robot;
	full_argc(argc);
	server.set(atoi(argv[2]), argv[1]);
	server.connection();
	robot.set(argv, server);
	robot.readnsend_name();
	robot.join_or_create();
	printf("start playing\n");
	robot.start_game();
	return 0;
}

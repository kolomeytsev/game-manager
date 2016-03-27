#ifndef BRAIN_H
#define BRAIN_H

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
	int plant;
	int auto_plant;
	Info_player* next;
};

struct Table_name {
	char* name;
	int number;
	Table_name* next;
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
	char *name; // name\n
	int turn;
	int active_players;
	int mode; // 0 - create, 1 - join
	char *arg;
	Info_market market;
	Info_auction* auct;
	Info_player* players;
	Communication server;
public:
	void set(char **argv, Communication serv);
	~RobotsBrain();
	void readnsend_name();
	void join_or_create();
	void join();
	void create();
	void start_easy_game(PolizItem *lst);
	void process_market();
	void process_info();
	int get_num(char *str, int *k);
	void analize_this_str(char *str);
	char *find_name(char *str);
	void buy_and_sell();
	void wait4turn();
	void produce();
	void add_new_elem(char* str);
	void print_auct_list(Info_auction *lst);
	void print_info();
	void free_list(Info_auction* lst);
	void free_auct_list();
	void free_info_list(Info_player* lst);
	void add_new_info_to_beg(char* nm, char* str);
	void add_new_info_to_end(char* nm, char* str);
	char* get_name();
	int get_turn();
	int get_active_players();
	int get_supply();
	int get_demand();
	int get_raw_price();
	int get_prod_price();
	Info_player* get_info();
	Info_auction* get_auct();
	int search_raw_sold(char* nm);
	int search_raw_price(char* nm);
	int search_prod_sold(char* nm);
	int search_prod_price(char* nm);
	void set_auct();
	int factories;
	void begin_constructing();
	void process_factories();
	int expected_raw_sold;
	int expected_prod_bought;
	int expected_raw_price;
	int expected_prod_price;
	int prev_raw_price;
	int prev_prod_price;
	void set_prev_price();
};

#endif

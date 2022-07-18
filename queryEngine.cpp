#include <list>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <string> 
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp> 
#include <time.h>

using namespace std;


struct Site{
	int ID;
	string name;
	map<string, Ttree> site_ttrees;  	//each ttree, one granularity for each site "1h":T-tree
}

struct Operator{
	enum otype{lt,lte,gt,gte,equal}
}

struct Expression{
	Field field;
	Operator op;
	string value;
}

struct Query{
	stack<string> conditions;
	vector<Field> fields;
	vector<Site> sources;
};

struct Node{
	string src_ip;
	string dst_ip;
	string src_port;
	string dst_port;
	int popularity;

}

struct Flowtree{
	map<int, Node> ft_nodes;
	int timestamp;
};

struct Ttree{
	map<int, Flowtree> tt_node;
}

struct Condition{
	Condition left;
	Condition right;
	string op; // and, or, = , >=, <=, > , <
}

Node search_ttree(Node qnode);
int field_valid(vector<string> fields);
int queried_granularity(stack<string> conditions);
Node build_query_node(stack<string> conditions);

int main(int argc, char *argv[]) {


	// sample query:
	// SELECT dstIP, dstPort, pop FROM A,B WHERE dstIP = "162.133.453.3|16" and ts > 100 and ts < 120;
	// then, the condition stack would look like:
	// and dstIP and < ts 120 > ts 100
	Query q;
	stack<string> conditions;
	conditions.push("and");
	conditions.push("=");
	conditions.push("dstPort");
	conditions.push("1000");
	conditions.push(">=");
	conditions.push("ts");
	conditions.push("1234");
	q.conditions = conditions;
	Source site1 = { .ID = 1, .name = "A"}
	Source site2 = { .ID = 2, .name = "B"}
	string field1 = "dstIP";
	string field2 = "srcIP";
	string field3 = "pop";
	vector<string> fields = {field1, field2, field3};
	if field_valid(fields)	q.fields = fields;
	q.sources = {site1, site2};

	int gran = queried_granularity(q.conditions);

	//assumption: Conditions can not contain range queries in any field other than timestamp.
	Node qnode = build_query_node(q.conditions);
	for (Source& s: q.sources){
		search_ttree(get_source_ttree(s, gran), qnode);
	}


/*
	Query q = fs_translate(argv[1]);

	for (int i = 0; i < q.fields.size(); i++)
		cout << "field i="<< i << " : " << q.fields[i] << "\n";

	for (int i = 0; i < q.sources.size(); i++)
		cout << "Source i="<< i << " : " << q.sources[i] << "\n";

	for (int i = 0; i < q.conditions.size(); i++)
		cout << "condition i="<< i << " : " << q.conditions[i] << "\n";

*/


}

Node build_query_node(stack<string> conditions){
	vector<Node> qnodes;
	string pop1, pop2, pop3;
	while (pop1 = conditions.pop() and pop2 = conditions.pop() and pop3 = conditions.pop()){
		if (!pop2.equals("ts")){ // else pop three more elements.
			if (pop3.equals("=") or ) {

				Node qnode;
				if(pop2.equals("srcIP")) qnode.srcIP = pop1;
				if(pop2.equals("dstIP")) qnode.dstIP = pop1;
				if(pop2.equals("srcPort")) qnode.srcPort = pop1;
				if(pop2.equals("dstPort")) qnode.dstPort = pop1;
				if(pop2.equals("popularity")) qnode.popularity = pop1;
				qnodes.push_back(qnode);
			}
		}
	}
}

int queried_granularity(stack<string> conditions){

	stack<string> mirror_conds = conditions;
	vector<string> conds = mirror_conds;
	string ts_min = "0";
	string ts_max = "0";
	string ts = "0";

	//copy the stack of conditions into an array inversely to simplify the search for a timestamp
	while (!mirror_conds.empty())	conds.push_back(mirror_conds.pop());


	//assumption: fieldname is always on the left. value is always on the right.
	for (int i = 1; i < conds.size(); i++){
		if (conds[i].equals("ts")){
			if (conds[i+1].equals("<") or conds[i+1].equals("<=")) ts_max = conds[i-1];
			else if (conds[i+1].equals(">") or conds[i+1].equals(">=")) ts_min = conds[i-1];
			else if (conds[i+1].equals("=")) ts = conds[i-1];
	}
	
	//assumption 1: timestamp is in terms of seconds
	//assumption 2: granularity is in terms of minutes
	//assumption 3: ts = x means from x until now

	time_t now = time(NULL);

	if (ts) 
		return (int(now) - atoi(ts.c_str())) / 60;
	else 
		return (atoi(ts_max.c_str()) - atoi(ts_min.c_str())) / 60;

}

int field_valid(vector<string> fields){
	authorized_fields = {"dstIP","srcIP", "dstPort", "srcPort", "pop", "ts"};
	for(int i=0; i<fields.size(); i++)
		if(find(authorized_fields.begin(), authorized_fields.end(), fields[i]) == authorized_fields.end())
			return 0;
	return 1;
}


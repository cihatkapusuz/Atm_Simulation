#include <pthread.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <unistd.h>
#include <deque>          
#include <list>           
#include <queue> 
using namespace std;
void *transaction(void *param); // declaration of ATM threads' runner function
void *human(void *param); // the declaration of customer threads' runner function
ofstream output; 
string outputfile;  // the filename of the output file
pthread_mutex_t wr = PTHREAD_MUTEX_INITIALIZER; // write enable/disable mutex
pthread_mutex_t atm[11]; // ATM mutexs, I created 11 but used only 10 of them, because arrays starts from 0 and I did not want to deal with +1 -1 situation
int sleep_time[301]; // sleep time of a customer, there are at most 300 customers so I guaranteed all of the customers can be stored
int atm_number[301]; // the ATM number that the customer wants to make transactions on it
string bill_type[301]; // the type of the corresponding bill
int payment_amount[301]; // amount to be paid for a bill
bool done[301]; // boolean array for marking a transaction is done or not done
int no_transactions=0; // number of transactions completed
pthread_mutex_t t_count = PTHREAD_MUTEX_INITIALIZER; // to avoid race condition, mutex lock to increment no_transactions
queue<string> billtype[11]; // ATM request queue for each ATM thread that stores type of the bill
queue<int> amount[11]; // ATM request queue for each ATM thread that stores amounts to be paid for a bill
queue<int> custoid[11]; // ATM request queue for each ATM thread that stores the customer id for that request
int electricity=0;  // total amount paid for electricity
int telecommunication=0; // total amount paid for telecommunnication
int gas=0; // total amount paid for gas
int TV=0; // total amount paid for cableTV
int water=0; // total amount paid for water
pthread_mutex_t elec = PTHREAD_MUTEX_INITIALIZER; // to avoid race condition, mutex lock to make additions safely for electricity
pthread_mutex_t tele = PTHREAD_MUTEX_INITIALIZER; // to avoid race condition, mutex lock to make additions safely for telecommunication
pthread_mutex_t gasl = PTHREAD_MUTEX_INITIALIZER; // to avoid race condition, mutex lock to make additions safely for gas
pthread_mutex_t TVl = PTHREAD_MUTEX_INITIALIZER; // to avoid race condition, mutex lock to make additions safely for cableTV
pthread_mutex_t wate = PTHREAD_MUTEX_INITIALIZER; // to avoid race condition, mutex lock to make additions safely for water
int main(int argc, char *argv[]){
	for(int i=1; i <=10; i++){
		atm[i] = PTHREAD_MUTEX_INITIALIZER; // initalize all ATM locks to 1 (enable them)
	}
	string out(argv[1]); // store argv[1] as string
	out=out.substr(0,out.find_last_of(".")); // get rid of .txt part of the input
	outputfile= out + "_log.txt"; // produce output file's name with defined rules
	output.open(outputfile); // open output file, I open file here to clear its contents before writing into it, by default it's append flag is not specified, so it clears the file
	output.close(); // close output file
	int no_customers; // number of customers that will be read from input file
	string input; // input line that will be read from the input file
	string parser,parser2,parser3; // the strings that are used for splitting input with respect to ","
	ifstream input_file; // input stream of input operations
	input_file.open(argv[1]); // opens input file for reading purposes
	getline(input_file,input);  // first line of the input file
	no_customers=stoi(input); // string to integer conversation
	int atm[11]; // an array to define atm id's
	for(int i = 1; i <= 10; i++){
		atm[i]=i; // filling with corresponding values, these will be used for ATM thread's creation
	}
	int customer[no_customers+1]; // ann array to define customer id's
	for(int i=1; i<= no_customers; i++){
		customer[i]=i; // filling with corresponding values, these will be used for customer thread's creation
		done[i]=false; // initialize all transactions to be not done	
}
	for(int i = 1; i <= no_customers; i++){
		getline(input_file,input); // gets the whole line
		
		parser=input.substr(0,input.find(",")); // gets sleep time as a string
		sleep_time[i]=stoi(parser); // string to integer conservation and then store it in sleep_time array at corresponding customer's index
		parser=input.substr(input.find(",")+1); // get rid of used part of the input line
		
		parser2=parser.substr(0,parser.find(",")); // get atm number as a string
		atm_number[i]=stoi(parser2); // string to integer conservation and then store it in atm_number array at corresponding custormer's index
		parser2=parser.substr(parser.find(",")+1); //  get rid of used part of the input line
		
		parser3=parser2.substr(0,parser2.find(",")); // get bill type as string
		bill_type[i]=parser3; // store bill_type in corresponding customer's index
		parser3=parser2.substr(parser2.find(",")+1); // get rid of used part of the input line
		
		payment_amount[i]=stoi(parser3); // string to integer conservation and then store it in payment_amout at corresponding customer's index

	}
	pthread_attr_t attr; // set of attributes for the thread
	pthread_attr_init(&attr); // initialize attributes of the thread to default
	pthread_t atmid[11]; // ATM threads' ids
	for(int i = 1; i <= 10; i++){
		pthread_create(&atmid[i], &attr , transaction , &atm[i]); // creation ATM threads
	}
	pthread_t customerid[no_customers+1]; // customer threads' ids
	
	for(int i = 1; i <= no_customers; i++){
		pthread_create(&customerid[i], &attr, human, &customer[i]); // creation of customer threads

	}
	for(int i = 1; i <= no_customers; i++){
		pthread_join(customerid[i], NULL); // when customer threads terminates notify main thread by joining all of them

	}
	while(no_transactions!=no_customers){ // busy waiting to be sure that all of the transactions are done
	}
	for(int i=1; i <=10; i++){
		pthread_cancel(atmid[i]); // cancelation of ATM threads, they are no longer needed, because all transactions are done
	}
	output.open(outputfile, ofstream::app); // open output file with append flag
	output << "All payments are completed." << endl; // write desired message
	output << "CableTV: " << TV << "TL" << endl; // write total amount paid for cableTV
	output << "Electricity: " << electricity << "TL" <<endl; // write total amount paid for electricity
	output << "Gas: " << gas << "TL" << endl; // write total amount paid for gas
	output << "Telecommunication: " << telecommunication << "TL" << endl; // write total amount paid for telecommunication
	output << "Water: " << water << "TL" << endl; // write total amount paid for water
	output.close(); // close output file
return 0;

}

void *human(void *param) { // runner function for customer threads
	int cid =  *((int *)param); // assigning the argument to an integer (customer number)
	usleep(sleep_time[cid]*1000); // customer thread sleeps with its corresponding value in milliseconds
	pthread_mutex_lock(&atm[atm_number[cid]]); // lock the customer's ATM before sending a transaction request
	billtype[atm_number[cid]].push(bill_type[cid]); // push type of the bill into corresponding ATM's billtype queue
	amount[atm_number[cid]].push(payment_amount[cid]); // push amount to be paid into corresponding ATM's amount queue
	custoid[atm_number[cid]].push(cid); // push customer id into corresponding ATM's custoid queue
	pthread_mutex_unlock(&atm[atm_number[cid]]); // release the ATM to enable it to be used from other customers
	while(done[cid]){ // customer waits till the transaction is completed
	}
	pthread_exit(0); // terminate the thread
}
void *transaction(void *param) { //runner function for ATM threads, makes transactions and writes them into output file
string transaction_type=""; // transaction type that will be taken from the billtype queue
int payment=0; // the amount to be paid that will be taken from the amount queue
int custid=0; // customer id that will be taken from the custoid queue
int aid =  *((int *)param); // assigning the argument to an integer (ATM number)
	while(1){ // an infinite loop that will be interrupted from main thread after completion of all transactions
		if(billtype[aid].size()!=0){ // checks the ATM's transaction request queue is empty or not
			transaction_type=billtype[aid].front(); // gets the head of the queue to make operation
			billtype[aid].pop(); // pops head of the billtype queue since it has been taken into account
			custid=custoid[aid].front(); // gets the corresponding customer id
			custoid[aid].pop(); // pops head of the custoid queue of that atm because the transaction will be done
			if(transaction_type=="water"){ // check transaction type before trying to lock corresponding bill type
				pthread_mutex_lock(&wate); // lock water mutex for updating total amount that paid for the water
				payment=amount[aid].front(); // gets the amount paid for that bill type
				amount[aid].pop(); // pops the head of the amount queue of that ATM since its operation is done
				water+=payment; // update total value paid for water
				pthread_mutex_unlock(&wate); // release water lock to be used from other transactions
			}
			if(transaction_type=="gas"){ // check transaction type before trying to lock corresponding bill type
				pthread_mutex_lock(&gasl); // lock gas mutex for updating total amount that paid for the gas
				payment=amount[aid].front(); // gets the amount paid for that bill type
				amount[aid].pop(); // pops the head of the amount queue of that ATM since its operation is done
				gas+=payment; // update total value paid for gas
				pthread_mutex_unlock(&gasl); // release gas lock to be used from other transactions
			}
			if(transaction_type=="telecommunication"){ // check transaction type before trying to lock corresponding bill type
				pthread_mutex_lock(&tele); // lock telecommunication mutex for updating total amount that paid for the telecommunication
				payment=amount[aid].front(); // gets the amount paid for that bill type
				amount[aid].pop(); // pops the head of the amount queue of that ATM since its operation is done
				telecommunication+=payment; // update total value paid for telecommunication
				pthread_mutex_unlock(&tele); // release telecommunication lock to be used from other transactions
			}
			if(transaction_type=="cableTV"){ // check transaction type before trying to lock corresponding bill type
				pthread_mutex_lock(&TVl);// lock cableTV mutex for updating total amount that paid for the cableTV
				payment=amount[aid].front(); // gets the amount paid for that bill type
				amount[aid].pop(); // pops the head of the amount queue of that ATM since its operation is done
				TV+=payment; // update total value paid for cableTV
				pthread_mutex_unlock(&TVl); // release cableTV lock to be used from other transactions
			}
			if(transaction_type=="electricity"){ // check transaction type before trying to lock corresponding bill type
				pthread_mutex_lock(&elec); // lock electricity mutex for updating total amount that paid for the electricty
				payment=amount[aid].front(); // gets the amount paid for that bill type
				amount[aid].pop(); // pops the head of the amount queue of that ATM since its operation is done
				electricity+=payment; // update total value paid for electricity
				pthread_mutex_unlock(&elec); // release electricity lock to be used from other transactions
			}
			pthread_mutex_lock(&wr); // lock write mutex to make write operation
			output.open(outputfile, ofstream::app); // open output file with append flag
			output << "Customer" << custid << "," << transaction_type << "," << payment << endl; // write transaction log into output file
			output.close(); // close output file
			pthread_mutex_unlock(&wr); // release write mutex to allow other threads to write
			pthread_mutex_lock(&t_count); // lock number of transactions mutex to avoid race condition
			done[custid]=true;
			no_transactions+=1; // increment number of transactions since it is completed
			pthread_mutex_unlock(&t_count); // release number of transactions mutex to allow other threas to increment too
		}
	}
	pthread_exit(0); // terminate the thread
}

//#include <blackadder.hpp>
#include <signal.h>
#include <nb_blackadder.hpp>

#include <sstream> 
#include <iostream>
#include <string>
#include <sqlite3.h>

NB_Blackadder *nb_ba;
sqlite3 *db;
int rc;
/*
    events that we need to handle:
    PUBLISHED_DATA - someone published to our scope!
    UNDEF_EVENT
    
*/
void eventHandler(Event *ev) {
    char *zErrMsg = 0;
    cout << "Received Event" << endl;
    //cout << "Type: " << (int) ev->type << endl;
    char * data = (char *) ev->data;
    if (ev->type == PUBLISHED_DATA) {
        cout << "processing command" << endl;
        char data2[ev->data_len];
        strncpy(data2, data, ev->data_len+1);
        data2[ev->data_len] = '\0';
        istringstream user_input( data2 );
        string cmd, key, val;
        getline( user_input, cmd, ',' );
        getline( user_input, key, ',' );
        getline( user_input, val);
        cout << "here's what I got: " << endl << cmd << "," << key << "," << val << endl;
        
        if (strcmp(cmd.c_str(), "put") == 0) {
            cout <<"putting value in db..." << endl;
            sqlite3_stmt *statement;
            ostringstream query;
            query << "insert into ds1 values('" << key << "', '" << val << "');";

	        if(sqlite3_prepare_v2(db, query.str().c_str(), -1, &statement, 0) == SQLITE_OK){
		    int cols = sqlite3_column_count(statement);
		    int result = 0;
		        while(true) {
			        result = sqlite3_step(statement);
			        if(result == SQLITE_ROW){
				        for(int col = 0; col < cols; col++)	{
					        string s = (char*)sqlite3_column_text(statement, col);
					        cout << "received this row back: " << s << endl;				     
					        break;
				        }
			        }
			        else{
			            
				        break;   
			        }
		        }
		        sqlite3_finalize(statement);
	        } else {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }
        } else if (strcmp(cmd.c_str(), "get") == 0) {
            cout << "getting value from db"<<endl;
            string user_scope = val;
            cout << "sending results to: " <<val << endl;
            string bin_user_scope = hex_to_chararray(user_scope);
            sqlite3_stmt *statement;
            ostringstream query;
            query << "select rid from ds1 where name='" << key << "';";
            bool results = false;

	        if(sqlite3_prepare_v2(db, query.str().c_str(), -1, &statement, 0) == SQLITE_OK){
		    int cols = sqlite3_column_count(statement);
		    int result = 0;
		        while(true) {
			        result = sqlite3_step(statement);
			        if(result == SQLITE_ROW){
			            results = true;
				        for(int col = 0; col < cols; col++)	{
					        string s = (char*)sqlite3_column_text(statement, col);
					        ostringstream user_answer;
					        user_answer << key << "=" << s;
					        string msg = user_answer.str();
                            unsigned int payload_size = 1000;
                            char * payload = (char *) malloc(payload_size);
                            strcpy(payload, msg.c_str());
                            nb_ba->publish_data(bin_user_scope, DOMAIN_LOCAL, NULL, 0u, payload, payload_size);
					        break;
				        }
			        }
			        else{
			            if (results == false) cout << "no results" << endl;
			            else cout << "done" << endl;
				        break;   
			        }
		        }
		        sqlite3_finalize(statement);
	        } else {
                fprintf(stderr, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
            }  
        }
    } 
    delete ev;
}

/*
strategy: publish the ds_scope, subscribe to it.
when that scope receives data: parse it
if we require a response, publish the corresponding user scope
*/
int main(int argc, char* argv[]) {
    string id;
    string prefix_id = "";
    string bin_id;
    string bin_prefix_id;
    string type;
    string cmd;
    if (argc > 1) {
        int user_or_kernel = atoi(argv[1]);
        if (user_or_kernel == 0) {
            nb_ba = NB_Blackadder::Instance(true);
        } else {
            nb_ba = NB_Blackadder::Instance(false);
        }
    } else {
        //By Default I assume blackadder is running in user space
        nb_ba = NB_Blackadder::Instance(true);
    }
    /*open the database*/
    rc = sqlite3_open("ds.db", &db);
     if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }
    /*Set the callback function*/
    nb_ba->setCallback(eventHandler);
    /***************************/
    cout << "Process ID: " << getpid() << endl;
    // publish the root scope @ /0000000000000000
    id = "0000000000000000";
    bin_id = hex_to_chararray(id);
    bin_prefix_id = hex_to_chararray(prefix_id);
    nb_ba->publish_scope(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);
    
    //subscribe to the ds-scope @ /00000000000000001111111111111111
    id = "1111111111111111";
    prefix_id = "0000000000000000";
    bin_id = hex_to_chararray(id);
    bin_prefix_id = hex_to_chararray(prefix_id);
    nb_ba->subscribe_scope(bin_id, bin_prefix_id, DOMAIN_LOCAL, NULL, 0);   
    
    nb_ba->join();
    nb_ba->disconnect();
    sqlite3_close(db);
    delete nb_ba;
    cout << "exiting...." << endl;
    return 0;

}

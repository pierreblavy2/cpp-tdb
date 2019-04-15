#include "tdb_sqlite.hpp"
#include <cassert>

//====================
//=== Connection_t ===
//====================

/*
auto tdb::Connection_t<tdb::Tag_sqlite>::Connection_t::operator=(Connection_t&& a)->Connection_t&{
	//doc : https://en.cppreference.com/w/cpp/thread/lock
	std::lock(a.native_mutex,this->native_mutex);
	std::lock_guard<std::mutex> lk1(a.native_mutex     , std::adopt_lock);
	std::lock_guard<std::mutex> lk2(this->native_mutex , std::adopt_lock);

	std::swap(a.native_connection, this->native_connection);
	std::swap(a.native_mutex, this->native_mutex);
	return *this;
}


tdb::Connection_t<tdb::Tag_sqlite>::Connection_t(Connection_t&& a){
	std::lock_guard<std::mutex> lk(a.native_mutex);
	std::swap(a.native_connection, this->native_connection);
	std::swap(a.native_mutex     , this->native_mutex);
}
*/



void tdb::Connection_t<tdb::Tag_sqlite>::cstr_limits(){
	//load database limits
	//https://www.sqlite.org/c3ref/limit.html
	auto read_limit=[&](const Sqlite_limit_range &r, int &i){
		auto current = sqlite3_limit(native_connection,r.sqlite_id,-1); //
		assert(current<=r.max_value);
		assert(current>=r.min_value);
		i=current;
		//assert(current==i); //this assertion fails if value is truncated
	};
	read_limit(sqlite_max_length_range             , sqlite_max_length_value);
	read_limit(sqlite_max_column_range             , sqlite_max_column_value);
	read_limit(sqlite_max_expr_depth_range         , sqlite_max_expr_depth_value);
	read_limit(sqlite_max_function_arg_range       , sqlite_max_function_arg_value);
	read_limit(sqlite_max_compound_select_range    , sqlite_max_compound_select_value);
	read_limit(sqlite_max_like_pattern_length_range, sqlite_max_like_pattern_length_value);
	read_limit(sqlite_max_variable_number_range    , sqlite_max_variable_number_value);
	read_limit(sqlite_max_trigger_depth_range      , sqlite_max_trigger_depth_value);
	read_limit(sqlite_max_attached_range           , sqlite_max_attached_value);
}





void tdb::Connection_t<tdb::Tag_sqlite>::disconnect(){
	std::lock_guard<std::mutex> lk(native_mutex);

	if(native_connection==nullptr){return;}

	auto status = sqlite3_close_v2(native_connection);
	native_connection=nullptr;

	if(status != SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Error when closing sqlite3 connection, error_code=" + std::to_string(status) );}
}


void tdb::Connection_t<tdb::Tag_sqlite>::connect(const std::string &db_name){
	std::lock_guard<std::mutex> lk(native_mutex);

	int rc = sqlite3_open(db_name.c_str() , &native_connection);

	if(rc!= SQLITE_OK){
		//doc says : Whether or not an error occurs when it is opened, resources associated with the database connection handle should be released by passing it to sqlite3_close() when it is no longer required.
		//doc : https://www.sqlite.org/c3ref/open.html
		sqlite3_close(native_connection);
		native_connection=nullptr;
		throw Exception_t<tdb::Tag_sqlite>("Cannot connect to sqlite. db_name=" + db_name + ", error_code=" + std::to_string(rc));
	}

	try{
		tdb::execute(*this,"PRAGMA foreign_keys = ON");
		cstr_limits();
	}catch(...){
		sqlite3_close(native_connection);
		native_connection=nullptr;
		throw;
	}

}





//=== native stuff ===
//human readable return values
std::string tdb::sqlite::error_to_string(int i){
	switch(i){
	case SQLITE_OK        :return "SQLITE_OK";
	case SQLITE_ERROR     :return "SQLITE_ERROR";
	case SQLITE_INTERNAL  :return "SQLITE_INTERNAL";
	case SQLITE_PERM      :return "SQLITE_PERM";
	case SQLITE_ABORT     :return "SQLITE_ABORT";
	case SQLITE_BUSY      :return "SQLITE_BUSY";
	case SQLITE_LOCKED    :return "SQLITE_LOCKED";
	case SQLITE_NOMEM     :return "SQLITE_NOMEM";
	case SQLITE_READONLY  :return "SQLITE_READONLY";
	case SQLITE_INTERRUPT :return "SQLITE_INTERRUPT";

	case SQLITE_IOERR     :return "SQLITE_IOERR";
	case SQLITE_CORRUPT   :return "SQLITE_CORRUPT";
	case SQLITE_NOTFOUND  :return "SQLITE_NOTFOUND";
	case SQLITE_FULL      :return "SQLITE_FULL";
	case SQLITE_CANTOPEN  :return "SQLITE_CANTOPEN";
	case SQLITE_PROTOCOL  :return "SQLITE_PROTOCOL";
	case SQLITE_EMPTY     :return "SQLITE_EMPTY";
	case SQLITE_SCHEMA    :return "SQLITE_SCHEMA";
	case SQLITE_TOOBIG    :return "SQLITE_TOOBIG";
	case SQLITE_CONSTRAINT:return "SQLITE_CONSTRAINT";

	case SQLITE_MISMATCH :return "SQLITE_MISMATCH";
	case SQLITE_MISUSE   :return "SQLITE_MISUSE";
	case SQLITE_NOLFS    :return "SQLITE_NOLFS";
	case SQLITE_AUTH     :return "SQLITE_AUTH";
	case SQLITE_FORMAT   :return "SQLITE_FORMAT";
	case SQLITE_RANGE    :return "SQLITE_RANGE";
	case SQLITE_NOTADB   :return "SQLITE_NOTADB";
	case SQLITE_NOTICE   :return "SQLITE_NOTICE";
	case SQLITE_WARNING  :return "SQLITE_WARNING";

	case SQLITE_ROW  :return "SQLITE_ROW";
	case SQLITE_DONE :return "SQLITE_DONE";
	}
	return "INVALID_SQLITE_CODE("+std::to_string(i)+")";
}


namespace{
	//human readable column types
	template<int I>	struct sqlite_coltype_t;
	template<>	struct sqlite_coltype_t<SQLITE_INTEGER>{static const std::string str(){return "integer";}};
	template<>	struct sqlite_coltype_t<SQLITE_FLOAT>  {static const std::string str(){return "float";}};
	template<>	struct sqlite_coltype_t<SQLITE_TEXT>   {static const std::string str(){return "text";}};
	template<>	struct sqlite_coltype_t<SQLITE_BLOB>   {static const std::string str(){return "blob";}};
	template<>	struct sqlite_coltype_t<SQLITE_NULL>   {static const std::string str(){return "null";}};
}

std::string tdb::sqlite::coltype_to_string(int i){
	switch(i){
	case SQLITE_INTEGER : return sqlite_coltype_t<SQLITE_INTEGER>::str();
	case SQLITE_FLOAT   : return sqlite_coltype_t<SQLITE_FLOAT>::str();
	case SQLITE_TEXT    : return sqlite_coltype_t<SQLITE_TEXT>::str();
	case SQLITE_BLOB    : return sqlite_coltype_t<SQLITE_BLOB>::str();
	case SQLITE_NULL    : return sqlite_coltype_t<SQLITE_NULL>::str();
	}
	throw Exception_t<Tag_sqlite>("wrong sqlite_coltype");
}






//const std::string tdb::sqlite::sql_string()     const {return ::sqlite3_sql(native_query);}
void tdb::sqlite::finalize (sqlite3_stmt* &native_query ){
	::sqlite3_finalize(native_query);
	native_query=nullptr;
}

void  tdb::sqlite::reset_binding(sqlite3_stmt* &native_query, int *native_nb_bind  ){
	  if(native_query==nullptr){return;}
	  ::sqlite3_reset(native_query);
	  (*native_nb_bind)=0;
	  sqlite3_clear_bindings(native_query);
	  ::sqlite3_reset(native_query);
}






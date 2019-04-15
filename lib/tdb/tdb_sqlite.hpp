#ifndef LIB_TDB_TDB_SQLITE_HPP_
#define LIB_TDB_TDB_SQLITE_HPP_

//sqlite driver for TDB
#include "tdb.hpp"


#include <tdb/tdb.hpp>
#include <sqlite3.h>


namespace tdb{
	struct Tag_sqlite{};
}


//===============
//=== connect ===
//===============

//user MUST specialise
template<> struct tdb::Rowid_t<tdb::Tag_sqlite>{
  typedef sqlite3_int64  type; //Line number identifiant
};


template<>
struct tdb::Connection_t<tdb::Tag_sqlite>{
	static constexpr bool is_implemented = true;

	~Connection_t() noexcept(false) {disconnect(); }            //disconnect
	void connect(const std::string &s);
	void disconnect();

	//Connection_t(Connection_t&&);
	//Connection_t& operator=(Connection_t&&);

	//not copiable, not movable
	Connection_t(Connection_t&&)=delete;
	Connection_t& operator=(Connection_t&&)=delete;

	Connection_t(const Connection_t&)           =delete;
	Connection_t& operator=(const Connection_t&)=delete;

	//(default) constructible, custom constructors
	Connection_t(){}
	explicit Connection_t(const char* db_name_)       {connect(db_name_);}
	explicit Connection_t(const std::string& db_name_){connect(db_name_);}

	//if possible construct from : db_name, db_host="", db_user="", db_pass="",port=0,extra="");
	explicit Connection_t(
			const std::string& db_name,
			const std::string& db_host,    //ignored
			const std::string& db_user="", //ignored
			const std::string& db_pass="", //ignored
			int   port = 0,                //ignored
			const std::string& extra = ""  //ignored
	){connect(db_name);}


	//mutex (required)
	std::mutex& get_mutex()const{return native_mutex;}


	//--- native ---
	mutable std::mutex native_mutex;
	sqlite3   *native_connection=nullptr; //OWNED

	//TODO
	//Generate_unique_id<size_t> savepoint_ids;


	//--- sqlite specific extra info ---
	//sqlite limits:  https://www.sqlite.org/limits.html
	struct Sqlite_limit_range{
		typedef decltype(SQLITE_LIMIT_LENGTH) id_type;
		typedef decltype(sqlite3_limit(native_connection, SQLITE_LIMIT_LENGTH, -1) ) value_type;
		id_type    sqlite_id;
		value_type default_value;
		value_type min_value;
		value_type max_value;
	};
	static constexpr Sqlite_limit_range sqlite_max_length_range             ={SQLITE_LIMIT_LENGTH     ,1000000000,1,2147483647};//Maximum length of a string or BLOB
	static constexpr Sqlite_limit_range sqlite_max_column_range             ={SQLITE_LIMIT_COLUMN     ,2000      ,1,32767}; //Maximum Number Of Columns
	static constexpr Sqlite_limit_range sqlite_max_sql_length_range         ={SQLITE_LIMIT_LENGTH     ,1000000   ,1,1073741824};  //Maximum Length Of An SQL Statement
	static constexpr Sqlite_limit_range sqlite_max_expr_depth_range         ={SQLITE_LIMIT_EXPR_DEPTH  ,1000,0,std::numeric_limits<int>::max()-1};   //Maximum Depth Of An Expression Tree
	static constexpr Sqlite_limit_range sqlite_max_function_arg_range       ={SQLITE_LIMIT_FUNCTION_ARG,100,1,127}; //Maximum Number Of Arguments On A Function
	static constexpr Sqlite_limit_range sqlite_max_compound_select_range    ={SQLITE_LIMIT_COMPOUND_SELECT,500,1,500}; //Maximum Number Of Terms In A Compound SELECT Statement
	static constexpr Sqlite_limit_range sqlite_max_like_pattern_length_range={SQLITE_LIMIT_LIKE_PATTERN_LENGTH,50000,1,50000};//Maximum Length Of A LIKE Or GLOB Pattern
	static constexpr Sqlite_limit_range sqlite_max_variable_number_range    ={SQLITE_LIMIT_VARIABLE_NUMBER,999,0,999};//Maximum Number Of Host Parameters In A Single SQL Statement
	static constexpr Sqlite_limit_range sqlite_max_trigger_depth_range      ={SQLITE_LIMIT_TRIGGER_DEPTH,1000,1,std::numeric_limits<int>::max()-1};//Maximum Depth Of Trigger Recursion
	static constexpr Sqlite_limit_range sqlite_max_attached_range           ={SQLITE_LIMIT_ATTACHED,10,1,25};//Maximum Number Of Attached Databases

	int sqlite_max_length_value              = 0;
	int sqlite_max_column_value              = 0;
	int sqlite_max_sql_length_value          = 0;
	int sqlite_max_expr_depth_value          = 0;
	int sqlite_max_function_arg_value        = 0;
	int sqlite_max_compound_select_value     = 0;
	int sqlite_max_like_pattern_length_value = 0;
	int sqlite_max_variable_number_value     = 0;
	int sqlite_max_trigger_depth_value       = 0;
	int sqlite_max_attached_value            = 0;

	private:
	void cstr_limits();

};


template<> struct tdb::Get_mutex_t<tdb::Tag_sqlite>;





//===========
//=== SQL ===
//===========

//Default implementation is OK for the following stuff.
//If you want to specialize, see the coresponding example in tdb_template.hpp
//  template<> struct tdb::Connect_t<tdb::Tag_sqlite>{...}; //Connect, default use Connection_t constructor
//  template<> struct SqlData_t<tdb::Tag_sqlite>{...}       //Store SQL, default use std::string
//  template<> struct Sql_t<tdb::Tag_sqlite>{...}           //Create SqlData_t, default use SqlData_t constructor
//  template<> struct Sql_debug_t<tdb::Tag_sqlite>{...}     //Sql_debug_t, print SQL. Default print the string



//=============
//=== Query ===
//=============

//--- Query_t (required) ---
//Return_tt : a std::tuple<...> conaining the types of the columns returned by the query
//Bind_tt   : a std::tuple<...> conaining the types bounded to the query
template<typename Return_tt, typename Bind_tt>
struct tdb::Query_t<tdb::Tag_sqlite,Return_tt,Bind_tt>{
	static constexpr bool is_implemented = true;

	//Queries are (default) constructible
	Query_t(){};
	Query_t(Connection_t<Tag_sqlite> &c, const SqlData_t<Tag_sqlite> &sql);

	//Queries are movable
	Query_t(Query_t&&q);
	Query_t& operator = (Query_t&&q);

	//Queries are not copiable
	Query_t(const Query_t&)=delete;
	Query_t& operator = (const Query_t&)=delete;

	~ Query_t(){sqlite3_finalize(native_query);}


	std::string sql_string()   const {return ::sqlite3_sql(native_query);}

	//SqlData_t<tdb::Tag_sqlite> sql; //store a copy
	sqlite3_stmt             *native_query     =nullptr; //OWNED
	sqlite3                  *native_connection=nullptr; //NOT owned
	int                       native_nb_bind   =0;//number of bound parameters

};


//Default implementation is OK for the following stuff.
//If you want to specialize, see the coresponding example in tdb_template.hpp
//  template<typename Return_tt, typename Bind_tt> struct tdb::Prepare_t<tdb::Tag_sqlite,Return_tt,Bind_tt>{...} //default calls Query_t constructor.
//  template<> struct  Prepare_t<Tag_sqlite>{...}// Prepare a query, deault fallback to Query_t constructor



//==============
//=== Result ===
//==============

template<typename Return_tt>
struct tdb::Result_t<tdb::Tag_sqlite,Return_tt>{
	static constexpr bool is_implemented=true;

	~Result_t();

	//movable
	Result_t(Result_t&&);
	Result_t& operator = (Result_t&&);

	//not copiable
	Result_t(const Result_t&)=delete;
	Result_t& operator = (const Result_t&)=delete;

	//construct from query (required for default implementation of Get_result_t)
	template<typename Bind_tt>
	explicit Result_t(Query_t<Tag_sqlite,Return_tt,Bind_tt>&q):native_query(q.native_query),native_nb_bind(&q.native_nb_bind){}

	//--- extra ---
	std::string sql_string()   const {return ::sqlite3_sql(native_query);}

	//--- native ---
	sqlite3_stmt *native_query  =nullptr; //NOT owned
	int          *native_nb_bind=nullptr; //NOT owned
	int           native_result=4; //what sqlite3_step returns. TODO
};


template<typename Return_tt> struct tdb::Try_fetch_t<tdb::Tag_sqlite,Return_tt>;






//==================
//=== Bind_one_t ===
//==================
// Bind_t default uses Bind_one (see tdb_sqlite.tpp)
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,double       ,I>; //double -> sqlite3_bind_double
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,int          ,I>; //int           -> sqlite3_bind_int
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,sqlite3_int64,I>; //sqlite3_int64 -> sqlite3_bind_int64
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,size_t       ,I>; //size_t        -> sqlite3_int64 => MAY OVERFLOW (throw)
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,std::string  ,I>; //std::string   -> sqlite3_bind_text
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,bool         ,I>; //bool          -> sqlite3_bind_int
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,char         ,I>; //char          -> sqlite3_bind_text
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,const char*  ,I>; //const char*   -> sqlite3_bind_text
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,tdb::Null    ,I>; //tdb::Null     -> sqlite3_bind_null


//=================
//=== Get_one_t ===
//=================
//idem
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,double,        I>; //double        <- sqlite3_column_double;
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,int,           I>; //int           <- sqlite3_column_int
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,sqlite3_int64, I>; //sqlite3_int64 <- sqlite3_column_int64
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,size_t,        I>; //size_t        <- sqlite3_column_int64
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,std::string,   I>; //std::string   <- sqlite3_column_text
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,bool,          I>; //bool          <- sqlite3_column_int  (expects 0 or 1, throw if anything else)
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,char,          I>; //char          <- sqlite3_column_text (expect a single char string, throw if anything else)

template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,tdb::Null,     I>; //Null          <- do nothing

//std::optional
template<size_t I, typename T > struct tdb::Get_one_t<tdb::Tag_sqlite,std::optional<T>, I>; //std::optional<T> <- dispatch, treat Null as empty optional







//===================================
//=== execute, insert, get_result ===
//===================================

//Execute_t,Insert_t,Get_result_t   (required)
template<typename Return_tt, typename Bind_tt> struct tdb::Execute_t   <tdb::Tag_sqlite,Return_tt,Bind_tt >;
template<typename Return_tt, typename Bind_tt> struct tdb::Insert_t    <tdb::Tag_sqlite,Return_tt,Bind_tt>;

//Default implementation is OK for the following stuff.
//  template<typename Return_tt, typename Bind_tt> struct tdb::Get_result_t<tdb::Tag_sqlite,Return_tt,Bind_tt>; //default use Result_t::Result_t(Query_t&)






//--- sqlite3 specific stuff ---

namespace tdb::sqlite{
	std::string coltype_to_string(int i);
	std::string error_to_string(int i);


	void finalize     (sqlite3_stmt* &native_query );
	void reset_binding(sqlite3_stmt* &native_query, int* native_nb_bind  );


	struct Query_guard{

		explicit Query_guard(sqlite3_stmt* &native_query_,int* native_nb_bind_ ):
				native_query(native_query_),
				native_nb_bind(native_nb_bind_)
		{}

		template<typename Return_tt, typename Bind_tt>
		explicit Query_guard( Query_t<tdb::Tag_sqlite,Return_tt, Bind_tt> &r):
				native_query  ( r.native_query),
				native_nb_bind(&r.native_nb_bind)
		{}

		~Query_guard(){reset_binding(native_query, native_nb_bind);}

		private:
		sqlite3_stmt* & native_query;
		int* native_nb_bind=nullptr;
	};

}

#include "tdb_sqlite.tpp"
#endif /* LIB_TDB_TDB_SQLITE_HPP_ */

#ifndef LIB_TDB_TDB_PSQL_HPP_
#define LIB_TDB_TDB_PSQL_HPP_
#include "tdb.hpp"

#include <libpq-fe.h>
#include <cstdint> //for OID
#include <ctime>   //for OID

#include <convert/convert.hpp>


//doc : https://www.postgresql.org/docs/9.1/libpq-example.html

namespace tdb{
	struct Tag_psql{};
}

//===============
//=== connect ===
//===============

//required
//The type used to identify rows
template<> struct tdb::Rowid_t<tdb::Tag_psql>{
  typedef long long int type;
  static_assert(std::numeric_limits<type>::max() >= 9223372036854775807,"Rowid_t<tdb::Tag_psql>::type will overflow" );
  //doc : https://www.postgresql.org/docs/9.1/datatype-numeric.html
  //BIGSERIAL is from 1 to 9223372036854775807
  //we expect that rows numbers are smaller than this value
  //this is also the biggest int that postgresql can handle
};



//required : Store connection info
template<>
struct tdb::Connection_t<tdb::Tag_psql>{
	static constexpr bool is_implemented = true;

	//NOT copiable, NOT movable
	Connection_t(Connection_t&&)                =delete;
	Connection_t& operator=(Connection_t&&)     =delete;
	Connection_t(const Connection_t&)           =delete;
	Connection_t& operator=(const Connection_t&)=delete;


	Connection_t(){}
	explicit Connection_t(const std::string &connect_str){connect(connect_str);}

	//Recomended : cannonical constructor
	explicit Connection_t(
			const std::string& db_name,
			const std::string& db_host,
			const std::string& db_user="",
			const std::string& db_pass="",
			int   port = 0, //0 fallback to psql default : 5432
			const std::string& extra = ""
	){connect(db_name,db_host,db_user,db_pass,port,extra);}


	~Connection_t(){disconnect();}

	void connect(const std::string &s);
	void connect(
			const std::string& db_name,
			const std::string& db_host,
			const std::string& db_user="",
			const std::string& db_pass="",
			int   port = 0,
			const std::string& extra = ""
	);
	void disconnect();

	PGconn *   native_connection=nullptr;
	mutable std::mutex native_connection_mutex;
};


//Required: return the connection mutex.
template<> struct tdb::Get_mutex_t<tdb::Tag_psql>{
	static constexpr bool is_implemented = true;
	static auto & run(const tdb::Connection_t<tdb::Tag_psql> &c){
		return c.native_connection_mutex;
	}
};


//===========
//=== SQL ===
//===========

//The default implementation stores SQL as std::strings.
//If you want to specialize, see the coresponding example in tdb_template.hpp
//  template<> struct tdb::Connect_t<tdb::Tag_xxx>{...}; //Connect, default use Connection_t constructor
//  template<> struct SqlData_t<tdb::Tag_xxx>{...}       //Store SQL, default use std::string
//  template<> struct Sql_t<tdb::Tag_xxx>{...}           //Create SqlData_t, default use SqlData_t constructor
//  template<> struct Sql_debug_t<tdb::Tag_xxx>{...}     //Sql_debug_t, print SQL. Default print the string




//=============
//=== Query ===
//=============

//--- Query_t (required) ---
//Return_tt : a std::tuple<...> conaining the types of the columns returned by the query
//Bind_tt   : a std::tuple<...> conaining the types bounded to the query   (no references, no cv)
//Bind_t2   : a std::tuple<...> conaining the types the user wants to bind (may contain references, or const)


namespace tdb::psql{
	//generate a unique name for the query
	std::string  query_name(const void * unique_id);

	std::string  result_error(const PGresult *res)noexcept(true);

	template<typename Bind_tt>
	std::array<Oid,std::tuple_size<Bind_tt>::value> constexpr  oid();

	template<typename Bind_tt>
	std::array<int,std::tuple_size<Bind_tt>::value> constexpr format();

	template<typename Bind_tt,typename Return_tt, typename Bind_t2 >
	void to_db  (
			std::array<std::string, std::tuple_size<Bind_tt>::value> &write_here,
			const Bind_t2 & bind_me
	);

	template<typename Return_tt>
	void from_db(
			Return_tt & write_here,
			Result_t<tdb::Tag_psql, Return_tt>& result
	);

}


template<typename Return_tt, typename Bind_tt>
struct tdb::Query_t<tdb::Tag_psql,Return_tt,Bind_tt>{
	static constexpr bool is_implemented = true;

	//required : default constructor + move
	Query_t()noexcept(true);
	Query_t(Query_t&&q)noexcept(true);
	Query_t& operator = (Query_t&&q)noexcept(true);

	//Queries are NOT copiable
	Query_t(const Query_t&)             =delete;
	Query_t& operator = (const Query_t&)=delete;

	//Required : clean up stuff
	//LOCKS the underlying database
	~Query_t()noexcept(true);

	//Recommended : construct from connection + sql
	Query_t(Connection_t<Tag_psql> &c, const SqlData_t<Tag_psql> &sql);

	//Recomended : print the sql used to run the Query_t
	std::string sql_string()const;


	//We create unique names by using the address of this char*
	//This char* must leave the same way native_query_result does
	//(so it must be moved with it)
	char* native_name_holder = nullptr;
	std::string native_name;

	std::string native_sql;

	PGresult * native_query_result  = nullptr; //OWNED

	//NOT owned, required for multi thread (as ~Query_t() must lock connection mutex)
	tdb::Connection_t<tdb::Tag_psql> *db=nullptr;

	//PGconn *   native_connection    = nullptr; //NOT owned


	std::array<std::string, std::tuple_size<Bind_tt>::value> paramValues;//serialize parameters here
	std::array<int, std::tuple_size<Bind_tt>::value> paramLengths;

	static constexpr std::array<Oid, std::tuple_size<Bind_tt>::value> native_oid    = psql::oid<Bind_tt>();
	static constexpr std::array<int, std::tuple_size<Bind_tt>::value> paramFormats  = psql::format<Bind_tt>();



};






//Default implementation is OK for the following stuff.
//If you want to specialize, see the coresponding example in tdb_template.hpp
//  template<typename Return_tt, typename Bind_tt> struct tdb::Prepare_t<tdb::Tag_xxx,Return_tt,Bind_tt>{...} //default calls Query_t constructor.
//  template<> struct  Prepare_t<Tag_xxx>{...}// Prepare a query, deault fallback to Query_t constructor



//==============
//=== Result ===
//==============

template<typename Return_tt>
struct tdb::Result_t<tdb::Tag_psql,Return_tt>{
	static constexpr bool is_implemented=true;

	//Required : movable
	Result_t(Result_t&&)             noexcept(true);
	Result_t& operator = (Result_t&&)noexcept(true);

	//not copiable
	Result_t(const Result_t&)             =delete;
	Result_t& operator = (const Result_t&)=delete;

	//Required : clean up
	~Result_t();

	//Required* : construct from query
	//*This is required to make Get_result_t default implementation works
	// If you cannot write such constructor, you can still specialise Get_result_t
	template<typename Bind_tt>
	explicit Result_t(Query_t<Tag_psql,Return_tt,Bind_tt>&q);

	//Recomended : return the sql as std::string
	std::string sql_string()const;

	//native
	PGresult * native_result = nullptr; //OWNED
	int current_row = 0;

};

//Required : Fetch data.
//No data   : return an empty std::optional<Return_tt>
//some data : return the current row in a std::optional<Return_tt>
//            AND iterate to the next line.
// Usage:
//    auto r = try_fetch(result);
//    while(r.has_value() ){
//        const auto row = tdb::get_row(result);
//        std::cout << std::get<0>(row)<<"\n";
//        r = try_fetch(result);
//    }

//tdb_psql.tpp implements
//  template<typename Return_tt>  struct tdb::Try_fetch_t<tdb::Tag_psql,Return_tt>;
//  template<typename Return_tt>  struct tdb::Count_row_t<tdb::Tag_psql,Return_tt>;



//============================
//=== Bind_t and Get_row_t ===
//============================
namespace tdb::psql{
	static constexpr inline int FORMAT_TEXT   = 0;
	static constexpr inline int FORMAT_BINARY = 1;

	template<typename T, typename is_enabled=void> struct BindInfo_t;
}

//Bind_t DO NOT USE Bind_one, but relies on tdb::psql::Bind_info
template <typename Return_tt, typename Bind_tt>
struct tdb::Bind_t<tdb::Tag_psql,Return_tt,Bind_tt>;

//Get_row_t DO NOT USE Get_one, but relies on tdb::psql::Bind_info
template <typename Return_tt>
struct tdb::Get_row_t<tdb::Tag_psql,Return_tt>;





//=======================
//=== execute, insert ===
//=======================
// tdb_psql.tpp implements
//   template<typename Return_tt, typename Bind_tt > struct tdb::Execute_t <tdb::Tag_psql,Return_tt, Bind_tt >;
//   template<typename Return_tt, typename Bind_tt>  struct tdb::Insert_t  <tdb::Tag_psql,Return_tt,Bind_tt>;





#include "tdb_psql.tpp"
#endif /* LIB_TDB_TDB_PSQL_HPP_ */

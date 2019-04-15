#include "tdb_psql.hpp"
//#include <catalog/pg_type.h> //-I/usr/include/pgsql/server/
#include <limits.h>  //CHAR_BIT


//===============
//=== connect ===
//===============

void tdb::Connection_t<tdb::Tag_psql>::disconnect(){
	if(native_connection!=nullptr){
		PQfinish(native_connection);
		native_connection = nullptr;
	}
}


namespace{
//Escape conninfo parameters values
std::string psql_param(const std::string &s){
	//doc : https://www.postgresql.org/docs/8.1/libpq.html
	//The passed string can be empty to use all default parameters, or it can
	//contain one or more parameter settings separated by whitespace.
	//Each parameter setting is in the form keyword = value. Spaces around the
	//equal sign are optional. To write an empty value or a value containing
	//spaces, surround it with single quotes, e.g., keyword = 'a value'.
	//Single quotes and backslashes within the value must be escaped
	//with a backslash, i.e., \' and \\.

	std::string r;
	for(const char &c : s){
		if(c=='\''){r+="\\'";  continue;}
		if(c=='\\'){r+="\\\\"; continue;}
		r+=c;
	}
	return r;
}
}

void tdb::Connection_t<tdb::Tag_psql>::connect(const std::string &conninfo){
	if(native_connection!=nullptr){
		PQfinish(native_connection);
	}
	native_connection=PQconnectdb(conninfo.c_str());

	if (PQstatus(native_connection) != CONNECTION_OK){
		std::string err = PQerrorMessage(native_connection);
		PQfinish(native_connection);
		native_connection=nullptr;
		throw tdb::Exception_t<tdb::Tag_psql>("Cannot connect to psql database, connexion_string = "+conninfo+", error="+err);
	}
}


void tdb::Connection_t<tdb::Tag_psql>::connect(
	const std::string& db_name,
	const std::string& db_host,
	const std::string& db_user,
	const std::string& db_pass,
	int   port,
	const std::string& extra
){
	std::string conninfo ="dbname = "   + psql_param(db_name);

	if(db_host!=""){conninfo+=" hostaddr = " + psql_param(db_host);}
	if(db_user!=""){conninfo+=" user = " + psql_param(db_user);}
	if(db_pass!=""){conninfo+=" password = " + psql_param(db_pass);}
	if(port!=0)    {conninfo+=" port = '" + std::to_string(port)+"'";}

	conninfo+=extra;

	connect(conninfo);
}


//=============
//=== Query_t ===
//=============





//=============
//=== Oid_t ===
//=============
//Oid are in /usr/include/pgsql/server/catalog/pg_type.h
//  cat /usr/include/pgsql/server/catalog/pg_type.h | grep OID | grep define
//doc : https://www.postgresql.org/message-id/AANLkTimiNjQa7ws1tyR_W6RQPec6RlxQtWfACNMnZ_1P%40mail.gmail.com

//#include <pgsql/server/catalog/pg_type.h>

/*
//force dependant types so enable_if works
#define MK_OID_T(X_type,X_value)\
		template<typename T>\
		struct tdb::psql::Oid_t<T, typename std::enable_if<std::is_same<X_type,T>::value,int >::type  >{\
			static constexpr bool is_implemented=true;\
			Oid_t()=delete;\
			static constexpr Oid value = X_value;\
			typedef X_type  mapped_type;\
		};


#define MK_OID_ARRAY(X_type,X_value)\
	template<size_t N> struct tdb::psql::Oid_t<X_type[N]>{\
		static constexpr bool is_implemented=true;\
		Oid_t()=delete;\
		static constexpr Oid value = X_value;\
		typedef X_type mapped_type[N];\
	};\
	\
	template<size_t N> struct tdb::psql::Oid_t< std::array<X_type,N> >{\
		static constexpr bool is_implemented=true;\
		Oid_t()=delete;\
		static constexpr Oid value = X_value;\
		typedef std::array<X_type,N> mapped_type;\
	};


//boolean, 'true'/'false'
MK_OID_T    (bool, BOOLOID);




//--- integers ---
//doc : https://en.cppreference.com/w/cpp/types/integer

//NOTE : the C++ implementation is somewho tricky, even for fixed size integer
//depending on the implementation and typedef, differents size integer MAY
//for stupid reasons map to the same type. When this happen it create
// "already defined" template errors
//MK_OID_T(std::int16_t, INT2OID);//2 bytes, 16 bits
//MK_OID_T(std::int32_t, INT4OID);//4 bytes, 32 bits
//MK_OID_T(std::int64_t, INT8OID);//8 bytes, 64 bits


//single character
MK_OID_T    (char, CHAROID);


//--- ints ---
namespace{
	template<size_t Byte_count> struct Oid_int_dispatch{};
	template<> struct Oid_int_dispatch<16>{static constexpr int value_signed = INT2OID; static constexpr int value_unsigned = INT4OID;};
	template<> struct Oid_int_dispatch<32>{static constexpr int value_signed = INT4OID; static constexpr int value_unsigned = INT8OID;};
	template<> struct Oid_int_dispatch<64>{static constexpr int value_signed = INT8OID; };
}

template<typename T>
struct tdb::psql::BindInfo_t<
    T,
	typename std::enable_if<
	      ! std::is_same<T,char>::value
		and std::is_integral<T>::value
		and std::is_signed<T>::value
    ,int >::type
>{
	static constexpr bool is_implemented=true;
	BindInfo_t()=delete;
	static constexpr size_t byte_count = sizeof(T)*CHAR_BIT;
	static constexpr Oid value = Oid_int_dispatch<byte_count>::value_signed;
	typedef std::time_t  mapped_type;
};

template<typename T>
struct tdb::psql::BindInfo_t<
    T,
	typename std::enable_if<
	        ! std::is_same<T,char>::value
		and   std::is_integral<T>::value
		and ! std::is_signed<T>::value
    ,int >::type
>{
	static constexpr bool is_implemented=true;
	BindInfo_t()=delete;
	static constexpr size_t byte_count = sizeof(T)*CHAR_BIT;
	static constexpr Oid value = Oid_int_dispatch<byte_count>::value_unsigned;
	typedef std::time_t  mapped_type;
};


//--- float, double ---
MK_OID_T(float, FLOAT4OID);//single-precision floating point number, 4-byte storage
MK_OID_T(double,FLOAT8OID);//double-precision floating point number, 8-byte storage
//MK_OID_T(???,NUMERICOID);//numeric(precision, decimal), arbitrary precision number

//--- string, text ---
MK_OID_T(std::string, TEXTOID);  //variable-length string, no limit specified
//MK_OID_ARRAY(char   , TEXTOID);



//--- time ---
//note : various OID exists for date, hour, date_time+timezone ...

//relative, limited-range time interval (Unix delta time)
//note for absolute, use : ABSTIMEOID
//depending on the implementation std::time_t MAY typedef it as
//std::int32_t or std::int64_t. Which creates duplicated template definitions.
*/

/*
//MK_OID_T(std::time_t,RELTIMEOID);
template<typename T>
struct tdb::psql::Oid_t<
    T,
	typename std::enable_if<
	    std::is_same<std::time_t,T>::value
		and ! std::is_same<std::time_t, std::int16_t>::value
		and ! std::is_same<std::time_t, std::int32_t>::value
		and ! std::is_same<std::time_t, std::int64_t>::value
    ,int >::type
>{
	static constexpr bool is_implemented=true;
	Oid_t()=delete;
	static constexpr int value = RELTIMEOID;
	typedef std::time_t  mapped_type;
};
*/







//relative, limited-range time interval (Unix delta time)
//note for absolute, use : ABSTIMEOID
//(abstime,abstime), time interval
/*
namespace{
	typedef decltype( std::declval<std::time_t>() - std::declval<std::time_t>() ) duration_t;
}
MK_OID_T( duration_t ,TINTERVALOID);
*/

//--- blob ---
//VARBITOID















//unsure



std::string  tdb::psql::result_error(const PGresult *res)noexcept(true){
	//doc : https://www.postgresql.org/docs/9.1/libpq-exec.html

	if(res==nullptr){
		return "PGresult* is null\n";
	}

	auto err_str=[](const std::string & title, const PGresult *res,const int field)->std::string{
		const char * msg = PQresultErrorField(res, field);
		if(msg==nullptr){return "";}

		return "\n  "+title+": " + msg ;
	};

	auto errorcode_to_string=[](ExecStatusType i)->std::string{
		switch(i){
		case ExecStatusType::PGRES_EMPTY_QUERY:   {return "PGRES_EMPTY_QUERY : empty query string was executed";}
		case ExecStatusType::PGRES_COMMAND_OK:    {return "PGRES_EMPTY_QUERY : a query command that doesn't return anything was executed properly by the backend";}
		case ExecStatusType::PGRES_TUPLES_OK:     {return "PGRES_TUPLES_OK a query command that returns tuples was executed properly by the backend, PGresult contains the result tuples";}
		case ExecStatusType::PGRES_COPY_OUT:      {return "PGRES_COPY_OUT : Copy Out data transfer in progress";}
		case ExecStatusType::PGRES_COPY_IN:       {return "PGRES_COPY_IN : Copy In data transfer in progress";}
		case ExecStatusType::PGRES_BAD_RESPONSE:  {return "PGRES_BAD_RESPONSE : an unexpected response was recv'd from the backend";}
		case ExecStatusType::PGRES_NONFATAL_ERROR:{return "PGRES_NONFATAL_ERROR : notice or warning message";}
		case ExecStatusType::PGRES_FATAL_ERROR:   {return "PGRES_FATAL_ERROR : query failed";}
		case ExecStatusType::PGRES_COPY_BOTH:     {return "PGRES_COPY_BOTH : Copy In/Out data transfer in progress";}
		case ExecStatusType::PGRES_SINGLE_TUPLE:  {return "PGRES_SINGLE_TUPLE :single tuple from larger resultset ";}
		}
		return "Unknown ExecStatusType";
	};

	std::string r;
	    r+= "\n  error_code: " + errorcode_to_string(PQresultStatus(res));
	    r+= err_str("severity",res, PG_DIAG_SEVERITY);
		r+= err_str("state",res, PG_DIAG_SQLSTATE);
		r+= err_str("msg",res, PG_DIAG_MESSAGE_PRIMARY);
		r+= err_str("details",res, PG_DIAG_MESSAGE_DETAIL);
		r+= err_str("hint",res, PG_DIAG_MESSAGE_HINT);
	return r;
}





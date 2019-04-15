#include <sstream>
#include <iostream>
#include <cassert>





//===============
//=== Convert ===
//===============

#include <boost/lexical_cast.hpp>
//string => number
template<typename To_tt>
struct Convert_t<
		To_tt,
		std::string,
		typename std::enable_if <std::is_integral<To_tt>::value or std::is_floating_point<To_tt>::value, tdb::Tag_psql
>::type  >{
	typedef To_tt To_t;
	typedef std::string From_t;
	typedef tdb::Tag_psql Context_tag_t;

	static To_t run(const From_t &f){
		return boost::lexical_cast<To_t>(f);
	}
};


//number=> string
template<typename From_tt>
struct Convert_t<
		std::string, //to
		From_tt,
		typename std::enable_if <std::is_integral<From_tt>::value or std::is_floating_point<From_tt>::value, tdb::Tag_psql
>::type  >{
	typedef std::string To_t;
	typedef From_tt     From_t;
	typedef tdb::Tag_psql Context_tag_t;

	static To_t run(const From_t &f){
		return boost::lexical_cast<To_t>(f);
	}
};


//===============
//=== Query_t ===
//===============
//doc :
//  - SQL prepare   : https://www.postgresql.org/docs/9.3/sql-prepare.html
//  - libpq prepare : https://www.postgresql.org/docs/9.1/libpq-exec.html



template<typename Return_tt, typename Bind_tt>
tdb::Query_t<tdb::Tag_psql,Return_tt,Bind_tt>::Query_t(Query_t&&q)noexcept(true){
	std::swap(native_name,q.native_name);
	std::swap(native_name_holder,q.native_name_holder);
	std::swap(native_query_result,q.native_query_result);
	std::swap(db,q.db);
	std::swap(paramValues,q.paramValues);
	std::swap(native_sql,q.native_sql);
}

template<typename Return_tt, typename Bind_tt>
auto tdb::Query_t<tdb::Tag_psql,Return_tt,Bind_tt>::operator = (Query_t&&q)noexcept(true) -> Query_t&{
	std::swap(native_name,q.native_name);
	std::swap(native_name_holder,q.native_name_holder);
	std::swap(native_query_result,q.native_query_result);
	std::swap(db,q.db);
	std::swap(paramValues,q.paramValues);
	std::swap(native_sql,q.native_sql);
	return *this;
}


template<typename Return_tt, typename Bind_tt>
tdb::Query_t<tdb::Tag_psql,Return_tt,Bind_tt>::Query_t()noexcept(true){}


template<typename Return_tt, typename Bind_tt>
tdb::Query_t<tdb::Tag_psql,Return_tt,Bind_tt>::~Query_t()noexcept(true){
	if(native_query_result==nullptr){
		assert(native_name_holder==nullptr);
	}

	if(native_query_result!=nullptr){

		assert(native_name_holder!=nullptr);

		//std::cout << "delete : "<<this->native_name<<std::endl;


		auto l = tdb::impl::connection_lock_guard<tdb::Tag_psql> (*db);

		std::string cleanup_sql= std::string("DEALLOCATE ") +  native_name;

		auto r = PQexec( db->native_connection ,  cleanup_sql.c_str() );
		assert(r!=nullptr);
		auto status = PQresultStatus(r);
		if(status!=PGRES_COMMAND_OK){
			std::cerr << "CANNOT clean psql prepared statement."
			  <<"\n  sql       ="<<cleanup_sql
			  <<"\n  name      ="<<native_name
			  <<"\n  error_code="<<std::to_string(status)
			  <<"\n  msg       ="<<tdb::psql::result_error(r)
			  <<std::endl;
		}

		PQclear(r);
		PQclear(native_query_result);
		delete native_name_holder;

	}
}


template<typename Return_tt, typename Bind_tt>
tdb::Query_t<tdb::Tag_psql,Return_tt,Bind_tt>::Query_t(
		Connection_t<Tag_psql> &c,
		const SqlData_t<Tag_psql> &sql
){
	//https://www.postgresql.org/docs/9.3/libpq-exec.html
	assert(native_query_result == nullptr);
	assert( db                 == nullptr);
	assert( native_name_holder == nullptr);

	static constexpr size_t param_size = std::tuple_size<Bind_tt>::value;

	native_name_holder = new char;
	this->native_name = psql::query_name(native_name_holder);
	//std::cout << "create : "<<this->native_name<<std::endl;

	this->native_sql  = sql.to_string();
	native_query_result = PQprepare(
			c.native_connection,
			native_name.c_str(),
			sql.c_str(),
			param_size,
			this->native_oid.data()
    );


	if(PQresultStatus(native_query_result) != PGRES_COMMAND_OK){
		std::string err = PQerrorMessage(c.native_connection) + std::string("\n") + psql::result_error(native_query_result);

		PQclear(native_query_result);
		native_query_result=nullptr;

		throw Exception_t<tdb::Tag_psql>("Cannot prepare query\n error:\n"+err+ "\nsql\n" + sql.c_str() +"\n");
	}

	this->db=&c; 	//this->native_connection = c.native_connection;

}


template<typename Return_tt, typename Bind_tt>
std::string tdb::Query_t<tdb::Tag_psql,Return_tt,Bind_tt>::sql_string()const{
	return this->native_sql;
}


inline std::string tdb::psql::query_name(const void * unique_id){
	//const void * unique_id = std::addressof(q);
	std::stringstream ss;
	ss <<"q_" <<  unique_id;
	return ss.str();
}





//================
//=== Result_t ===
//================



template<typename Return_tt>
tdb::Result_t<tdb::Tag_psql,Return_tt>::Result_t(Result_t&& r)noexcept(true){
	std::swap(native_result,r.native_result);
	std::swap(current_row,  r.current_row);
}

template<typename Return_tt>
auto tdb::Result_t<tdb::Tag_psql,Return_tt>::operator = (Result_t&& r)noexcept(true)->Result_t&{
	std::swap(native_result,r.native_result);
	std::swap(current_row  , r.current_row);
	return *this;
}

template<typename Return_tt>
tdb::Result_t<tdb::Tag_psql,Return_tt>::~Result_t(){
	PQclear(native_result);
}

template<typename Return_tt>
template<typename Bind_tt>
tdb::Result_t<tdb::Tag_psql,Return_tt>::Result_t(Query_t<Tag_psql,Return_tt,Bind_tt>&q){
	assert(native_result==nullptr);

	//TODO bind must fill : paramValues, paramLengths, paramFormats


	static constexpr int  resultFormat= 0; //0=string. 1=binary.
	//WARNING : doc in unclear, no idea which format is expected for binary
	//(endianes, size ???)

	//std::string to c_str
	std::array<const char *, std::tuple_size<Bind_tt>::value > paramValues_cstr;
	for(size_t i = 0; i < std::tuple_size<Bind_tt>::value; ++i){
		paramValues_cstr[i]=q.paramValues[i].c_str();
	}


	this->native_result = PQexecPrepared(
			q.db->native_connection,
			q.native_name.c_str() ,
			std::tuple_size<Bind_tt>::value,
			paramValues_cstr.data(),
			q.paramLengths.data(),
			q.paramFormats.data(),
			resultFormat
	);


}






//============================
//=== Bind_t and Get_row_t ===
//============================


//--- BindInfo_t ---
namespace tdb::psql{

	namespace impl{
	template<typename T> struct is_optional_t{static constexpr bool value = false;};
	template<typename T> struct is_optional_t<std::optional<T>> {static constexpr bool value = true;};

	template<typename T>
	static constexpr bool is_optional = is_optional_t<T>::value;
	}


	template<typename T, typename is_enabled>
	struct BindInfo_t{

		static constexpr Oid oid    = 0; 	//let the server decide

		//doc for binary format is unclear, so it's safer to pass text
		static constexpr int format = FORMAT_TEXT;

		static_assert(format==FORMAT_TEXT,"libpq hates you : don't use binary interface.");
		//WARNING :
		//libpq propose a text and a binary interface to bind/fetch data but
		//-  for binding :  text v.s. binary is chosen for each bound parameter
		//   you can therefore send param 1 as text, and param 2 as binary.
		//   this is the expected behaviour as it make sence to choose depending
		//   on the parameter type.
		//-  for fetching : all column must be fetched using the same interface
		//   (all are text, or all are binary, no mix). This is terribly stupid
		//   as exchange format is related to parameters types, not to something
		//   query specific. Morever it's unconsistend with binding.
		//=> It's therefore impossible to define how parameters are exchanged
		//   in a consistent way for each parameter.
		//
		//=> !!! ALWAYS bind and fetch as FORMAT_TEXT. !!!
		//
		//=> Note that the doc is trolling you, saying that psql CAN define
		//   a format per fetched column but actually DON'T do it
		//   (see resultFormat in PQexecParams at
		//   https://www.postgresql.org/docs/9.1/libpq-exec.html )

		//WARNING2:
		//The binary interface require architecture independant values + OID,
		//and the way to generate them from native types is poorly documented.

		//convert to text/binary format
		//default : text
		static std::string to_db(const T&t){
			//std::cout << "binding"<<convert<std::string,tdb::Tag_psql>(t)<<std::endl;
			return convert<std::string,tdb::Tag_psql>(t);
		}

		//convert from text/binary format
		//default : text
		static T from_db(const std::optional<std::string> &s){
			if constexpr (impl::is_optional<T>){
				if(s.has_value()){return convert<typename T::value_type,tdb::Tag_psql>(s.value() ); }
				else{return T();}
			}else{
				if(s.has_value()){return convert<T,tdb::Tag_psql>(s.value() );} //Error here : missing Convert_t
				else             {throw Exception_t<tdb::Tag_psql>("NULL value");}
			}
		}
	};


	namespace impl{

		template<size_t I> struct BindInfo_r{

			template<typename Bind_tt>
			static constexpr void run_oid(std::array<Oid,std::tuple_size<Bind_tt>::value>& write_here){
				BindInfo_r<I-1>::template run_oid<Bind_tt>(write_here);
				typedef typename std::tuple_element<I-1,Bind_tt>::type el_t;
				write_here[I-1]=BindInfo_t<el_t>::oid;
			}

			template<typename Bind_tt>
			static constexpr void run_format(std::array<int, std::tuple_size<Bind_tt>::value>& write_here){
				BindInfo_r<I-1>::template run_format<Bind_tt>(write_here);
				typedef typename std::tuple_element<I-1,Bind_tt>::type el_t;
				write_here[I-1]=BindInfo_t<el_t>::format;
			}

			template<typename Bind_tt, typename Bind_t2 >
			static void run_to_db(
					std::array<std::string, std::tuple_size<Bind_tt>::value> &write_here,
					std::array<int, std::tuple_size<Bind_tt>::value> &length,
					const Bind_t2 & bind_me
			){
				BindInfo_r<I-1>::template run_to_db<Bind_tt,Bind_t2>(write_here,length,bind_me);
				typedef typename std::tuple_element<I-1,Bind_tt>::type el_t;
				write_here[I-1] = BindInfo_t<el_t>::to_db( std::get<I-1>(bind_me) ) ;
				length[I-1]     = write_here[I-1].length();
			}

			template<typename Return_tt>
			static void run_from_db(
				Return_tt & write_here,
				const PGresult *res,
				int row
			){
				BindInfo_r<I-1>::template run_from_db<Return_tt>(write_here,res,row);

				typedef typename std::tuple_element<I-1,Return_tt>::type el_t;

				//https://www.postgresql.org/docs/9.3/libpq-exec.html


				std::optional<std::string> s;
				if(!PQgetisnull(res,row,I-1)){
					//not null => get data
					const size_t data_size = PQgetlength(res,row,I-1);
					s = std::string();
					s.value().assign(PQgetvalue(res,row,I-1), data_size);
				}

				//assign (even if s is null)
				std::get<I-1>(write_here)=BindInfo_t<el_t>::from_db( s ) ;
			}
		};


		template<> struct BindInfo_r<0>{

			template<typename Bind_tt>
			static constexpr void run_oid(std::array<Oid,std::tuple_size<Bind_tt>::value>& write_here){}

			template<typename Bind_tt>
			static constexpr void run_format(std::array<int, std::tuple_size<Bind_tt>::value>& write_here){}

			template<typename Bind_tt, typename Bind_t2 >
			static void run_to_db(
					std::array<std::string, std::tuple_size<Bind_tt>::value> &write_here,
					std::array<int, std::tuple_size<Bind_tt>::value> &length,
					const Bind_t2 & bind_me
			){}

			template<typename Return_tt>
			static void run_from_db(
				Return_tt & write_here,
				const PGresult *res,
				int row
			){}
		};
	}



	template<typename Bind_tt>
	auto constexpr oid()->std::array<Oid,std::tuple_size<Bind_tt>::value>{
		std::array<Oid,std::tuple_size<Bind_tt>::value> r = std::array<Oid,std::tuple_size<Bind_tt>::value>();
		impl::BindInfo_r<std::tuple_size<Bind_tt>::value>::template run_oid<Bind_tt>(r);
		return r;
	}

	template<typename Bind_tt>
	auto constexpr format()-> std::array<int,std::tuple_size<Bind_tt>::value>{
		std::array<int,std::tuple_size<Bind_tt>::value> r=std::array<int,std::tuple_size<Bind_tt>::value>();
		impl::BindInfo_r<std::tuple_size<Bind_tt>::value>::template run_format<Bind_tt>(r);
		return r;
	}

	template<typename Bind_tt, typename Bind_t2 >
	void to_db  (
			std::array<std::string, std::tuple_size<Bind_tt>::value> &write_here,
			std::array<int, std::tuple_size<Bind_tt>::value>         &length,
			const Bind_t2 & bind_me
	){
		impl::BindInfo_r<std::tuple_size<Bind_tt>::value>::template run_to_db<Bind_tt,Bind_t2>(write_here, length, bind_me);
	}


	template<typename Return_tt>
	void from_db(
			Return_tt & write_here,
			Result_t<tdb::Tag_psql, Return_tt>& result
	){
		//https://www.postgresql.org/docs/9.3/libpq-exec.html

		//use PQnparams to check that result size match Return_tt size
		static constexpr size_t expected_n_col = std::tuple_size<Return_tt>::value;
		assert(expected_n_col== PQnfields(result.native_result));
		/*if( PQnfields(res) != expected_n_col){
			throw Error_t<Tag_psql>("The result has "+std::to_string( PQnfields(res))+" columns, Return_tt has size " +std::to_string(expected_n_col) );
		}*/

		impl::BindInfo_r<expected_n_col>::template run_from_db<Return_tt> (
				write_here,
				result.native_result ,
				result.current_row
		);


	}

}




template <typename Return_tt, typename Bind_tt>
struct tdb::Bind_t<tdb::Tag_psql,Return_tt,Bind_tt>{
	template< typename Bind_t2>
	static void run(Query_t<tdb::Tag_psql, Return_tt, Bind_tt >& q, const Bind_t2& bind_me){
		 tdb::psql::template to_db<Bind_tt,Bind_t2> (q.paramValues, q.paramLengths ,bind_me); //bind in q
	}
};

template <typename Return_tt>
struct tdb::Get_row_t<tdb::Tag_psql,Return_tt>{
	template<typename Write_here_t>
	static void run(Result_t<tdb::Tag_psql, Return_tt>& q, Write_here_t & write_here){
		 tdb::psql::from_db(write_here,q);
	}
};





template<typename Return_tt>
struct tdb::Count_row_t<tdb::Tag_psql,Return_tt>{
	static constexpr bool is_implemented = true;
	static tdb::Rowid<tdb::Tag_psql> run(const tdb::Result_t<tdb::Tag_psql,Return_tt> &result){
		return PQntuples(result.native_result);
	}
};


template<typename Return_tt>
struct tdb::Try_fetch_t<tdb::Tag_psql,Return_tt>{
	static constexpr bool is_implemented=true;
	static std::optional<Return_tt> run(tdb::Result_t<tdb::Tag_psql,Return_tt> &result){
		//TODO_xxx function must be replaced here, by the correct implementation.

		bool row_exists = result.current_row < tdb::count_row(result);

		auto status = PQresultStatus(result.native_result);
		if(status ==PGRES_COMMAND_OK ){return std::optional<Return_tt>();} //empty result set

		bool ok = status==PGRES_TUPLES_OK;


		if(ok and row_exists ){
			std::optional<Return_tt> r =  get_row(result);
			++result.current_row;//next line
			return r;
		}

		if(ok and !row_exists ){
			return  std::optional<Return_tt>();//empty
		}

		throw Exception_t<Tag_psql>("Cannot fetch the data" );
	}
};







//=======================
//=== execute, insert ===
//=======================


//required : Execute_t
template<typename Return_tt, typename Bind_tt >
struct tdb::Execute_t <tdb::Tag_psql,Return_tt, Bind_tt >{
	static constexpr bool is_implemented = true;
	static void run(Query<tdb::Tag_psql,Return_tt, Bind_tt > &q){


		PGresult* res = nullptr;
		if constexpr(std::tuple_size<Bind_tt>::value == 0){
			res = PQexec(q.db->native_connection, q.native_sql.c_str() );
		}else{

			//std::string to c_str
			std::array<const char *, std::tuple_size<Bind_tt>::value > paramValues_cstr;
			for(size_t i = 0; i < std::tuple_size<Bind_tt>::value; ++i){
				paramValues_cstr[i]=q.paramValues[i].c_str();
			}

			res = PQexecParams(
					q.db->native_connection,  //connection
					q.native_sql.c_str(), //sql
					std::tuple_size<Bind_tt>::value, //nParams
					q.native_oid.data(),//paramTypes (i.e. oid)
					paramValues_cstr.data(),  //paramValues
					q.paramLengths.data(), //paramLengths
					q.paramFormats.data(), //paramFormats
					tdb::psql::FORMAT_TEXT

			);

		}


		if (PQresultStatus(res) != PGRES_COMMAND_OK){
			std::string msg = "Error in Execute_t: " + psql::result_error(res);
			PQclear(res);
			throw Exception_t<tdb::Tag_psql>(msg + "\n  sql: " + q.native_sql );
		}else{
			PQclear(res);
		}

	};
};





//required : Insert_t
template<typename Return_tt, typename Bind_tt>
struct tdb::Insert_t  <tdb::Tag_psql,Return_tt,Bind_tt>{
	static constexpr bool is_implemented = true;
	static Rowid<tdb::Tag_psql> run(Query<tdb::Tag_psql,Return_tt,Bind_tt> &q){
		//https://stackoverflow.com/questions/2944297/postgresql-function-for-last-inserted-id

		//NOTE : Return_tt is ignored. The query is executed as if it returns a Rowid
		//static_assert(std::is_same<Return_tt,std::tuple<Rowid<tdb::Tag_psql>> >::value, "Insert_t<Tag_psql> expect queries that return a rowid, ex :  insert into table_xxx(xxx) values(?) RETURNING xxx ");



		//auto res = PQexec(q.native_connection, q.native_sql.c_str() );

		PGresult* res = nullptr;
		if constexpr(std::tuple_size<Bind_tt>::value == 0){
			res = PQexec(q.db->native_connection, q.native_sql.c_str() );
		}else{

			//std::string to c_str
			std::array<const char *, std::tuple_size<Bind_tt>::value > paramValues_cstr;
			for(size_t i = 0; i < std::tuple_size<Bind_tt>::value; ++i){
				paramValues_cstr[i]=q.paramValues[i].c_str();
			}

			res = PQexecParams(
					q.db->native_connection,  //connection
					q.native_sql.c_str(), //sql
					std::tuple_size<Bind_tt>::value, //nParams
					q.native_oid.data(),//paramTypes (i.e. oid)
					paramValues_cstr.data(),  //paramValues
					q.paramLengths.data(), //paramLengths
					q.paramFormats.data(), //paramFormats
					tdb::psql::FORMAT_TEXT

			);

		}

		const int status = PQresultStatus(res);
		const bool ok = (status==PGRES_COMMAND_OK)or(status==PGRES_TUPLES_OK) ;
		if (!ok){
			std::string msg = "Error in Insert_t: " + psql::result_error(res);
			PQclear(res);
			throw Exception_t<tdb::Tag_psql>(msg+ "\n  sql: " + q.native_sql );
		}

		//nothing was inserted : return 0
		int nb_rows = PQntuples(res);
		if(nb_rows==0){return 0;}



		std::tuple<Rowid<tdb::Tag_psql>> write_here;
		psql::impl::BindInfo_r<1>::template run_from_db<  std::tuple<Rowid<tdb::Tag_psql>>  > (
				write_here,
				res,
				nb_rows-1 //last row
		);

		PQclear(res);
		return std::get<0>(write_here) ;
	}
};

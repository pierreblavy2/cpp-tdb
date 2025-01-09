#include "helpers/select_type.hpp"
#include <cstring> //strlen
#include <cassert>







//==================
//=== Connection ===
//==================

template<>
struct tdb::Get_mutex_t<tdb::Tag_sqlite>{
	static constexpr bool is_implemented = true;
	static auto & run(const tdb::Connection_t<tdb::Tag_sqlite> &c){
		return c.get_mutex();
	}
};



//=============
//=== Query ===
//=============

template<typename Return_tt, typename Bind_tt>
tdb::Query_t<tdb::Tag_sqlite,Return_tt,Bind_tt>::Query_t(
		tdb::Query_t<tdb::Tag_sqlite,Return_tt,Bind_tt>&&q
){
  std::swap(this->native_query        ,q.native_query);
  std::swap(this->native_connection   ,q.native_connection);
  std::swap(this->native_nb_bind      ,q.native_nb_bind);
}

template<typename Return_tt, typename Bind_tt>
auto tdb::Query_t<tdb::Tag_sqlite,Return_tt,Bind_tt>::operator = (
		tdb::Query_t<tdb::Tag_sqlite,Return_tt,Bind_tt>&&q
)->tdb::Query_t<tdb::Tag_sqlite,Return_tt,Bind_tt>&
{
	std::swap(this->native_query        ,q.native_query);
	std::swap(this->native_connection   ,q.native_connection);
	std::swap(this->native_nb_bind      ,q.native_nb_bind);
	return *this;
}


template<typename Return_tt, typename Bind_tt>
tdb::Query_t<tdb::Tag_sqlite,Return_tt,Bind_tt>::Query_t(
		tdb::Connection_t<tdb::Tag_sqlite> &c,
		const tdb::SqlData_t<tdb::Tag_sqlite> &sql
){
  //prepare
  //static constexpr unsigned int prepare_flags = ;

  int status = sqlite3_prepare_v2(c.native_connection, sql.c_str(), -1,  &native_query, 0);
  if(status !=  SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Wrong sqlite query, error_code=" + std::to_string(status)+ ", sql=" + sql.to_string() +", sqlite3_msg="+sqlite3_errmsg(c.native_connection) );}
  this->native_connection = c.native_connection;
}


//==============
//=== Result ===
//==============

//reset binding at destruction
template<typename Return_tt>
tdb::Result_t<tdb::Tag_sqlite,Return_tt>::~Result_t(){
	tdb::sqlite::reset_binding(native_query,native_nb_bind);
}

//movable
template<typename Return_tt>
tdb::Result_t<tdb::Tag_sqlite,Return_tt>::Result_t(tdb::Result_t<tdb::Tag_sqlite,Return_tt>&& a){
	std::swap(a.native_query,   this->native_query);
	std::swap(a.native_nb_bind, this->native_nb_bind);
	std::swap(a.native_result,  this->native_result);
}

template<typename Return_tt>
tdb::Result_t<tdb::Tag_sqlite,Return_tt>& tdb::Result_t<tdb::Tag_sqlite,Return_tt>::operator=(tdb::Result_t<tdb::Tag_sqlite,Return_tt>&& a){
	std::swap(a.native_query,   this->native_query);
	std::swap(a.native_nb_bind, this->native_nb_bind);
	std::swap(a.native_result,  this->native_result);
	return *this;
}

//Has_row_t (required)
//TODO a status enum in result :  no_lines, more_lines, finished.
// for multiple results
/*
while(1)
{
    // fetch a row's status
    retval = sqlite3_step(stmt);

    if(retval == SQLITE_ROW)
    {
        Something =
             (int)sqlite3_column_int(stmt, 0);
             // or other type - sqlite3_column_text etc.
        // ... fetch other columns, if there are any
    }
    else if(retval == SQLITE_DONE)
    {
        break;
    }
    else
    {
        sqlite3_finalize(stmt);
        printf("Some error encountered\n");
        break;
    }
}
*/

template<typename Return_tt>
struct tdb::Try_fetch_t<tdb::Tag_sqlite,Return_tt>{
	static constexpr bool is_implemented=true;
	static std::optional<Return_tt> run(tdb::Result_t<tdb::Tag_sqlite,Return_tt> &result){
		std::optional<Return_tt> r;

		result.native_result = sqlite3_step(result.native_query);

		if(result.native_result == SQLITE_ROW){
			r = get_row(result);
			return r;
		}
		else if(result.native_result == SQLITE_DONE){
			return r;
		}else{
			//sqlite3_finalize(result.native_query);
			throw Exception_t<Tag_sqlite>("Cannot fetch the data" + sqlite::error_to_string(result.native_result)  );
		}
	}
};



//================
//=== Bind_one ===
//================


//--- bind (required) ---
  //double        -> sqlite3_bind_double
  //sqlite3_int64 -> sqlite3_bind_int64
  //int           -> sqlite3_bind_int
  //size_t        -> sqlite3_int64       => MAY OVERFLOW (throw)
  //std::string   -> sqlite3_bind_text
  //const char*   -> sqlite3_bind_text
  //tdb::Null     -> sqlite3_bind_null
  //bool          -> sqlite3_bind_int
  //char          -> sqlite3_bind_text

//double -> sqlite3_bind_double
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,double,I>{
	static constexpr bool is_implemented=true;
	static_assert(I <  std::numeric_limits<int>::max() , "Too many bound parameters");

	template<typename Return_tt, typename Bind_tt>
	static void run(Query_t<Tag_sqlite,Return_tt,Bind_tt>&q, double d){
		++q.native_nb_bind;
		auto status = sqlite3_bind_double(q.native_query,q.native_nb_bind,d);
		if(status != SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Cannot bind double in sqlite3, index=" + std::to_string(q.native_nb_bind) +", value=" + std::to_string(d) + ",  error_code="+ std::to_string(status)+", sql="+q.sql_string());}
	};
};


//sqlite3_int64 -> sqlite3_bind_int64
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,sqlite3_int64,I>{
	static constexpr bool is_implemented=true;
	static_assert(I <  std::numeric_limits<int>::max() , "Too many bound parameters");

	template<typename Return_tt, typename Bind_tt>
	static void run(Query_t<Tag_sqlite,Return_tt,Bind_tt>&q, const sqlite3_int64 & d){
		++q.native_nb_bind;
		auto status = sqlite3_bind_int64(q.native_query,q.native_nb_bind,d);
		if(status != SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Cannot bind int64 in sqlite3, index=" + std::to_string(q.native_nb_bind) +", value=" + std::to_string(d) + ",  error_code="+ std::to_string(status)+", sql="+q.sql_string());}
	};
};


//int -> sqlite3_bind_int
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,int,I>{
	static constexpr bool is_implemented=true;
	static_assert(I <  std::numeric_limits<int>::max() , "Too many bound parameters");

	//see tdb::Bind_one_t<tdb::Tag_sqlite,size_t,I> if these assertion fail, to bind as blob
	//static_assert(std::numeric_limits<int>::max()   <=std::numeric_limits<sqlite3_int64>::max()   , "int are too large to be bound as sqlite3_int64 (max error)" );
	//static_assert(std::numeric_limits<int>::lowest()>=std::numeric_limits<sqlite3_int64>::lowest(), "int are too large to be bound as sqlite3_int64 (min error)" );
	template<typename Return_tt, typename Bind_tt>
	static void run(Query_t<tdb::Tag_sqlite,Return_tt,Bind_tt>&q, const int & d){
		++q.native_nb_bind;
		auto status = sqlite3_bind_int(q.native_query,q.native_nb_bind, d);
		if(status != SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Cannot bind int in sqlite3, index=" + std::to_string(q.native_nb_bind) +", value=" + std::to_string(d) + ",  error_code="+ std::to_string(status)+", sql="+q.sql_string());}
	};
};


//size_t -> sqlite3_int64   => MAY OVERFLOW (throw)
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,size_t,I>{
	static constexpr bool is_implemented=true;
	static_assert(I <  std::numeric_limits<int>::max() , "Too many bound parameters");

	template<typename Return_tt, typename Bind_tt>
	static void run(tdb::Query_t<tdb::Tag_sqlite,Return_tt,Bind_tt>&q, const size_t & d){
	++q.native_nb_bind;

	//if false there is a runtime error if values are bigger than int64

	//NOT TESTED : blob version
	//	auto status = sqlite3_bind_blob (q.query_native, q.nb_bind, &d, sizeof(size_t), SQLITE_STATIC);
	//	if(status != SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Cannot bind size_t as blob in sqlite3, index=" + std::to_string(q.nb_bind) +", value=" + std::to_string(d) + ",  error_code="+ std::to_string(status)+", sql="+q.sql_string());}
	//	return;

	//convert to the largest type first
	typedef typename tdb::impl::select_type< (std::numeric_limits<sqlite3_int64>::max() > std::numeric_limits<size_t>::max())      , sqlite3_int64, size_t >::type max_t;
	typedef typename tdb::impl::select_type< (std::numeric_limits<sqlite3_int64>::lowest() < std::numeric_limits<size_t>::lowest()), sqlite3_int64, size_t >::type min_t;

	bool is_small_enough =
				static_cast<max_t>(d) <= static_cast<max_t>( std::numeric_limits<sqlite3_int64>::max() )
			and static_cast<min_t>(d) >= static_cast<min_t>( std::numeric_limits<sqlite3_int64>::lowest() );
	if(!is_small_enough){
		throw Exception_t<tdb::Tag_sqlite>(
				"Cannot bind size_t as int64 in sqlite3, the size is out of range"
				", value="+std::to_string(d)+
				", min="+std::to_string(std::numeric_limits<sqlite3_int64>::lowest())+
				", max="+std::to_string(std::numeric_limits<sqlite3_int64>::max())+
				", index="   + std::to_string(q.native_nb_bind) +
				", sql="+q.sql_string()
		);
	}
	auto status = sqlite3_bind_int64(q.native_query,q.native_nb_bind, static_cast<sqlite3_int64>(d));
	if(status != SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Cannot bind size_t as sqlite3_int64 in sqlite3, index=" + std::to_string(q.native_nb_bind) +", value=" + std::to_string(d) + ",  error_code="+ std::to_string(status)+", sql="+q.sql_string());}
	return;

	};
};


//std::string -> sqlite3_bind_text
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,std::string,I>{
	static constexpr bool is_implemented=true;
	static_assert(I <  std::numeric_limits<int>::max() , "Too many bound parameters");

	template<typename Return_tt, typename Bind_tt>
	static void run(Query_t<Tag_sqlite,Return_tt,Bind_tt>&q, const std::string & d){
		++q.native_nb_bind;
		const size_t d_size = d.size();
		assert(d_size <  std::numeric_limits<int>::max());

		auto status = sqlite3_bind_text(q.native_query,q.native_nb_bind,d.c_str(),static_cast<int>(d_size),SQLITE_STATIC);
		if(status != SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Cannot bind string as text, index=" + std::to_string(q.native_nb_bind) +", value=" + d + ",  error_code="+ std::to_string(status)+", sql="+q.sql_string());}
	}
};


//const char* -> sqlite3_bind_text
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,const char * ,I>{
	  static constexpr bool is_implemented=true;
	  static_assert(I <  std::numeric_limits<int>::max() , "Too many bound parameters");

	  template<typename Return_tt, typename Bind_tt>
	  static void run(Query_t<Tag_sqlite,Return_tt,Bind_tt>&q, const char *  d){
		++q.native_nb_bind;
		const size_t d_size = std::strlen(d);
		assert(d_size <  std::numeric_limits<int>::max());

		auto status = sqlite3_bind_text(q.native_query,q.native_nb_bind,d,static_cast<int>(d_size),SQLITE_STATIC);
		if(status != SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Cannot bind const char* as text, index=" + std::to_string(q.native_nb_bind) +", value=" + std::string(d) + ",  error_code="+ std::to_string(status)+", sql="+q.sql_string());}
	  }
};


//Null -> sqlite3_bind_null
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,tdb::Null ,I>{
	  static constexpr bool is_implemented=true;
	  static_assert(I <  std::numeric_limits<int>::max() , "Too many bound parameters");

	  template<typename Return_tt, typename Bind_tt>
	  static void run(Query_t<Tag_sqlite,Return_tt,Bind_tt>&q, const tdb::Null d){
		++q.native_nb_bind;
		auto status = sqlite3_bind_null(q.native_query,q.native_nb_bind);
		if(status != SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Cannot bind null, index=" + std::to_string(q.native_nb_bind) +",  error_code="+ std::to_string(status)+", sql="+q.sql_string());}
	  }
};


//bool -> sqlite3_bind_int
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,bool,I>{
	static constexpr bool is_implemented=true;

	template<typename Return_tt, typename Bind_tt>
	static void run(Query_t<Tag_sqlite,Return_tt,Bind_tt>&q, const bool  d){
		++q.native_nb_bind;

		int i; if(d){i=1;}else{i=0;}
		auto status = sqlite3_bind_int(q.native_query,q.native_nb_bind, i);
		if(status != SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Cannot bind bool as int in sqlite3, index=" + std::to_string(q.native_nb_bind) +", value=" + std::to_string(d) + ",  error_code="+ std::to_string(status)+", sql="+q.sql_string());}
	};
};


//char -> sqlite3_bind_text
template<size_t I> struct tdb::Bind_one_t<tdb::Tag_sqlite,char,I>{
	static constexpr bool is_implemented=true;

	template<typename Return_tt, typename Bind_tt>
	static void run(Query_t<Tag_sqlite,Return_tt,Bind_tt>&q, const char d){
		++q.native_nb_bind;
		auto status = sqlite3_bind_text(q.native_query,q.native_nb_bind,&d,sizeof(char),SQLITE_STATIC);
		if(status != SQLITE_OK){throw Exception_t<tdb::Tag_sqlite>("Cannot bind char as text, index=" + std::to_string(q.native_nb_bind) +", value=" + d + ",  error_code="+ std::to_string(status)+", sql="+q.sql_string());}
	}
};




//===============
//=== Get_one ===
//===============
//double <- sqlite3_column_double;
template<size_t I>
struct tdb::Get_one_t<tdb::Tag_sqlite,double,I>{
	  static constexpr bool is_implemented=true;

	  template<typename Return_tt>
	  static void run(const Result_t<Tag_sqlite,Return_tt> &result, double& write_here){
		  //assert(I < sqlite3_column_count(result.native_query) );
		  assert(I < std::numeric_limits<int>::max() );
		  const auto coltype=sqlite3_column_type(result.native_query, static_cast<int>(I) );
		  if(coltype!=SQLITE_FLOAT){throw tdb::Exception_t<tdb::Tag_sqlite>("sqlite : wrong type cannot get double ,column="+std::to_string(I)+", sql="+result.sql_string() + ", type=" + tdb::sqlite::coltype_to_string(coltype));}
		  write_here=sqlite3_column_double(result.native_query,static_cast<int>(I));
	 }
};


//int <- sqlite3_column_int
template<size_t I>
struct tdb::Get_one_t<tdb::Tag_sqlite,int,I>{
	  static constexpr bool is_implemented=true;

	  template<typename Return_tt>
	  static void run(const Result_t<Tag_sqlite,Return_tt> &result, int& write_here){
		  static_assert(I <  std::numeric_limits<int>::max(),"I is too large");
		  const auto coltype=sqlite3_column_type(result.native_query, static_cast<int>(I) );
		  if(coltype!=SQLITE_INTEGER){throw tdb::Exception_t<tdb::Tag_sqlite>("sqlite : wrong type cannot get int ,column="+std::to_string(I)+", sql="+result.sql_string() + ", type=" + tdb::sqlite::coltype_to_string(coltype));}
		  write_here=sqlite3_column_int(result.native_query,static_cast<int>(I));
	 }
};


//sqlite3_int64 <- sqlite3_column_int64
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,sqlite3_int64,I>{
	  static constexpr bool is_implemented=true;

	  template<typename Return_tt>
	  static void run(const Result_t<Tag_sqlite,Return_tt> &result, sqlite3_int64& write_here){
		  static_assert(I <  std::numeric_limits<int>::max(),"I is too large");
		  const auto coltype=sqlite3_column_type(result.native_query, static_cast<int>(I) );
		  if(coltype!=SQLITE_INTEGER){throw tdb::Exception_t<tdb::Tag_sqlite>("sqlite : wrong type cannot get int ,column="+std::to_string(I)+", sql="+result.sql_string() + ", type=" + tdb::sqlite::coltype_to_string(coltype));}
		  write_here=sqlite3_column_int64(result.native_query,static_cast<int>(I));
	 }
};


//size_t <- sqlite3_column_int64
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,size_t,I>{
	  static constexpr bool is_implemented=true;

	  template<typename Return_tt>
	  static void run(const Result_t<Tag_sqlite,Return_tt> &result, size_t& write_here){
		  static_assert(I <  std::numeric_limits<int>::max(),"");
		  //assert( static_cast<int>(I) < sqlite3_column_count(result.native_query) );
		  //assert( result.native_query != nullptr);
		  const auto coltype=sqlite3_column_type(result.native_query, static_cast<int>(I) );
		  if(coltype!=SQLITE_INTEGER){throw tdb::Exception_t<tdb::Tag_sqlite>("sqlite : wrong type cannot get size_t as sqlite3_int64 ,column="+std::to_string(I)+", sql="+result.sql_string() + ", type=" + tdb::sqlite::coltype_to_string(coltype));}
		  //TODO bound checking
		  write_here=sqlite3_column_int64(result.native_query,static_cast<int>(I));
	 }
};


//std::string <- sqlite3_column_text
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,std::string,I>{
	  static constexpr bool is_implemented=true;

	  template<typename Return_tt>
	  static void run(const Result_t<Tag_sqlite,Return_tt> &result, std::string& write_here){
		  //doc : https://stackoverflow.com/questions/804123/const-unsigned-char-to-stdstring
		  static_assert(I <  std::numeric_limits<int>::max(),"I is too large");
		  const auto coltype=sqlite3_column_type(result.native_query, static_cast<int>(I) );
		  if(coltype!=SQLITE_TEXT){throw tdb::Exception_t<tdb::Tag_sqlite>("sqlite : wrong type cannot get string ,column="+std::to_string(I)+", sql="+result.sql_string() + ", type=" + tdb::sqlite::coltype_to_string(coltype));}
		  write_here=static_cast<std::string>( reinterpret_cast<const char*>(sqlite3_column_text  (result.native_query, static_cast<int>(I))) );
	 }
};


//bool <- sqlite3_column_int (MAY THROW)
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,bool,I>{
	  static constexpr bool is_implemented=true;

	  template<typename Return_tt>
	  static void run(const Result_t<Tag_sqlite,Return_tt> &result, std::string& write_here){
		  //get an int
		  const auto coltype=sqlite3_column_type(result.native_query, static_cast<int>(I) );
		  if(coltype!=SQLITE_INTEGER){throw tdb::Exception_t<tdb::Tag_sqlite>("sqlite : wrong type cannot get bool (from integer) ,column="+std::to_string(I)+", sql="+result.sql_string() + ", type=" + tdb::sqlite::coltype_to_string(coltype));}
		  int i=sqlite3_column_int(result.native_query,static_cast<int>(I));

		  //int to bool
		  if     (i==0){write_here = false; return;}
		  else if(i==1){write_here = true;  return;}
		  throw Exception_t<tdb::Tag_sqlite>("sqlite : wrong value cannot interpret int as bool ,column="+std::to_string(I)+", sql="+result.sql_string() + ", type=" + tdb::sqlite::coltype_to_string(coltype), ", int="+std::to_string(i));

	  }
};


//char <- sqlite3_column_text
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,char,I>{
	  static constexpr bool is_implemented=true;

	  template<typename Return_tt>
	  static void run(const Result_t<Tag_sqlite,Return_tt> &result, std::string& write_here){
		  //get string
		  std::string s;
		  tdb::Get_one_t<tdb::Tag_sqlite,std::string,I>::run(result,s);

		  //string to char
		  if(s.size()!=1){
				 throw Exception_t<tdb::Tag_sqlite>("sqlite : wrong value cannot interpret string as char ,column="+std::to_string(I)+", sql="+result.sql_string(), ", string="+s+", string_size="+std::to_string(s.size()) );
		  }
		  write_here=s[0];
	  }
};

//Null <- do nothing
template<size_t I> struct tdb::Get_one_t<tdb::Tag_sqlite,tdb::Null,I>{
	  static constexpr bool is_implemented=true;

	  template<typename Return_tt>
	  static void run(const Result_t<Tag_sqlite,Return_tt> &result, Null& write_here){}
};


//std::optional<T> <- dispatch, treat Null as empty optional
template<size_t I, typename T >
struct tdb::Get_one_t<tdb::Tag_sqlite,std::optional<T>, I >{
  static constexpr bool is_implemented=true;

  template<typename Return_tt>
  static void run(const Result_t<Tag_sqlite,Return_tt> &result,std::optional<T> & write_here){
		  //doc : https://stackoverflow.com/questions/804123/const-unsigned-char-to-stdstring
		  static_assert(I <  std::numeric_limits<int>::max(),"I is too large");
		  const auto coltype=sqlite3_column_type(result.native_query, static_cast<int>(I) );
		  if(coltype==SQLITE_NULL){write_here.reset(); return;};

		  T t;
		  Get_one_t<Tag_sqlite,T,I>::run(result,t);
		  write_here=t;

	}
};





//=========================================
//=== Execute_t, Insert_t, Get_result_t ===
//=========================================
//Execute_t (required)
template<typename Return_tt, typename Bind_tt>
struct tdb::Execute_t<tdb::Tag_sqlite,Return_tt,Bind_tt >{
	static constexpr bool is_implemented = true;
	static void run(Query<tdb::Tag_sqlite,Return_tt,Bind_tt> &q){

		tdb::sqlite::Query_guard query_guard(q);

		int querry_result;
		do{
			querry_result = sqlite3_step(q.native_query);
		}while(querry_result  == SQLITE_ROW);

		if(querry_result!=SQLITE_DONE){
			throw Exception_t<Tag_sqlite>("sqlite : error during execute : error_code=" + sqlite::error_to_string(querry_result)+", sql="+q.sql_string()+", msg="+sqlite3_errmsg(q.native_connection));
		}
	}
};



//Insert_t (required)
template<typename Return_tt, typename Bind_tt>
struct tdb::Insert_t<tdb::Tag_sqlite,Return_tt,Bind_tt>{
	static constexpr bool is_implemented = true;
	static Rowid<tdb::Tag_sqlite> run(Query<tdb::Tag_sqlite,Return_tt,Bind_tt> &q){
		tdb::sqlite::Query_guard query_guard(q);

		int querry_result;
		do{querry_result = sqlite3_step(q.native_query);}
		while(querry_result  == SQLITE_ROW);

		if(querry_result!=SQLITE_DONE){throw Exception_t<Tag_sqlite>("sqlite : error during execute, error_code=" + sqlite::error_to_string(querry_result)+", sql="+q.sql_string()+", msg="+sqlite3_errmsg(q.native_connection));}
		auto rowid=sqlite3_last_insert_rowid(q.native_connection);

		return rowid; //and reset Query_t (by query_guard)
	}
};













#ifndef TDB_BIND_SQLITE_CHRONO_HPP
#define TDB_BIND_SQLITE_CHRONO_HPP

#include <chrono>
#include  "../tdb_sqlite.hpp"


//=== Bind durations ===
template<size_t I,typename Rep, typename Period, typename... A>
struct tdb::Bind_one_t<
  tdb::Tag_sqlite,
  std::chrono::duration<Rep,Period,A...>,
  I
>{
	static constexpr bool is_implemented=true;
	static_assert(I <  std::numeric_limits<int>::max() , "Too many bound parameters");

	typedef std::chrono::duration<Rep,Period,A...>        duration_t;
	//typedef decltype( std::declval<duration_t>().count() ) count_t;
	typedef Rep count_t;


	template<typename Return_tt, typename Bind_tt>
	static void run(Query_t<Tag_sqlite,Return_tt,Bind_tt>&q, const  duration_t& x){
		count_t tmp = x.count();
		tdb::Bind_one_t<tdb::Tag_sqlite,count_t,I>::run(q,tmp);
	};
};



//=== Get durations ===
template<size_t I,typename Rep, typename Period, typename... A>
struct tdb::Get_one_t<tdb::Tag_sqlite,std::chrono::duration<Rep,Period,A...>,I>{
	  static constexpr bool is_implemented=true;
	  typedef std::chrono::duration<Rep,Period,A...> duration_t;
	  typedef Rep count_t;


	  template<typename Return_tt>
	  static void run(const Result_t<Tag_sqlite,Return_tt> &result, duration_t& write_here){
		  static_assert(I <  std::numeric_limits<int>::max(),"I is too large");
		  const auto coltype=sqlite3_column_type(result.native_query, static_cast<int>(I) );
		  if(coltype!=SQLITE_INTEGER){throw tdb::Exception_t<tdb::Tag_sqlite>("sqlite : wrong type cannot get integer (for chrono_duration) ,column="+std::to_string(I)+", sql="+result.sql_string() + ", type=" + tdb::sqlite::coltype_to_string(coltype));}
		  count_t tmp =static_cast<count_t>( sqlite3_column_int64(result.native_query,static_cast<int>(I)) );
		  write_here  = duration_t(tmp);
	 }
};




#endif

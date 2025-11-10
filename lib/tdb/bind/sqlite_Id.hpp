#ifndef LIB_TDB_BIND_SQLITE_ID_HPP_
#define LIB_TDB_BIND_SQLITE_ID_HPP_

#include <tdb/tdb_sqlite.hpp>
#include <Id.hpp>

template<size_t I, typename Value_t, typename Tag_t, bool is_arithmetic>
struct tdb::Bind_one_t<
  tdb::Tag_sqlite,
  Id_t<Value_t,Tag_t,is_arithmetic>,
  I
>{
	static constexpr bool is_implemented=true;
	static_assert(I <  std::numeric_limits<int>::max() , "Too many bound parameters");

	template<typename Return_tt, typename Bind_tt>
	static void run(Query_t<Tag_sqlite,Return_tt,Bind_tt>&q, const  Id_t<Value_t,Tag_t,is_arithmetic>& x){
		tdb::Bind_one_t<tdb::Tag_sqlite,Value_t,I>::run(q,x.value);
	};
};





#endif /* LIB_TDB_BIND_SQLITE_ID_HPP_ */

#ifndef LIB_TDB_FUNCTORS_EXAMPLES_FN_GET_VALUE_CPP_
#define LIB_TDB_FUNCTORS_EXAMPLES_FN_GET_VALUE_CPP_


#include "../Fn_get_value_unique.hpp"
#include "../Fn_get_value_optional.hpp"

#include <iostream>
#include <tdb/tdb_sqlite.hpp>


//test code
namespace{

[[maybe_unused]] void example(){

	typedef tdb::Tag_sqlite Tag_xxx;
	tdb::Connection_t<Tag_xxx> connection("/tmp/test.sqlite");

	//--- Fn_value_unique ---
	//multi thread
    tdb::Fn_get_value_unique<
	  Tag_xxx ,
	  std::tuple<int>,
	  std::tuple<double,double>,
      true
	> fn_unique1(connection, "select max(i1) from test where d1 != $1 and d2 != $2");
    [[maybe_unused]] int i1 = fn_unique1(1.0,1.0);

	//single thread
    tdb::Fn_get_value_unique<
	  Tag_xxx ,
	  std::tuple<int>,
	  std::tuple<double,double>,
      false
	> fn_unique2(connection, "select max(i1) from test where d1 != $1 and d2 != $2");
    [[maybe_unused]] int i2 = fn_unique2(1.0,1.0);


	//--- Fn_value_optional ---
    //multi thread
    tdb::Fn_get_value_optional<
	  Tag_xxx ,
	  std::tuple<int>,
	  std::tuple<double,double>,
      true
	> fn_optional3(connection, "select max(i1) from test where d1 != $1 and d2 != $2");
    [[maybe_unused]]  std::optional<int> i3 = fn_optional3(1.0,1.0);

    //single thread
    tdb::Fn_get_value_optional<
	  Tag_xxx ,
	  std::tuple<int>,
	  std::tuple<double,double>,
      false
	> fn_optional4(connection, "select max(i1) from test where d1 != $1 and d2 != $2");
    [[maybe_unused]]  std::optional<int> i4 = fn_optional4(1.0,1.0);


}
}


#endif /* LIB_TDB_FUNCTORS_EXAMPLES_FN_GET_VALUE_CPP_ */

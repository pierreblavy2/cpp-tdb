#include "../Fn_get_row_optional.hpp"
#include "../Fn_get_row_unique.hpp"


#include <iostream>
#include <tdb/tdb_sqlite.hpp>


//test code
namespace{

[[maybe_unused]] void example(){

	typedef tdb::Tag_sqlite Tag_xxx;
	tdb::Connection_t<Tag_xxx> connection("/tmp/test.sqlite");


	//--- get unique ---
	//multi thread
	tdb::Fn_get_row_unique<
		Tag_xxx ,
		std::tuple<int,int>,
		std::tuple<double,double>,
		true
	> fn_row_unique1(connection, "select max(i1), max(i2) from test where d1 != $1 and d2 != $2");
	[[maybe_unused]] std::tuple<int,int> fn_row_unique_r1 =  fn_row_unique1(1.0,1.0);

	//single thread
	tdb::Fn_get_row_unique<
			Tag_xxx ,
			std::tuple<int,int>,
			std::tuple<double,double>,
			false
	> fn_row_unique2(connection, "select max(i1), max(i2) from test where d1 != $1 and d2 != $2");
	[[maybe_unused]] std::tuple<int,int> fn_row_unique_r2 =  fn_row_unique2(1.0,1.0);



	//--- get optional ---
	//multi thread
	tdb::Fn_get_row_optional<
		Tag_xxx ,
		std::tuple<int,int>,
		std::tuple<double,double>,
		true
	> fn_row_optional3(connection, "select max(i1), max(i2) from test where d1 != $1 and d2 != $2");
	[[maybe_unused]] std::optional<std::tuple<int,int> > fn_row_optional_r3 =  fn_row_optional3(1.0,1.0);

	//single thread
	tdb::Fn_get_row_optional<
		Tag_xxx ,
		std::tuple<int,int>,
		std::tuple<double,double>,
		false
	> fn_row_optional4(connection, "select max(i1), max(i2) from test where d1 != $1 and d2 != $2");
	[[maybe_unused]] std::optional<std::tuple<int,int> > fn_row_optional_r4 =  fn_row_optional4(1.0,1.0);

}

}

#include "../Fn_execute.hpp"

#include <iostream>
#include <tdb/tdb_sqlite.hpp>
#include <tdb/tdb_psql.hpp>

//test code
namespace{
    [[maybe_unused]] void example(){

	typedef tdb::Tag_sqlite Tag_xxx;
	tdb::Connection_t<Tag_xxx> connection("/tmp/test.sqlite");


	//multi thread
	tdb::Fn_execute<
	  Tag_xxx ,
	  std::tuple<>,
	  std::tuple<int>,
	  true
	> fn_execute(connection, "delete from test where i1=$1");
	fn_execute(42);

	//single thread
	tdb::Fn_execute<
	  Tag_xxx ,
	  std::tuple<>,
	  std::tuple<int>,
	  false
	> fn_execute_st(connection, "delete from test where i1=$1");
	fn_execute_st(42);



	std::cout << "Fn_execute ok"<<std::endl;

    }
}

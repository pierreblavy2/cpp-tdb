#include "../Fn_foreach.hpp"

#include <iostream>
#include <tdb/tdb_sqlite.hpp>

//test code
namespace{
    [[maybe_unused]] void example(){

	typedef tdb::Tag_sqlite Tag_xxx;
	tdb::Connection_t<Tag_xxx> connection("/tmp/test.sqlite");


	//--- Fn_foreach (void) ---
	//multi thread
    tdb::Fn_foreach<
	  Tag_xxx ,
	  std::tuple<int>,
	  std::tuple<double,double>,
      true
	> fn_foreach(connection, "select i1 from test where d1 != $1 and d2 != $2");
    fn_foreach( [](int i){std::cout <<i<<std::endl;}, 1.0,1.0);

	//single thread
    tdb::Fn_foreach<
	  Tag_xxx ,
	  std::tuple<int>,
	  std::tuple<double,double>,
      false
	> fn_foreach2(connection, "select i1 from test where d1 != $1 and d2 != $2");
    fn_foreach2( [](int i){std::cout <<i<<std::endl;}, 1.0,1.0);

	std::cout << "Fn_foreach (void) "<<std::endl;



	//--- Fn_foreach (bool) ---
	//multi thread
    tdb::Fn_foreach<
	  Tag_xxx ,
	  std::tuple<int>,
	  std::tuple<double,double>,
      true
	> fn_foreach3(connection, "select i1 from test where d1 != $1 and d2 != $2");
    [[maybe_unused]] bool b1 = fn_foreach3( [](int i)->bool{std::cout <<i<<std::endl; return true;}, 1.0,1.0);

	//single thread
    tdb::Fn_foreach<
	  Tag_xxx ,
	  std::tuple<int>,
	  std::tuple<double,double>,
      false
	> fn_foreach4(connection, "select i1 from test where d1 != $1 and d2 != $2");
    [[maybe_unused]] bool b2 = fn_foreach4( [](int i)->bool{std::cout <<i<<std::endl; return true;}, 1.0,1.0);

	std::cout << "Fn_foreach (void) "<<std::endl;




    }
}

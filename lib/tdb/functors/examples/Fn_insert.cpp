#include "../Fn_insert.hpp"

#include <iostream>
#include <tdb/tdb_sqlite.hpp>


//test code
namespace{

[[maybe_unused]] void example(){

	typedef tdb::Tag_sqlite Tag_xxx;
	tdb::Connection_t<Tag_xxx> connection("/tmp/test.sqlite");

	//--- Fn_insert ---
	//WARNING : rowid is undefined for inapropriate queries, like psql insert
	//statements without "returning xxxx", or sqlite statements that do not insert

	//multi thread
    tdb::Fn_insert<
	Tag_xxx ,
	  std::tuple<>,
	  std::tuple<int,int,double,double>,
      true
	> fn_insert1(connection, "insert into test(i1,i2,d1,d2) values ($1,$2,$3,$4) returning i1");
    [[maybe_unused]] tdb::Rowid<Tag_xxx> rowid1 = fn_insert1(42,43,42.0,43.0);

    //single thread
    tdb::Fn_insert<
	Tag_xxx ,
	  std::tuple<>,
	  std::tuple<int,int,double,double>,
      false
	> fn_insert2(connection, "insert into test(i1,i2,d1,d2) values ($1,$2,$3,$4) returning i1");
    [[maybe_unused]] tdb::Rowid<Tag_xxx> rowid2 = fn_insert2(42,43,42.0,43.0);



}
}


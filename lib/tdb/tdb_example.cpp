#include "tdb_psql.hpp"
#include <iostream>

//Fn_xxx examples are in functors/example/
//The template file for writing database drivers is in doc/tdb_template.hpp

namespace{
  [[maybe_unused]] void example(){

		tdb::Connection_t<tdb::Tag_psql> connection("pierre","127.0.0.1","pierre","xxxxx");

		tdb::execute (connection,"drop table if exists test;");
		tdb::execute (connection,"create table if not exists test(i1 integer, i2 integer, d1 float, d2 float);");

		tdb::execute  (connection,"insert into test (i1,i2,d1,d2)values($1,$2,$3,$4);", std::make_tuple(1,1,0.5,0.4) );
		tdb::execute_a(connection,"insert into test (i1,i2,d1,d2)values($1,$2,$3,$4);", 10,1,0.5,0.4 );

		auto rowid = tdb::insert  (connection,"insert into test (i1,i2,d1,d2)values(100,1,1,1) RETURNING i1");
		assert(rowid==100);

		auto tr = tdb::transaction(connection);
			tdb::insert  (connection,"insert into test (i1,i2,d1,d2)values($1,$2,$3,$4);", std::make_tuple(1000,1,0.5,0.4) );
			tdb::insert_a(connection,"insert into test (i1,i2,d1,d2)values($1,$2,$3,$4);", 10000,1,0.5,0.4 );
		tr.commit();


		//--- query, result, fetch ---
		auto query =  tdb::prepare_new< std::tuple<int> , std::tuple<> >(connection,"select i1 from test");
		auto result = tdb::get_result_a(query);

		//TODO test this code for sqlite
		while( auto r = try_fetch(result) ){
		    std::cout << std::get<0>(r.value())<<"\n";
		}
  }
}

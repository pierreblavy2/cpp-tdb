#include "../Fn_get_table.hpp"

#include <iostream>
#include <tdb/tdb_sqlite.hpp>

#include <container/vector.hpp>

//test code
namespace{

[[maybe_unused]] void example(){

	typedef tdb::Tag_sqlite Tag_xxx;
	tdb::Connection_t<Tag_xxx> connection("/tmp/test.sqlite");


	//--- Fn_get_table  ---
	//multi thread
    tdb::Fn_get_table<
	  Tag_xxx ,
	  std::tuple<int,int>,
	  std::tuple<double,double>,
      true
	> fn_insert_table1(connection, "select i1,i2 from test where d1 != $1 and d2 != $2");
    std::vector<std::tuple<int,int> >table1;

    fn_insert_table1(std::back_insert_iterator(table1), 5.5 , 8.4); //output_iterator
    fn_insert_table1(                          table1 , 5.5 , 8.4); //idem, container (require container/vector.hpp)

	//single thread
    tdb::Fn_get_table<
	  Tag_xxx ,
	  std::tuple<int,int>,
	  std::tuple<double,double>,
      false
	> fn_insert_table2(connection, "select i1,i2 from test where d1 != $1 and d2 != $2");
    std::vector<std::tuple<int,int> >table2;

    fn_insert_table2(std::back_insert_iterator(table2), 5.5 , 8.4); //output_iterator
    fn_insert_table2(                          table2 , 5.5 , 8.4); //idem, container (require container/vector.hpp)


}
}

#include "../Fn_get_column.hpp"

#include <iostream>
#include <tdb/tdb_sqlite.hpp>

#include <container/vector.hpp>


//test code
namespace{

[[maybe_unused]] void example(){

	typedef tdb::Tag_sqlite Tag_xxx;
	tdb::Connection_t<Tag_xxx> connection("/tmp/test.sqlite");


	//multi thread
    tdb::Fn_get_column<
	  Tag_xxx ,
	  std::tuple<int>,
	  std::tuple<double,double>,
      true
	> fn_get_column1(connection, "select i1 from test where d1 != $1 and d2 != $2");
    std::vector<int> v1;

    fn_get_column1(std::back_insert_iterator(v1), 5.5 , 8.4); //output iterator
    fn_get_column1(v1                           , 5.5 , 8.4); //idem container (require container/vector.hpp)


	//single thread
    tdb::Fn_get_column<
	  Tag_xxx ,
	  std::tuple<int>,
	  std::tuple<double,double>,
      false
	> fn_get_column2(connection, "select i1 from test where d1 != $1 and d2 != $2");
    std::vector<int> v2;
    fn_get_column2(std::back_insert_iterator(v2), 5.5 , 8.4); //output iterator
    fn_get_column2(v2                           , 5.5 , 8.4); //idem container (require container/vector.hpp)




}





}

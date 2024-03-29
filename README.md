# cpp-tdb : C++ Template Data Base wrapper

## Introduction
This project allows to connect to **postresql** and **sqlite** databases, execute queries and retrive results in a consistent way. A template user friendly interface is provided in order to easily convert sql queries to functors and to add support for user defined types. 

New backend can be added to the library by template specialization.

## License
The code is published under [LGPL 3.0](https://www.gnu.org/licenses/lgpl-3.0.txt) license. Copyright goes to Pierre Blavy, INRA.

# Usage

## Install
- Copy the /src/lib folder into your project
- Include : -Ilib/
- Link : -lsqlite -lpq


## Examples 
Queries examples are in [src/lib/tdb/tdb_examples.cpp](lib/tdb/functors/examples)

## Execute queries
Note that, the C++ code is the same for Sqlite, simply `#include tdb/tdb_sqlite.hpp` and use `tdb::Tag_sqlite`. The SQL is somehow different. For example SQLITE doesn't need a return statement in insert queries in order to get a Rowid, and uses ? as placeholder for bound parameters.

```cpp
#include <tdb/tdb_psql.hpp>
#include <iostream>

void example(){
  //connect
  //conncetion accept arbitrary args (depending on the underlying database driver)
  //but we expect : db_name, host, user, pass, port
  tdb::Connection_t<tdb::Tag_psql> connection("pierre","127.0.0.1","pierre","xxxxx");

  //execute 
  tdb::execute (connection,"create table if not exists test(i1 integer, i2 integer, d1 float, d2 float);");

  //execute + bind
  tdb::execute  (connection,"insert into test (i1,i2,d1,d2)values($1,$2,$3,$4);", std::make_tuple(1,1,0.5,0.4) );
  tdb::execute_a(connection,"insert into test (i1,i2,d1,d2)values($1,$2,$3,$4);", 10,1,0.5,0.4 );

  //insert => return Rowid
  tdb::Rowid<tdb::Tag_psql> rowid = tdb::insert  (connection,"insert into test (i1,i2,d1,d2)values(100,1,1,1) RETURNING i1");

  //insert + bind => return Rowid
  tdb::Rowid<tdb::Tag_psql> rid;
  rid = tdb::insert  (connection,"insert into test (i1,i2,d1,d2)values($1,$2,$3,$4);", std::make_tuple(1000,1,0.5,0.4) );
  rid = tdb::insert_a(connection,"insert into test (i1,i2,d1,d2)values($1,$2,$3,$4);", 10000,1,0.5,0.4 );

  //prepare<Return_tt, Bind_tt>
  //Return_tt : a tuple that contains types returned by the query
  //Bind_tt   : a tuple that contains types bounded to the query.
  //NOTE :  Return_tt and Bind_tt uses unqualified types (no const, no references)
  auto query =  tdb::prepare_new< std::tuple<int> , std::tuple<> >(connection,"select i1 from test");

  //get_reqult(query, const Bind_tt&)
  //get_result_a(query, bind...)
  auto result = tdb::get_result_a(query);
	
  while( auto r = try_fetch(result) ){
    std::cout << std::get<0>(r.value())<<"\n";
  }
}
```


## Functors
The tdb library allows to prepare queries, and encapsulate parameters binding and result fetching into functors. Functors reference the underlying connection (so it must live longer thant the functor), and holds a prepared statement. 

```cpp
#include <tdb/functors/Fn_get_value_unique.hpp>
void example(){

  typedef tdb::Tag_sqlite Tag_xxx;
  tdb::Connection_t<Tag_xxx> connection("/tmp/test.sqlite");
  static constexpr bool is_multi_thread = true;

  tdb::Fn_get_value_unique<
    Tag_xxx ,                    //tag that identifies the data base driver
    std::tuple<int>,             //Return_tt return type (always in a tuple, no references, no const)
    std::tuple<double,double>,   //Bind_tt   bounded parameters types, (always in a tuple, no references, no const)
    is_multi_thread              //Use the multi thread version (i.e., automatically locks the connection mutex)
  > fn(connection, "select max(i1) from test where d1 != $1 and d2 != $2");

  //fn is a functor, that can be called as a regular function
  int i = fn(1.0,1.0);

}
```

- Functors examples are in [src/lib/tdb/functors/examples/](lib/tdb/functors/examples) 
- All functors summary are in [src/lib/tdb/functors/all.hpp](lib/tdb/functors/all.hpp) 


Functor_name (1)|prototype (2)|Extra_t... | Example |
----------------|-------------|-----------|---------|
`tdb::Fn_execute`      |`void fn(bind_me...)`| | [Fn_execute.cpp](lib/tdb/functors/examples/Fn_execute.cpp) |
`tdb::Fn_insert` 	    |`Rowid<Tag_xxx> fn(bind_me...)`| |[Fn_insert.cpp](lib/tdb/functors/examples/Fn_insert.cpp) |
`tdb::Fn_get_value_unique`|`T fn(bind_me...)`| |[Fn_get_value.cpp](lib/tdb/functors/examples/Fn_get_value.cpp) |
`tdb::Fn_get_value_optional`|`std::optional<T> fn(bind_me...)`| |[Fn_get_value.cpp](lib/tdb/functors/examples/Fn_get_value.cpp) |
`tdb::Fn_get_row_unique`|`std::tuple<Return_t...> fn(bind_me...)`| |[Fn_get_row.cpp](lib/tdb/functors/examples/Fn_get_row.cpp) |	
`tdb::Fn_get_row_optional`|`std::optional<std::tuple<Retunr_t...> > fn(bind_me...)`| |[Fn_get_row.cpp](lib/tdb/functors/examples/Fn_get_row.cpp) |
`tdb::Fn_get_column`|`std::vector<T> write_here; fn(std::back_inserter(write_here) , bind_me... );`| |[Fn_get_column.cpp](lib/tdb/functors/examples/Fn_get_column.cpp) |	
`tdb::Fn_get_table`|`std::vector<std::tuple<...> > write_here; fn(std::back_inserter(write_here) , bind_me... )`| |[Fn_get_table.cpp](lib/tdb/functors/examples/Fn_get_table.cpp) |	
`tdb::Fn_foreach`|`void_or_bool fn([](...){}, bind_me... )`| |[Fn_foreach.cpp](lib/tdb/functors/examples/Fn_foreach.cpp) |	
`tdb::Fn_function`|`void_or_bool fn(bind_me... )`|`Function_t`| [Fn_function.cpp](lib/tdb/functors/examples/Fn_function.cpp) |


(1) All functors have the following template parameters
  - Tag_t The tag that identifies the database driver type (ex tdb::Tag_sqlite)
  - Return_tt Returned types ex std::tuple<int,double> (no const, no reference)
  - Bind_tt Bounded types ex std::tuple<int,int> (no const, no reference)
  - Multi_thread (default=true)
    - true : locks the connection mutex, do stuff, unlock. 
    - false : do stuff without touching mutex
  - Extra_t...Extra template args specific to each functor
  
(2) The generated functor prototype. Note that void_or_bool note either void when the functor passed in extra have an operator() that returns void, or bool when the functor passed in extra have an operator() that returns bool



# Write a tdb database driver (draft).
Database drivers are programmed trough template specialisation. Each specialized templates are named Xxxx_t, and takes as the first template argument a Tag_xxxx that identified the driver the specialization refers to. 

Examples are avaiable in [tdb_psql.hpp](lib/tdb/tdb_psql.hpp) and [tdb_sqlite.hpp](lib/tdb/tdb_sqlite.hpp). In order to be consistent, your drivers should go in lib/tdb/tdb_xxxx.hpp, and the xxxx suffix must be the same as the one in Tag_xxxx

## Create a custom tag (required)
Your Tag is an empty class in the tdb namespace. For code consistency, your tag should be named ``Tag_xxxx`` and must be in the tdb namespace.
```cpp
namespace tdb{
	struct Tag_mydriver{};
}
```

## Define the type for Rowid (required)
The Rowid it the type that identifies the lines in the database, it's typically an integer.
```cpp
template<> struct tdb::Rowid_t<tdb::Tag_psql>{
  typedef !MyIntType type;   //ex MyIntType = long unsigend int
};
```







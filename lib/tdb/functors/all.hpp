#ifndef LIB_TDB_FUNCTORS_ALL_HPP_
#define LIB_TDB_FUNCTORS_ALL_HPP_

//All functors uses the same template parameters
// 1 : tdb::Tag_xxx   : the tag that identifies the database (ex tdb::Tag_sqlite)
// 2 : Return_tt      : returned types ex std::tuple<int,double> [1]
// 3 : Bind_tt        : binded types ex std::tuple<int,char> [1]
// 4 : Multi_thread   : a boolean, default = true
//                      - true  : the function lock the connection mutex, do stuff, unlock, returns
//                      - false : the function don't lock anything
// 5+: Extra          : Extra template args specific to each Functor

// [1] : - we expect here the unqualified types (no const, no references).
//       - In order to stay consistent, we ALWAYS expect a std::tuple here,
//         even if there is no (std::tuple<>) or a single (std::tuple<xxxx>)
//         parameter


//--- insert, execute ---
#include "Fn_insert.hpp"   //Rowid<Tag_xxx> fn(bind_me...)
#include "Fn_execute.hpp"  //void           fn(bind_me...)

//--- Get a unique value ---
#include "Fn_get_value_unique.hpp"     //T                 fn(bind_me...)
#include "Fn_get_value_optional.hpp"   //std::optional<T>  fn(bind_me...)

//--- Get a row in a tuple ---
#include "Fn_get_row_unique.hpp"     //std::tuple<Retunr_t...>                  fn(bind_me...)
#include "Fn_get_row_optional.hpp"   //std::optional<std::tuple<Retunr_t...> >  fn(bind_me...)

//--- apply a functor to all/some lines ---
#include "Fn_foreach.hpp"    //void_or_bool   fn([](...){}, bind_me... );
#include "Fn_function.hpp"   //void_or_bool   fn(           bind_me... );

//TODO a non blocking version,
//- that encapsulate functors into jobs.
//- that moves jobs into a thread pool

//--- insert into a container ---
#include "Fn_get_column.hpp" //std::vector<T> write_here                ;fn(std::back_inserter(write_here) , bind_me... );
#include "Fn_get_table.hpp"  //std::vector<std::tuple<...> > write_here ;fn(std::back_inserter(write_here) , bind_me... );


#endif /* LIB_TDB_FUNCTORS_ALL_HPP_ */

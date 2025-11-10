#ifndef LIB_TDB_FUNCTORS_FN_CREATE_HPP_
#define LIB_TDB_FUNCTORS_FN_CREATE_HPP_

//Execute code at construction.
//This is where create queries must go, in order to enforce the creation of tables before other queries.

#include "../tdb.hpp"

namespace tdb{


    template<typename Tag_t,  typename Return_tt, typename Bind_tt, bool Multi_thread>
	struct Fn_create;

	template<typename Tag_t,  typename... Return_a, typename... Bind_a>
	struct Fn_create<Tag_t,  std::tuple<Return_a...> , std::tuple<Bind_a...> , false>{

		typedef std::tuple<Return_a...> Return_tt;
		typedef std::tuple<Bind_a...>   Bind_tt;
		static_assert(std::tuple_size<Return_tt>::value==0,"Fn_create Return_tt must be std::tuple<> (as execute returns nothing)");

		//movable (do nothing), NOT copiable
		Fn_create(Fn_create&&){}
		Fn_create(const Fn_create&)=delete;
		Fn_create& operator=(const Fn_create&)=delete;

		template<typename Sql_tt>
		explicit Fn_create(Connection_t<Tag_t>& db, const Sql_tt &sql,  const Bind_a&... bind_me){
            tdb::execute_a(db,sql,bind_me...);
		}

	};



	template<typename Tag_t,  typename... Return_a, typename... Bind_a>
	struct Fn_create<Tag_t,  std::tuple<Return_a...> , std::tuple<Bind_a...> , true>{

		typedef std::tuple<Return_a...> Return_tt;
		typedef std::tuple<Bind_a...>   Bind_tt;
		static_assert(std::tuple_size<Return_tt>::value==0,"Fn_create Return_tt must be std::tuple<> (as insert returns nothing)");

		//movable (do nothing), NOT copiable
		Fn_create(Fn_create&&){}
		Fn_create(const Fn_create&)=delete;
		Fn_create& operator=(const Fn_create&)=delete;

		template<typename Sql_tt>
		explicit Fn_create(Connection_t<Tag_t>& db, const Sql_tt &sql,  const Bind_a&... bind_me){
			auto l = impl::connection_lock_guard (db);
            tdb::execute_a(db,sql,bind_me...);
		}
	};
}//end namesapce tdb





#endif /* LIB_TDB_FUNCTORS_FN_CREATE_HPP_ */

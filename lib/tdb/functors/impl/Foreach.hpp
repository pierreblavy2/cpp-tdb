#ifndef LIB_TDB_FUNCTORS_IMPL_FOREACH_HPP_
#define LIB_TDB_FUNCTORS_IMPL_FOREACH_HPP_

#include <utility>


namespace tdb::impl{

	template<bool Multi_thread>
	struct Foreach;

	//--- single thread version ---
	template<>	struct Foreach<false>{
		Foreach()=delete;

		template<typename Fn_t, typename Query_t, typename... Bind_a>
		static bool foreach_bool(Query_t &q, Fn_t fn, const Bind_a&... bind_me){
			auto result = tdb::get_result_a(q, bind_me...);
			while( auto r = try_fetch(result) ){
				bool b = std::apply(fn,r.value());
				if(!b){return false;}
			}
			return true;
		}

		template<typename Fn_t, typename Query_t, typename... Bind_a>
		static void foreach_void(Query_t &q, Fn_t fn, const Bind_a&... bind_me){
			auto result = tdb::get_result_a(q, bind_me...);
			while( auto r = try_fetch(result) ){
				std::apply(fn,r.value());
			}
		}
	};


	//--- multi thread version ---
	template<>	struct Foreach<true>{
		Foreach()=delete;

		template<typename Fn_t, typename Query_t, typename Db_t, typename... Bind_a>
		static bool foreach_bool(Db_t &db, Query_t &q, Fn_t fn, const Bind_a&... bind_me){
			auto l = impl::connection_lock_guard (db);
			auto result = tdb::get_result_a(q, bind_me...);
			while( auto r = try_fetch(result) ){
				bool b = std::apply(fn,r.value());
				if(!b){return false;}
			}
			return true;
		}

		template<typename Fn_t, typename Query_t, typename Db_t, typename... Bind_a>
		static void foreach_void(Db_t &db, Query_t &q,  Fn_t fn, const Bind_a&... bind_me){
			auto l = impl::connection_lock_guard (db);
			auto result = tdb::get_result_a(q, bind_me...);
			while( auto r = try_fetch(result) ){
				std::apply(fn,r.value());
			}
		}
	};
}




#endif /* LIB_TDB_FUNCTORS_IMPL_FOREACH_HPP_ */

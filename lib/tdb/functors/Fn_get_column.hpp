#ifndef LIB_TDB_FUNCTORS_FN_INSERT_COLUMN_HPP_
#define LIB_TDB_FUNCTORS_FN_INSERT_COLUMN_HPP_


#include "../tdb.hpp"
#include "impl/is_iterator.hpp"
#include <container/container.hpp>

namespace tdb{

template<typename Tag_t,  typename Return_tt, typename Bind_tt, bool Multi_thread>
	struct Fn_get_column;

	template<typename Tag_t,  typename Return_tt_, typename... Bind_a>
	struct Fn_get_column<Tag_t, Return_tt_, std::tuple<Bind_a...> , false >{

		typedef Return_tt_ Return_tt;
		typedef std::tuple<Bind_a...> Bind_tt;
		typedef typename std::tuple_element<0,Return_tt>::type el0_t;
		static_assert(std::tuple_size<Return_tt>::value == 1, "Fn_get_column expect a single value in Return_tt");


		//movable, NOT copiable
		Fn_get_column(Fn_get_column&&)=default;
		Fn_get_column(const Fn_get_column&)=delete;
		Fn_get_column& operator=(const Fn_get_column&)=delete;
		Fn_get_column()=delete;

		template<typename... A>
		Fn_get_column(Connection_t<Tag_t>& db, A&& ... a ){
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (db,q,s);
		}

		//output_iterator
		template<typename Write_here_tt>
		typename std::enable_if<tdb::impl::is_iterator<Write_here_tt>,void>::type
		operator()(Write_here_tt write_here, const Bind_a&... bind_me){
	    	static_assert(tdb::impl::is_iterator_of_type<Write_here_tt,std::output_iterator_tag>,"Wrong iterator type in Fn_get_column, an output iterator is required.");

			auto result = tdb::get_result_a(q,bind_me...);
			while( auto r = try_fetch(result) ){
			   (*write_here)=std::get<0>(r.value());
			}
			    	//container::add_anywhere(write_here,std::get<0>(r.value()));
		}

		//containers
		template<typename Write_here_tt>
		typename std::enable_if<! tdb::impl::is_iterator<Write_here_tt>,void>::type
		operator()(Write_here_tt &write_here, const Bind_a&... bind_me){
	    	static_assert(container::Add_anywhere_t<Write_here_tt>::is_implemented,"Missing implementation of container::Add_anywhere_t (did you forget to include container/xxx.hpp?)");

			auto result = tdb::get_result_a(q,bind_me...);
			while( auto r = try_fetch(result) ){
				container::add_anywhere(write_here,std::get<0>(r.value()));
			}

		}



		Query<Tag_t,Return_tt,Bind_tt > q;
	};

	template<typename Tag_t,  typename Return_tt_, typename... Bind_a>
	struct Fn_get_column<Tag_t, Return_tt_, std::tuple<Bind_a...> , true >{

		typedef Return_tt_ Return_tt;
		typedef std::tuple<Bind_a...> Bind_tt;
		typedef typename std::tuple_element<0,Return_tt>::type el0_t;
		static_assert(std::tuple_size<Return_tt>::value == 1, "Fn_get_column expect a single value in Return_tt");

		//movable, NOT copiable
		Fn_get_column(Fn_get_column&&)=default;
		Fn_get_column(const Fn_get_column&)=delete;
		Fn_get_column& operator=(const Fn_get_column&)=delete;
		Fn_get_column()=delete;

		template<typename... A>
		Fn_get_column(Connection_t<Tag_t>& db_, A&& ... a ):db(db_){
			auto l = impl::connection_lock_guard (db);
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (q, db,s);
		}


		//output_iterator
		template<typename Write_here_tt>
		typename std::enable_if<tdb::impl::is_iterator<Write_here_tt>,void>::type
		operator()(Write_here_tt write_here, const Bind_a&... bind_me){
	    	static_assert(tdb::impl::is_iterator_of_type<Write_here_tt,std::output_iterator_tag>,"Wrong iterator type in Fn_get_column, an output iterator is required.");

			auto l = impl::connection_lock_guard (db);

			auto result = tdb::get_result_a(q,bind_me...);
			while( auto r = try_fetch(result) ){
			   (*write_here)=std::get<0>(r.value());
			}
			    	//container::add_anywhere(write_here,std::get<0>(r.value()));
		}

		//container
		template<typename Write_here_tt>
		typename std::enable_if<! tdb::impl::is_iterator<Write_here_tt>,void>::type
		operator()(Write_here_tt &write_here, const Bind_a&... bind_me){
	    	static_assert(container::Add_anywhere_t<Write_here_tt>::is_implemented,"Missing implementation of container::Add_anywhere_t (did you forget to include container/xxx.hpp?)");

			auto l = impl::connection_lock_guard (db);

			auto result = tdb::get_result_a(q,bind_me...);
			while( auto r = try_fetch(result) ){
				container::add_anywhere(write_here,std::get<0>(r.value()));
			}

		}





		Connection_t<Tag_t>& db;
		Query<Tag_t,Return_tt,Bind_tt > q;
	};


}//end namespace tdb




#endif /* LIB_TDB_FUNCTORS_FN_INSERT_COLUMN_HPP_ */

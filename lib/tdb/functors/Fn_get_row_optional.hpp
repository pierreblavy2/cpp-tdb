#ifndef LIB_TDB_FUNCTORS_FN_ROW_OPTIONAL_HPP_
#define LIB_TDB_FUNCTORS_FN_ROW_OPTIONAL_HPP_

#include "../tdb.hpp"
/*
  tdb::Fn_get_row_optional<
	  tdb::Tag_psql ,
	  std::tuple<int,int>,
	  std::tuple<double,double>,
      true
	> fn_row_optional(connection, "select max(i1), max(i2) from test where d1 != $1 and d2 != $2");
    std::optional<std::tuple<int,int> > fn_row_optional_r =  fn_row_optional(1.0,1.0);
	std::cout << "Fn_row_unique ok"<<std::endl;
 */
namespace tdb{

template<typename Tag_t,  typename Return_tt, typename Bind_tt, bool Multi_thread>
	struct Fn_get_row_optional;

	template<typename Tag_t,  typename Return_tt_, typename... Bind_a>
	struct Fn_get_row_optional<Tag_t, Return_tt_, std::tuple<Bind_a...> , false >{

		typedef Return_tt_ Return_tt;
		typedef std::tuple<Bind_a...> Bind_tt;

		//movable, NOT copiable
		Fn_get_row_optional(Fn_get_row_optional&&)=default;
		Fn_get_row_optional(const Fn_get_row_optional&)=delete;
		Fn_get_row_optional& operator=(const Fn_get_row_optional&)=delete;
		Fn_get_row_optional()=delete;

		template<typename... A>
		Fn_get_row_optional(Connection_t<Tag_t>& db, A&& ... a ){
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (db,q,s);
		}

		std::optional<Return_tt> operator()( const Bind_a&... bind_me){
			auto result = tdb::get_result_a(q, bind_me...);
			auto l1 = tdb::try_fetch(result);
			if(!l1.has_value()){return l1;}

			auto l2 = tdb::try_fetch(result);
			if(l2.has_value()){throw std::runtime_error("Error in Fn_value_optional : more than 1 line"); }
			return l1;

		}

		Query<Tag_t,Return_tt,Bind_tt > q;
	};

	template<typename Tag_t,  typename Return_tt_, typename... Bind_a>
	struct Fn_get_row_optional<Tag_t, Return_tt_, std::tuple<Bind_a...> , true >{

		typedef Return_tt_ Return_tt;
		typedef std::tuple<Bind_a...> Bind_tt;

		//movable, NOT copiable
		Fn_get_row_optional(Fn_get_row_optional&&)=default;
		Fn_get_row_optional(const Fn_get_row_optional&)=delete;
		Fn_get_row_optional& operator=(const Fn_get_row_optional&)=delete;
		Fn_get_row_optional()=delete;

		template<typename... A>
		Fn_get_row_optional(Connection_t<Tag_t>& db_, A&& ... a ):db(db_){
			auto l = impl::connection_lock_guard (db);
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (q, db,s);
		}

		std::optional<Return_tt> operator()( const Bind_a&... bind_me){
			auto l = impl::connection_lock_guard (db);
			auto result = tdb::get_result_a(q, bind_me...);
			auto l1 = tdb::try_fetch(result);
			if(!l1.has_value()){return l1;}

			auto l2 = tdb::try_fetch(result);
			if(l2.has_value()){throw std::runtime_error("Error in Fn_value_optional : more than 1 line"); }
			return l1;
		}

		Connection_t<Tag_t>& db;
		Query<Tag_t,Return_tt,Bind_tt > q;
	};


}//end namespace tdb




#endif /* LIB_TDB_FUNCTORS_FN_ROW_OPTIONAL_HPP_ */

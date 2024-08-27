#ifndef LIB_TDB_TDB_HPP_
#define LIB_TDB_TDB_HPP_

//--- Base code to make tdb work ---
//Xxxx_t is a template interface (generally for user specialisation)
//Xxxx   is stuff that should be called to use the database

#include <memory>
#include <mutex>
#include <stdexcept>
#include <optional>
#include <string>
#include <sstream>

#include <filesystem>
#include <fstream>

#include "helpers/tuple_ref.hpp"

namespace tdb{


    //=============
    //=== types ===
    //=============

	//--- rowid (required) ---
	template<typename Tag_t> struct Rowid_t;

	template<typename Tag_t> using Rowid = typename Rowid_t<Tag_t>::type;

	//--- Null (don't touch) ---
    struct Null{};
    //This value represent the sql NULL value.
    // Bind does nothing (skip to next column)
    // Get  does nothing (skip to nex column)

    //For values that may be NULL sometimes but not always, use std::optional



    //==================
    //=== Exceptions ===
    //==================
	struct Exception_base:std::runtime_error{typedef std::runtime_error Base_t; using Base_t::Base_t;};
	template<typename Tag_t> struct Exception_t:Exception_base{typedef Exception_base Base_t; using Base_t::Base_t;};



	//===============
	//=== connect ===
	//===============

	//Connection_t (required)
	template<typename Tag_t>	struct Connection_t{
		 static constexpr bool is_implemented = false;
	};

	//Connect_t (optional)
	template<typename Tag_t>
	struct Connect_t{
		private:
		static_assert(Connection_t<Tag_t>::is_implemented,"Connection_t<Tag_t> must be implemented");

		public:
		static constexpr bool is_implemented = true;

		template<typename... Args>
		static Connection_t<Tag_t> run(Args&&... a){
			static constexpr bool is_constructible = std::is_constructible<Connection_t<Tag_t>,Args...>::value;
			static_assert(is_constructible, "The default implementation of Connect_t require a constructor of Connection_t(Args...)");
			return   Connection_t<Tag_t>  (std::forward<Args>(a)...);
		}
	};

	//User interface (don't touch)
	template<typename Tag_t, typename...Args>
	Connection_t<Tag_t> connect(Args&&... a ){
		return Connect_t<Tag_t>::run(std::forward<Args>(a)...);
	}


	//===========
	//=== sql ===
	//===========

	//Store unprepared SQL queries (optional, default store in a string)
    template<typename Tag_t> struct SqlData_t{
    	static constexpr bool is_implemented = true;

    	//recomended (but user is allowed to construct from anything)
    	explicit SqlData_t(const std::string &s):sql(s){};
    	explicit SqlData_t(const char * s):sql(s){};


    	//required
    	//copiable
    	SqlData_t(SqlData_t&)=default;
    	SqlData_t(const SqlData_t&)=default;

    	SqlData_t& operator=(const SqlData_t&)=default;
    	SqlData_t& operator=(SqlData_t&&)=default;

    	const std::string &to_string ()const{return sql;}
    	const char        *c_str     ()const{return sql.c_str();}


    	private:
    	std::string sql;
    };

    //Construct SqlData_t (optional, default forward to SqlData_t constructor)
    template<typename Tag_t, typename... Args> struct Sql_t{
    	private:
    	static_assert(SqlData_t<Tag_t>::is_implemented,"SqlData_t<Tag_t> must be implemented");


    	public:
		static constexpr bool is_implemented=true;
		static SqlData_t<Tag_t> run(Args&&... a){
			static constexpr bool is_constructible =std::is_constructible<SqlData_t<Tag_t>,Args...>::value;
			static_assert(is_constructible, "The default implementation of Sql_t require a constructor of SqlData_t(Args...)");
			return   SqlData_t<Tag_t>  (std::forward<Args>(a)...);
		}
    };

    //Debug a SqlData_t (optional, default return debug_me.to_string())
    template<typename Tag_t> struct Sql_debug_t{
    	static const std::string & run(const SqlData_t<Tag_t> &s){return s.to_string();}
    };

    //--- user interface (don't touch)
    template<typename Tag_t, typename...Args>
    SqlData_t<Tag_t> sql(Args&&... a ){
		return Sql_t<Tag_t,Args...>::run(std::forward<Args>(a)...);
	}

    template<typename Tag_t>
	decltype(auto) sql_debug( const SqlData_t<Tag_t> & debug_me){
		return Sql_debug_t<Tag_t>::run(debug_me);
	}


	//===============
	//=== prepare ===
	//===============
	//SqlData + connection => query

    //Stores a prepared query
	//Note : some db can prepare queries without knowing the types that will
	//be bound latter on, and some cannot. As C++ knows types the tdb interface
    //requires you to ALWAYS provide types that should be used (Return_tt,Bind_tt)

    //With tdb, you cannot bind one by one, therefore
    //DO NOT WRITE : bind(query, a_int); bind(query, a_double);
    //WRITE          bind(query, a_int, a_double);
    //This constraint enforce the correctness of bindings. If for some reason
    //you need to bind in an multi step way, use a tuple
    //WRITE : std::tuple<int,double> r;
    //        std::get<0>(r)=a_int; std::get<1>(r)=a_double;
    //        bind(query, r);


    //--- Query_t (required) ---
    //Return_tt : a std::tuple<...> conaining the types of the columns
    //    returned by the query
    //Bind_tt : a std::tuple<...> conaining the types bounded to the query
    template<typename Tag_t, typename Return_tt, typename Bind_tt>
    struct Query_t{
    	static constexpr bool is_implemented = false;
    };

    template<typename Tag_t, typename Return_tt, typename Bind_tt>
    using Query = Query_t<Tag_t,Return_tt,Bind_tt>;





    //--- Prepare_t (optional) ---
    //Prepare a query
    //default forward (Connection&, SqlData&) to Query_t constructor




    template<typename Tag_t, typename Return_tt, typename Bind_tt>
    struct Prepare_t{
    	typedef  Query_t<Tag_t,Return_tt,Bind_tt> Query_type;

    	static_assert(SqlData_t   <Tag_t>::is_implemented, "SqlData_t<Tag_t> must be implemented");
    	static_assert(Connection_t<Tag_t>::is_implemented, "Connection_t<Tag_t> must be implemented");
    	static_assert(Query_type         ::is_implemented, "Query_t<Tag_t> must be implemented");

		static constexpr bool is_constructible =std::is_constructible< Query_type , Connection_t<Tag_t>&, const SqlData_t<Tag_t>& >::value;
		static_assert(is_constructible, "The default implementation of Prepare_t require a constructor of Query_t( Connect_t<Tag_t>&, const SqlData_t<Tag_t>& )");

		static constexpr bool is_default_constructible =std::is_constructible< Query_type>::value;
		static_assert(is_default_constructible, "The default implementation of Prepare_t require a default constructor of Query_t()");

    	public:
		static constexpr bool is_implemented = true;

		static Query_type run(Connection_t<Tag_t> &db, const SqlData_t<Tag_t> &sql){
			return   Query_type(db,sql);
		}

		static void  run(Query_type &here, Connection_t<Tag_t> &db, const SqlData_t<Tag_t> &sql){
			Query_type q(db,sql);
			std::swap(q,here);
		}


    };

    //prepare (don't touch)
    namespace impl{
    	template<typename T>
    	struct Void_to_empty_tuple{
    		typedef T type;
    	};

    	template<>
    	struct Void_to_empty_tuple<void>{
			typedef std::tuple<> type;
		};
    }


    //prepare_xxx (don't touch)
    //step1 convert args to SqlData_t
    //step2 use Prepare_t

    template<
	  typename Return_tt=std::tuple<>, //required
	  typename Bind_tt  =std::tuple<>, //required
	  typename Tag_t, //deduced
	  typename...A    //deduced
	>
    Query_t<Tag_t,Return_tt,Bind_tt> prepare_new(
    		Connection_t<Tag_t>& db,
			A&&... prepare_from
    ){
    	//use void as a shortcut for empty tuples
    	typedef typename impl::Void_to_empty_tuple<Return_tt>::type Return_t2;
    	typedef typename impl::Void_to_empty_tuple<Bind_tt  >::type Bind_t2;

    	static_assert(std::is_constructible<SqlData_t<Tag_t>, A...>::value,"SqlData_t must be constructible from A...");

    	const SqlData_t<Tag_t>sql(std::forward<A>(prepare_from)...); //step1
    	return Prepare_t<Tag_t,Return_t2,Bind_t2>::run(db,sql);      //step2
    }

    template<
	  typename Return_tt=std::tuple<>, //deduced
	  typename Bind_tt  =std::tuple<>, //deduced
	  typename Tag_t, //deduced
	  typename...A    //deduced
	>
    void prepare_here(
    		Connection_t<Tag_t>& db,
			Query_t<Tag_t,Return_tt,Bind_tt> &q,
			A&&... prepare_from
    ){
    	//use void as a shortcut for empty tuples
    	typedef typename impl::Void_to_empty_tuple<Return_tt>::type Return_t2;
    	typedef typename impl::Void_to_empty_tuple<Bind_tt  >::type Bind_t2;

    	static_assert(std::is_constructible<SqlData_t<Tag_t>, A...>::value,"SqlData_t must be constructible from A...");

    	const SqlData_t<Tag_t>sql(std::forward<A>(prepare_from)...); //step1
    	return Prepare_t<Tag_t,Return_t2,Bind_t2>::run(q,db,sql);    //step2
    }

    template<
	  typename Return_tt=std::tuple<>, //deduced
	  typename Bind_tt  =std::tuple<>, //deduced
	  typename Tag_t, //deduced
	  typename...A    //deduced
	>
    void prepare_here(
			Query_t<Tag_t,Return_tt,Bind_tt> &q,
    		Connection_t<Tag_t>& db,
			A&&... prepare_from
    ){
    	prepare_here(db,q,std::forward<A>(prepare_from)...);
    }


	//============
	//=== bind ===
	//============

    //--- Bind_one_t (depends, but right place to do things) ---
    //These helpers are used, to recursively bind all arguments one after one
    //Bind_one_t works correctly with the default implementation of Bind_t

    //Bind_t binds ALL the arguments that are required by the query. This is the
    //proper way to bind arguments. Bind_one_t is just a way to easily implement
    //Bind_t.



    template<typename Tag_t, typename T,  size_t I, typename is_enabled=void>
    struct Bind_one_t;//Bind the Ith argument which is of type T



    //--- Bind_r (don't touch) ---
    //helper, recursively bind args
    /*
    namespace helpers{

		template <typename Tag_t, typename Bind_tt, size_t I>
		struct Bind_r{

			template<typename Return_tt, typename Bind_t2>
			static void run(Query_t<Tag_t,Return_tt,Bind_tt> &q, const Bind_t2 &bind_me){
				Bind_r<Tag_t,Bind_tt,I-1>::run(q,bind_me);
				typedef typename std::tuple_element<I-1,Bind_tt>::type el_t;
				Bind_one_t<Tag_t,el_t,I-1>::run(q,std::get<I-1>(bind_me));
			}
		};
		template <typename Tag_t, typename Bind_tt>
		struct Bind_r<Tag_t,Bind_tt,0>{
			template<typename Return_tt,typename Bind_t2>
			static void run(Query_t<Tag_t,Return_tt,Bind_tt> &, const Bind_t2&){}
		};

		template <
		    typename Tag_t,     //deduced
			typename Return_tt, //deduced
			typename Bind_tt,   //deduced
			typename Bind_t2    //deduced
	    >
		void bind_r(Query_t<Tag_t,Return_tt,Bind_tt> &q, const Bind_t2& bind_me){
			Bind_r<Tag_t,Bind_tt,std::tuple_size<Bind_tt>::value> ::run(q,bind_me);
        }
    }*/



    //--- bind, bind_a (don't touch) ---
    //bind  (query, tuple_to_bind)
    //bind_a(query, arg_to_bind1, ...);

    namespace helpers{
      template <size_t I, typename Tag_t,typename Return_tt, typename Bind_tt>
      void bind_ra(Query_t<Tag_t, Return_tt, Bind_tt >&){}

      template <size_t I, typename Tag_t,typename Return_tt, typename Bind_tt, typename T, typename...A >
      void bind_ra(Query_t<Tag_t, Return_tt, Bind_tt >& q, const T&t, const A& ... a){
           tdb::Bind_one_t<Tag_t,T,I>::run(q,t);
           bind_ra<I+1>(q,a...);
      }


      template <size_t I, typename Tag_t,typename Return_tt, typename Bind_tt, typename Bind_tt2 >
      void bind_rt(Query_t<Tag_t, Return_tt, Bind_tt >& q, Bind_tt2&& bind_tt){
          if constexpr(I < std::tuple_size_v<Bind_tt> ){
              typedef std::remove_const_t<std::remove_reference_t<std::tuple_element_t<I,std::remove_reference_t<Bind_tt2>>>> el_t;
              tdb::Bind_one_t<Tag_t,el_t,I>::run( q,std::get<I>(bind_tt) );
              bind_rt<I+1>(q,std::forward<Bind_tt2>(bind_tt));
          }
      }
    }

    //bind_a (don't touch)
    //bind arguments
    template <typename Tag_t,typename Return_tt, typename Bind_tt, typename...A >
    void bind_a(Query_t<Tag_t, Return_tt, Bind_tt >& q, const A&... bind_me){
    	static_assert(std::tuple_size<Bind_tt>::value == sizeof...(bind_me), "Error in bind_a : wrong number of arguments");
        //Bind_t<Tag_t,Return_tt,Bind_tt >::run(q, std::tie(bind_me...) );
        helpers::bind_ra<0>(q,bind_me...);
    }

    //bind a tuple containing ALL arguments
    template <typename Tag_t,typename Return_tt, typename Bind_tt, typename Bind_tt2>
    void bind(Query_t<Tag_t, Return_tt, Bind_tt >& q, Bind_tt2&& bind_me){

        static_assert(std::tuple_size<std::remove_reference_t<Bind_tt> >::value == std::tuple_size<std::remove_reference_t<Bind_tt2>>::value, "Error in bind : wrong number of arguments");
        //Bind_t<Tag_t,Return_tt,Bind_tt >::run(q, std::tie(bind_me...) );
        helpers::bind_rt<0>(q,std::forward<Bind_tt2>(bind_me));
    }


    //--- Bind_t (optional) ---

    //bind Null (optional)
    //default do nothing, NO default, as some DB require to increment a bind counter.
    //don't forget to specialize if you need to increment a column counter
    /*template<typename Tag_t, size_t I>
    struct Bind_one_t<Tag_t, tdb::Null, I>{
    	template<typename Return_tt, typename Bind_tt>
    	static void run(Query_t<Tag_t,Return_tt, Bind_tt>&q , const tdb::Null &t){}
    };*/

    //bind std::optional as Null (don't touch)
    template<typename Tag_t, typename T, size_t I>
    struct Bind_one_t<Tag_t, std::optional<T>, I>{
    	template<typename Return_tt, typename Bind_tt>
    	static void run(Query_t<Tag_t,Return_tt, Bind_tt>&q , const std::optional<T> &t){
    		if(t.has_value()){Bind_one_t<Tag_t, T,I>::run(q,t.value());}
    		else             {Bind_one_t<Tag_t,tdb::Null ,I>::run(q,tdb::Null());}
    	}
    };




    //====================
    //=== multi thread ===
    //====================
    template<typename Tag_t>
    struct Get_mutex_t{
    	static constexpr bool is_implemented = false;
    };

    template<typename Tag_t>
    auto & get_mutex(const Connection_t<Tag_t> &c){
    	static_assert(Get_mutex_t<Tag_t>::is_implemented,"You need to implement Get_mutex_t<Tag_t>");
    	return Get_mutex_t<Tag_t>::run(c);
    }


    //implementation helper, don't touch
    namespace impl{
    	template<typename Tag_t>
    	auto connection_lock_guard(const Connection_t<Tag_t> &c){
    		typedef typename std::remove_reference<decltype ( tdb::get_mutex(c) )>::type mutex_t;
    		return std::lock_guard<mutex_t>(tdb::get_mutex(c));
    	}
    }



	//===============
	//=== execute ===
	//===============

    //--- Result ---
    //Results points on a native structure that store a result.
    //This native structure is allowed to do anything, which means that it
    //MAY (very probably) point to the native query
    //Therefore the user MUST ONLY USE one result at a time for a given query

    //Result_t (required)
    //Result has the responsability to buffer rows,
    //current row will be returned as const reference with get_row
    //next_row MUST be called to get a row

    //The typical iteration code is as follow
    //{
    //  auto result = get_result(...);
    //  if constexpr(has_count_row<Tag_xxxx>){/*reserve memory according to count_row(result)*/}
    //  while(next_row(result)){
    //    const auto & row = get_row(result);
    //    do_something(row);
    //  }
    //} //don't forget to delete Result after usage.

    template<typename Tag_t, typename Return_tt> struct Result_t{
    	static constexpr bool is_implemented = false;
    };

    template<typename Tag_t, typename Return_tt>
    using Result=Result_t<Tag_t,Return_tt>;



    //Required : Fetch data.
    //No data   : return an empty std::optional<Return_tt>
    //some data : return the current row in a std::optional<Return_tt>
    //            AND iterate to the next line.
    // Usage:
    // r is a std::optional<std::tuple<...> >
    //while( auto r = try_fetch(result) ){
    //    std::cout << std::get<0>(r.value())<<"\n";
    //}

    template<typename Tag_t, typename Return_tt>
    struct Try_fetch_t{
    	static constexpr bool is_implemented = false;
    };

    //don't touch
    template<typename Tag_t, typename Return_tt>
    std::optional<Return_tt> try_fetch(Result_t<Tag_t,Return_tt> &r){
    	static_assert(Try_fetch_t<Tag_t,Return_tt>::is_implemented,"Try_fetch_t<Tag_t,Return_tt> must be implemented");
    	return Try_fetch_t<Tag_t,Return_tt>::run(r);
    }

    //--- DOC : try_fetch usage ---
    /*auto r = try_fetch(result);
	while(r.has_value() ){
	    const auto row = tdb::get_row(result);
	    std::cout << std::get<0>(row)<<"\n";
	    r = try_fetch(result);
	}*/


    //has_count_row
    //count_row MAY NOT be implemented, as some db don't provide row counting capabilities
    //Caller code MUST use a if constexpr(has_count_row<Tag_t>){...}

    //template<typename Tag_t>
    //struct Has_count_row_t{
    //	static constexpr bool value = false;
    //};



    //template<typename Tag_t, typename Return_tt>
    //constexpr inline bool has_count_row(const Result_t<Tag_t,Return_tt> &){return Has_count_row_t<Tag_t>::value;}

    //Count_row_t (optional),
    //define Has_count_row_t accordingly
    //Count_row return the number of row, in Result WHEN the query was executed
    //next_row DO NOT CHANGE count_row.
    //the main purpose of count_row is to reserve memory for vectors of rows

    template<typename Tag_t, typename Return_tt>
    struct Count_row_t{
    	static constexpr bool is_implemented = false;
    };

    //count_row (don't touch)
    template<typename Tag_t, typename Return_tt>
    Rowid<Tag_t> count_row(const Result_t<Tag_t,Return_tt> &r){
    	static_assert(Count_row_t<Tag_t,Return_tt>::is_implemented,"Count_row_t<Tag_t,Return_tt> must be implemented");
    	return Count_row_t<Tag_t,Return_tt>::run(r);
    }

    template<typename Tag_t,typename Return_tt=void >
    constexpr bool has_count_row = Count_row_t<Tag_t,Return_tt>::is_implemented;


	//===========
	//=== get ===
	//===========

    //--- Get_one_t (depends, but right place to do things) ---
    //These helpers are used, to recursively get all arguments one after one
    //Get_one_t works correctly with the default implementation of Get_t

    //Get_one_t is just an easy way to implement Get_t.


    template<typename Tag_t, typename T,  size_t I, typename is_enabled=void>
    struct Get_one_t;//Get the Ith argument, write it to T




    //--- Get_r (don't touch) ---
    //helper, recursively get
    namespace helpers{
		template <typename Tag_t, typename Return_tt, size_t I>
		struct Get_r{
			template<typename Return_t2>
			static void run(Result_t<Tag_t,Return_tt> &q, Return_t2 &write_here){
				Get_r<Tag_t,Return_tt,I-1>::run(q,write_here);
				typedef typename std::tuple_element<I-1,Return_tt>::type el_t;
				Get_one_t<Tag_t,el_t,I-1>::run(q,std::get<I-1>(write_here));
			}

		};
		template <typename Tag_t, typename Return_tt>
		struct Get_r<Tag_t,Return_tt,0>{
			template<typename Return_t2>
			static void run(Result_t<Tag_t,Return_tt> &, Return_t2 &){}
		};

		template <typename Tag_t, typename Return_tt, typename Return_t2>
		void get_r(Result_t<Tag_t,Return_tt> &q, Return_t2 &write_here){
			static_assert(std::tuple_size<Return_tt>::value == std::tuple_size<Return_t2>::value,"Tuple size mismatch in Get_r (Get_row_t implementation)");
			Get_r<Tag_t,Return_tt,std::tuple_size<Return_tt>::value> ::run(q,write_here);
		}
    }

    //--- Get_row_t (optional) ---
    //The default implementation calls Get_one from the first to the last argument
    template <typename Tag_t,typename Return_tt>
    struct Get_row_t{
    	template<typename Write_here_t>
    	static void run(Result_t<Tag_t, Return_tt>& q, Write_here_t & write_here){
    		helpers::get_r(q,write_here);
    	}
    };

    //get (don't touch)

    template <typename Tag_t,typename Return_tt>
    Return_tt get_row(Result_t<Tag_t, Return_tt>& q){
    	Return_tt  r;
    	Get_row_t<Tag_t,Return_tt>::run(q,r);
    	return r;
    }

    template <typename Tag_t,typename Return_tt, typename Write_here_t>
    void get_row(Result_t<Tag_t, Return_tt>& q, Write_here_t & write_here){
    	Get_row_t<Tag_t,Return_tt>::run(q,write_here);
    }

    template <typename Tag_t,typename Return_tt, typename Bind_tt, typename...A >
    void get_row_a(Result_t<Tag_t, Return_tt>& q, A&... write_here){
    	Get_row_t<Tag_t,Return_tt>::run(q, std::tie(write_here...) );
    }







    //get Null (optional)
    //default do nothing
    //don't forget to specialize if you need to increment a column counter
    template<typename Tag_t, size_t I>
    struct Get_one_t<Tag_t, tdb::Null, I>{
    	template<typename Return_tt, typename Bind_tt>
    	static void run(Result_t<Tag_t,Return_tt>&q , tdb::Null &t){}
    };





    //--- execute ---
    //execute returns void

    //Execute_t (required)
    template<typename Tag_t, typename Return_tt, typename Bind_tt>
    struct Execute_t{
    	static constexpr bool is_implemented = false;
    };

    //execute (don't touch)
    template<typename Tag_t, typename Return_tt>
    void execute(Query<Tag_t,Return_tt,std::tuple<> > &q){
    	static_assert(Execute_t<Tag_t,Return_tt, std::tuple<> >::is_implemented,"Execute_t is not implemented");
    	Execute_t<Tag_t,Return_tt, std::tuple<> >::run(q);
    }

    template<typename Tag_t, typename Sql_t>
    void execute(Connection_t<Tag_t> &c, const Sql_t &sql_t){
    	static_assert(Execute_t<Tag_t,std::tuple<>, std::tuple<> >::is_implemented,"Execute_t is not implemented");

    	auto s = sql<Tag_t>(sql_t);
    	Query_t<Tag_t,std::tuple<>,std::tuple<> > q;
    	prepare_here(q,c,s);
    	execute(q);
    }






    template<typename Tag_t, typename Return_tt, typename Bind_tt, typename Bind_t2>
    void execute(Query<Tag_t,Return_tt,Bind_tt > &q, const Bind_t2 &bind_me){
    	static_assert(Execute_t<Tag_t,Return_tt, Bind_tt >::is_implemented,"Execute_t is not implemented");
    	tdb::bind(q,bind_me);
    	Execute_t<Tag_t,Return_tt, Bind_tt >::run(q);
    }

    template<typename Tag_t, typename Sql_t,  typename Bind_t2>
    void execute(Connection_t<Tag_t> &c, const Sql_t &sql_t, const Bind_t2 &bind_me){
    	typedef impl::tuple_val<Bind_t2> Bind_tt;

    	static_assert(Execute_t<Tag_t,std::tuple<>, Bind_tt >::is_implemented,"Execute_t is not implemented");

    	auto s = sql<Tag_t>(sql_t);
    	Query<Tag_t,std::tuple<>, Bind_tt > q;
    	prepare_here(q,c,s);
    	execute(q, bind_me);
    }




    template<typename Tag_t, typename... A>
    void execute_a(Query<Tag_t,std::tuple<>, std::tuple<A...> > &q, const A&... bind_me){
    	execute(q,std::tie(bind_me...));
    }

    template<typename Tag_t, typename Sql_tt,typename... A>
    void execute_a(Connection_t<Tag_t> &c, const Sql_tt &sql_t, const A&... bind_me){
    	execute(c,sql_t,std::tie(bind_me...));
    }



    //--- read multiple input ---
    //read from input stream, multiple queries are allowed
    //will be default implemented trough ReadString_t that execute MULTIPLE queries
    //Execute_t (required)
    template<typename Tag_t> void read_istream(Connection_t<Tag_t> &q, std::istream &in);
    template<typename Tag_t> void read_string (Connection_t<Tag_t> &q, const std::string &s);
    template<typename Tag_t> void read_file   (Connection_t<Tag_t> &q, const std::filesystem::path &p);


    template<typename Tag_t>
    struct Read_Istream_t{
    	static constexpr bool is_implemented = true;
    	static void run(Connection_t<Tag_t> &q, std::istream &in){
    		//https://stackoverflow.com/questions/116038/what-is-the-best-way-to-slurp-a-file-into-a-stdstring-in-c
    		std::string buffer(static_cast<std::stringstream const&>(std::stringstream() << in.rdbuf()).str());
    		read_string(q,buffer);
    	}
    };

    template<typename Tag_t>
    struct Read_String_t{
    	static constexpr bool is_implemented = false;
    	//static void run(Connection_t<Tag_t> &q, const std::string &in){}
    };


    template<typename Tag_t>
    void read_istream(Connection_t<Tag_t> &q, std::istream &in){
    	static_assert(Read_Istream_t<Tag_t>::is_implemented,"ReadIstream_t is not implemented");
    	Read_Istream_t<Tag_t>::run(q, in);
    }

    template<typename Tag_t>
    void read_string(Connection_t<Tag_t> &q, const std::string &s){
    	static_assert(Read_Istream_t<Tag_t>::is_implemented,"ReadString_t is not implemented");
    	Read_String_t<Tag_t>::run(q,s);
    }

    template<typename Tag_t>
    void read_file(Connection_t<Tag_t> &q, const std::filesystem::path &p){
    	static_assert(Read_Istream_t<Tag_t>::is_implemented,"ReadIstream_t is not implemented");
    	std::ifstream in(p);
    	if(!in){throw std::runtime_error("Cannot open "+p.string());}
    	read_istream(q,in);
    }




    //--- insert ---
    //insert  returns a Rowid

    //Insert_t (required)
    template<typename Tag_t, typename Return_tt, typename Bind_tt>
    struct Insert_t{
    	static constexpr bool is_implemented = false;
    };

    //insert (don't touch)
    template<typename Tag_t>
    Rowid<Tag_t> insert(Query<Tag_t,std::tuple<>,std::tuple<> > &q){
    	static_assert(Insert_t<Tag_t,std::tuple<>, std::tuple<> >::is_implemented,"Insert_t is not implemented");
    	return Insert_t<Tag_t,std::tuple<>, std::tuple<> >::run(q);
    }

    template<typename Tag_t,  typename Bind_tt, typename Bind_t2>
    Rowid<Tag_t> insert(Query<Tag_t,std::tuple<>,Bind_tt > &q, const Bind_t2 &bind_me){
    	static_assert(Insert_t<Tag_t,std::tuple<>, Bind_tt >::is_implemented,"Insert_t is not implemented");
    	tdb::bind(q,bind_me);
    	return Insert_t<Tag_t,std::tuple<>, Bind_tt >::run(q);
    }

    template<typename Tag_t,typename Sql_tt>
    Rowid<Tag_t> insert(Connection_t<Tag_t> &c, const Sql_tt &sql_t){
    	static_assert(Insert_t<Tag_t,std::tuple<>, std::tuple<> >::is_implemented,"Insert_t is not implemented");

    	auto s = sql<Tag_t>(sql_t);
    	Query_t<Tag_t,std::tuple<>,std::tuple<> > q;
    	prepare_here(q,c,s);
    	return insert(q);
    }

    template<typename Tag_t , typename Bind_t2, typename Sql_tt>
    Rowid<Tag_t> insert(Connection_t<Tag_t> &c, const Sql_tt &sql_t, const Bind_t2 &bind_me){
    	typedef impl::tuple_val<Bind_t2> Bind_tt;
    	static_assert(Insert_t<Tag_t,std::tuple<>, Bind_tt >::is_implemented,"Insert_t is not implemented");

    	auto s = sql<Tag_t>(sql_t);
    	Query<Tag_t,std::tuple<>, Bind_tt > q;
    	prepare_here(q,c,s);
    	return insert(q, bind_me);
    }



    template<typename Tag_t>
    Rowid<Tag_t> insert_a(Query<Tag_t,std::tuple<>, std::tuple<> > &q){
        return insert(q,std::tie());
    }

    template<typename Tag_t, typename A1, typename... A>
    Rowid<Tag_t> insert_a(Query<Tag_t,std::tuple<>, std::tuple<A1,A...> > &q, const A1& bind_me1, const A&... bind_me){
    	return insert(q,std::tie(bind_me1,bind_me...));
    }


    //--- get_result ---
    //get_result returns a result

    //Get_result_t (optional, default use Result_t constructor)
    template<typename Tag_t, typename Return_tt, typename Bind_tt>
    struct Get_result_t{
    	static constexpr bool is_implemented = std::is_constructible< Result_t<Tag_t,Return_tt>, Query_t<Tag_t,Return_tt,Bind_tt>>::value;

    	static Result_t<Tag_t,Return_tt> run(Query_t<Tag_t,Return_tt,Bind_tt> &q){
    		//static_assert(is_implemented,"The default implementation of Get_result_t require the constructor Result_t<Tag_t,Return_tt>::Result_t(Query_t<Tag_t,Return_tt> &) ");
    		return Result_t<Tag_t,Return_tt>(q);
    	}

    };

    //get_result (don't touch)
    template<typename Tag_t, typename Return_tt>
    Result<Tag_t,Return_tt> get_result( Query_t< Tag_t,Return_tt,std::tuple<> > &q){
    	return Get_result_t<Tag_t,Return_tt, std::tuple<> >::run(q);
    }

    template<typename Tag_t, typename Return_tt, typename Bind_tt, typename Bind_t2>
    Result<Tag_t,Return_tt> get_result(Query_t<Tag_t,Return_tt,Bind_tt > &q, const Bind_t2 &bind_me){
    	bind(q,bind_me);
    	return Get_result_t<Tag_t,Return_tt, Bind_tt >::run(q);
    }

    template<typename Tag_t, typename Return_tt, typename Bind_tt, typename... A>
    Result<Tag_t,Return_tt> get_result_a(Query<Tag_t,Return_tt,Bind_tt > &q, const A&... bind_me){
    	static_assert(std::tuple_size<Bind_tt>::value == sizeof...(bind_me), "Error in get_result_a : wrong number of arguments");
    	bind_a(q,bind_me...);
    	return Get_result_t<Tag_t,Return_tt, Bind_tt >::run(q);
    }




    //-- get_unique (don't touch)
    //get a unique line, throw if not exactly one line  --
    /*
	template<typename Tag_t, typename Return_tt>
	void get_unique(Query<Tag_t,Return_tt,std::tuple<> > &q){
		 if constexpr(has_count_row<Tag_t>){
			 Result<Tag_t,Return_tt> r = get_result(q);
			 auto row_number = count_row(r);
			 if(r!=1){throw std::runtime_error("Error in get_unique : one row is expected, but result has " + std::to_string(row_number) + " row(s)" );}
			 return;
		 }else{
			Result<Tag_t,Return_tt> r = get_result(q);
			if(!has_row(r)){throw std::runtime_error("Error in get_unique : empty result");}
			next_row(r);
			if(has_row(r)){throw std::runtime_error("Error in get_unique : more than one row");}
			return;
		 }
	}*/


	template<typename Tag_t, typename Return_tt, typename Bind_tt, typename Write_here_tt, typename Bind_t2>
	void get_unique(
			Query<Tag_t,Return_tt,Bind_tt > &q,
			Write_here_tt &write_here ,
			const Bind_t2 &bind_me
	){
		 if constexpr(has_count_row<Tag_t>){
			 Result<Tag_t,Return_tt> r = get_result(q,bind_me);
			 auto row_number = count_row(r);
			 if(row_number!=1){throw std::runtime_error("Error in get_unique : one row is expected, but result has " + std::to_string(row_number) + " row(s)" );}
			 write_here =  get_row(r);
			 return;
		 }else{
			Result<Tag_t,Return_tt> r = get_result(q,bind_me);
			auto row1 = try_fetch(r);
			auto row2 = try_fetch(r);

			if(!row1.has_value()){throw std::runtime_error("Error in get_unique : empty result");}
			if( row2.has_value()){throw std::runtime_error("Error in get_unique : more than one row");}

			write_here=row1.value();
			return;
		 }
	}

	template<typename Tag_t, typename Return_tt,typename Write_here_tt>
	void get_unique(
			Query<Tag_t,Return_tt,std::tuple<>  > &q,
			Write_here_tt &write_here
	){
		get_unique(q,write_here,std::tuple<>() );
	}



    //--- Transaction_t (required, default) ---
    template<typename Tag_t, bool autocommit=false>
    struct Transaction_t{
    	static constexpr bool is_implemented = true;
    	explicit Transaction_t(Connection_t<Tag_t> &db_):db(db_){
    		begin();
    	}

    	~ Transaction_t(){
    		if(is_finished){return;}
    		if constexpr(autocommit){commit();}
    		else                    {rollback();}
    	}

    	Connection_t<Tag_t> &db;
    	bool is_finished=false;

    	void commit()  {is_finished=true; tdb::execute_a(db,"COMMIT");}
    	void rollback(){is_finished=true; tdb::execute_a(db,"ROLLBACK");}

    	private:
    	void begin()   {tdb::execute_a(db,"BEGIN transaction");}
    };

    //transaction (don't touch)
    template<bool autocommit=false, typename Tag_t>
    [[nodiscard]] Transaction_t<Tag_t,autocommit> transaction(Connection_t<Tag_t> &conn){
    	return Transaction_t<Tag_t,autocommit>(conn);
    }



    //--- savepoint (required, default) ---
	template<typename Tag_t, bool autocommit=false>
	struct Savepoint_t{
		static constexpr bool is_implemented = true;
		explicit Savepoint_t(Connection_t<Tag_t> &db_):db(db_){
			name ="s_"+mk_name(this);
			begin();
		}

		~ Savepoint_t(){
			if(is_finished){return;}
			if constexpr(autocommit){commit();}
			else                    {rollback();}
		}

		Connection_t<Tag_t> &db;
		bool is_finished=false;
		std::string name;

		void commit()  {is_finished=true; tdb::execute_a(db,"RELEASE SAVEPOINT " + name);}
		void rollback(){is_finished=true; tdb::execute_a(db,"ROLLBACK TO SAVEPOINT " + name);};

		private:
		void begin()   {tdb::execute_a(db,"SAVEPOINT "+name);}

		static std::string mk_name(void * unique_id){
			std::stringstream ss;
			ss <<"q_" <<  unique_id;
			return ss.str();
		}

	};







}//end namespace tdb



#endif /* LIB_TDB_TDB_HPP_ */

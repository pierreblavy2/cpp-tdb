# Introduction

## Problems of conversion

### Heterogeneity
C++ provides rules for converting between types, trough default conversion operators, static_cast, 
boost::lexical_cast, boost_numeric cast and a bunch of function like std::to_string or atoi or the .c_str() member function. 

### Unextensible
If free function can be overloaded, the use of members functions makes adding new convestions to old classes impossible. 
Morever, overloading functions requires to write an overload for each pair of conversions types, and doesn't  benefit easily of
the power of template programming.

### No context
There's no clear way to choose how things should be converted based on the context. For example, I want to convert 
a bool to "0" or "1" string when I want to store them in a flat file, but to "true" or "false" when I want to display them in
the console, so a unique bool to string conversion will never do the job.

## Solution
The convert library provides an unified, context dependant, extensible way to define conversions.


### Design of the convert library

In order to solve the following problems, the convert library works by defining the following code, with some extra magic to
avoid trivials conversions (ex : convert from T to T).

```cpp
template<typename To_tt, typename From_tt, typename Context_tag_tt>
struct Convert_t{...};

template<typename To_t, typename Context_tag=void, typename From_t> 
decltype(auto) convert(const From_t &f){/*calls Convert_t<To_t,From_t,Context_tag>::run(f)*/}

template<typename Context_tag=void, typename To_t, typename From_t> 
void convert(To_t &write_here, const From_t &f){/*write_here=Convert_t<To_t,From_t,Context_tag>::run(f)*/}
```

- ```From_tt``` is the
- ```To_tt``` is the type to convert to
- ```Context_tag_t``` is the type that describes the context of the conversion. Any empty tag class will to the job.




# Use existing conversions

## without context
This example shows how to convert from const char * (cstring) to a std::string. 
As no context is specified, the Context_tag is defaulted to void, and the default template implementation of Convert_t is used.

```cpp
#include <convert/string_cstring.hpp>
void example(){
  const char * cs = "hello";
  std::string s1 = convert<std::string>(cs);    
  std::string s2;  convert<std::string>(cs,s2); //does the same thing
}
```

## with context
```cpp
  #include <convert/convert.hpp>
  struct MyContext_tag{};
  
  //define the bool to string conversion in MyContext_tag context 
  //(see : define new conversions)
  template<>
  struct Convert_t<std::string, bool, MyContext_tag>{
	typedef std::string To_t;
	typedef bool        From_t;
	typedef MyContext_tag Context_tag_t;

	static To_t run(const From_t &b){
		if(b){return "true";}else{return "false";}
	}
   };
   
   //use it
   void example(){
       bool b = true;
       std::string b_str =  convert<std::string, MyContext_tag>(b);
       std::string b_str2;  convert< MyContext_tag>(b, b_str2); //does the same thing
   }
```


# Define new conversions

## In the default context
To define a default implementation for unspecified or undefined context, simply keep Context_tag_tt as a template parameter, and specialize Convert_t

```cpp
#include <convert/convert.hpp>
  
//default string to bool
template<typename Context_tag_tt>
struct Convert_t<std::string, bool, Context_tag_tt>{
    typedef std::string To_t;
    typedef bool        From_t;
    typedef Context_tag_tt Context_tag_t;

    static To_t run(const From_t &b){
        if(b){return "true";}else{return "false";}
    }
};
```


## In a custom context
To define an implementation for a custom context do a full specialization. Note that the full specialization will take precedence on the parcially specialized one, typically the one with a template Context_tag_tt parameter. Therefore you can mix a default context with any custom context you want.

```cpp
#include <convert/convert.hpp>
struct MyContext_tag{};
  
//MyContext_tag string to bool
template<>
struct Convert_t<std::string, bool, MyContext_tag>{
    typedef std::string   To_t;
    typedef bool          From_t;
    typedef MyContext_tag Context_tag_t;

    static To_t run(const From_t &b){
        if(b){return "1;}else{return "0";}
    }
};
```

## With std::enable_if
The following example uses boost::lexical_cast to convert between string and numbers in MyContext_tag context

```cpp
#include <boost/lexical_cast.hpp>
#include <convert/convert.hpp>
struct MyContext_tag{};


//string => number
template<typename To_tt>
struct Convert_t<
    To_tt,
    std::string,
    typename std::enable_if <
       std::is_integral<To_tt>::value or std::is_floating_point<To_tt>::value, MyContext_tag>
    ::type 
>
{
    typedef To_tt To_t;
    typedef std::string From_t;
    typedef tdb::Tag_psql Context_tag_t;

    static constexpr bool is_implemented = true;

    static To_t run(const From_t &f){
        return boost::lexical_cast<To_t>(f);
    }
};

//number=> string
template<typename From_tt>
struct Convert_t<
    std::string, //to
    From_tt,
    typename std::enable_if <
        std::is_integral<From_tt>::value or std::is_floating_point<From_tt>::value, MyContext_tag
    >::type
>
{
    static constexpr bool is_implemented = true;

    typedef std::string To_t;
    typedef From_tt     From_t;
    typedef tdb::Tag_psql Context_tag_t;

    static To_t run(const From_t &f){
        return boost::lexical_cast<To_t>(f);
    }
};
```


# License
The code is published under [LGPL 3.0](https://www.gnu.org/licenses/lgpl-3.0.txt) license. Copyright goes to Pierre Blavy.




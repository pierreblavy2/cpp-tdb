#include "is_iterator.hpp"
#include <vector>

using namespace tdb::impl;

static_assert(!tdb::impl::is_iterator_of_type<int             ,std::output_iterator_tag>, "");
static_assert(!tdb::impl::is_iterator_of_type<std::vector<int>,std::output_iterator_tag>, "");
typedef std::vector<int> int_v;
typedef std::back_insert_iterator<int_v> it_t;
static_assert( is_iterator_of_type<  it_t ,std::output_iterator_tag>, "");



static_assert(!is_iterator<int>             , "");
static_assert(!is_iterator<std::vector<int>>, "");
typedef std::vector<int> int_v;
typedef std::back_insert_iterator<int_v> it_t;
static_assert( is_iterator<  it_t>, "");


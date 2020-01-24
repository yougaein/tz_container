#include "set_tz"
#include <algorithm>
#include <ruby.h>
#include <new>


using namespace std;
using namespace tz;


void Init_ext_set(void){
	VALUE cESet = rb_define_class("ESet", rb_cObject);
	rb_define_method(cESet, "initialize", eSet_initialize, -1);
	rb_define_method(cESet, "begin", eSet_begin, -1);
	rb_define_method(cESet, "end", eSet_end, -1);
	rb_define_method(cESet, "insert", eSet_insert, -1);

	VALUE cESetIt = rb_define_class("ESetIt", rb_cObject);
	rb_define_method(cESetIt, "inc", eSetIt_inc, -1);
	rb_define_method(cESetIt, "dec", eSetIt_dec, -1);
	rb_define_method(cESetIt, "it", eSetIt_it, -1);

	rb_define_method(cKernel, "find", cKernel_find, -1);
	rb_define_method(cKernel, "erase", cKernel_erase, -1);
	rb_define_method(cKernel, "for_each", cKernel_for_each, -1);
}

struct LessESet{
	bool operator<(VALUE a, VALUE b) const;
}

class ESet{
	set* _s;
	typedef bool (*CompareBy)(VALUE);
	typedef bool (*Less)(VALUE, VALUE);
public:
	ESet() : _s(new set<VALUE>){ }
	ESet(CompareBy f) : _s(new set_by<VALUE>(f)){ }
	ESet(Less f) : _s(new set_less<VALUE>(f)){ }
	~ESet(){ delete _s; }
	bool isLeagal();
}

struct ValueCompareBy{
	VALUE block;
	typedef VALUE InitArgType;
	ValueCompareBy(InitArgType b) : block(b){ }
	bool operator()(VALUE a, VALUE b){
		return *** block.method.call(a) < block.method.call(b);
	}
};


struct ValueLess{
	VALUE block;
	typedef VALUE InitArgType;
	ValueLess(InitArgType b) : block(b){ }
	bool operator()(VALUE a, VALUE b){
		return *** block.method.call(a, b);
	}
};


typedef setbase<VALUE> ESet;
typedef set<VALUE, CompareBy> ESetCompareBy;
typedef set<VALUE, CompareLess> ESetCompareLess;


static void wrap_ESet_free(ESet* p){
	if(p->isLeagal()) p->~ESet();
	ruby_xfree(p);
}


static void wrap_ESet_mark(ESet* p){
	for_each(p->begin(), p->end(), 
		[](VALUE v){
			rb_gc_mark(v);
		}
	);
}

// ESet.new do |a, b|
//   a < b
// end

static VALUE wrap_ESet_alloc(VALUE klass){
	return Data_Wrap_Struct(klass, wrap_ESet_mark, wrap_ESet_free, ruby_xmalloc(sizeof(ESetCompareLess)));
}


static VALUE wrap_ESet_init(VALUE self, VALUE block) {
	ESet* p;
	if(block is no arg){
		Data_Get_Struct(self, ESet, p);
		new(p) ESet();
	}else if(block has one arg){
		Data_Get_Struct(self, ESetBy, p);
		new(p) ESetCompareBy(block);
	}else{
		Data_Get_Struct(self, ESetLess, p);
		new(p) ESetCompareLess(block);
	}
}




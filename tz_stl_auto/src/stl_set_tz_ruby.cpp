/*
 * Copyright (c) 2020
 *
 * Yougaein@github (Kokeji Yougain)
 */

#include "set_tz"
#include <algorithm>
#include <ruby.h>
#include <new>


using namespace std;
using namespace tz;


static ID ID_CALL;
static ID ID_LESS;
static ID ID_EQ;

static VALUE cESet = Qnil;
static VALUE cESetIt = Qnil;

struct ValueCompareBy{
	VALUE block;
	ValueCompareBy(VALUE b) : block(b){ }
	bool operator()(VALUE a, VALUE b){
		if(block != Qnil){
			VALUE ac = rb_funcall(block, ID_CALL, 1, a);
			VALUE bc = rb_funcall(block, ID_CALL, 1, b);
			VALUE res = rb_funcall(ac, ID_LESS, 1, bc);
			return res != Qfalse && res != Qtrue;
		}else{
			return a < b;
		}
	}
};


typedef multiset<VALUE, ValueCompareBy> ESet;


static void eSet_free(ESet* p){
	p->~ESet();
	ruby_xfree(p);
}


static void eSet_mark(ESet* p){
	for(ESet::iterator it = p->begin() ; it != p->end() ; ++it){
		if(&*it != NULL){
			rb_gc_mark(*it);
		}
		
	}
	rb_gc_mark(p->_M_t._M_impl._M_key_compare.block);
}

static void eSetIt_free(ESet::iterator* p){
	p->ESet::iterator::~iterator();
	ruby_xfree(p);
}


static void eSetIt_mark(ESet::iterator* p){
	rb_gc_mark(**p);
	VALUE m = ((ESet*)p->_M_node->_TZ_tree)->_M_t.memo;
	if(m)
		rb_gc_mark(m);
}

// ESet.new do |a, b|
//   a < b
// end

static ESet* eSet(VALUE self){
	ESet* p;
	Data_Get_Struct(self, ESet, p);
	return p;
}


static VALUE eSet_alloc(VALUE klass){
	return Data_Wrap_Struct(klass, eSet_mark, eSet_free, ruby_xmalloc(sizeof(ESet)));
}


static VALUE eSetIt_alloc(VALUE klass){
	return Data_Wrap_Struct(klass, eSetIt_mark, eSetIt_free, ruby_xmalloc(sizeof(ESet::iterator)));
}


static VALUE eSet_init(int argc, VALUE *argv, VALUE self) {
	ESet* p = eSet(self);
	VALUE block;
	rb_scan_args(argc, argv, "0&", &block);
	new(p) ESet(ValueCompareBy(block));
	p->_M_t.memo = self;
	return Qnil;
}


static ESet::iterator* eSetIt(VALUE self){
	ESet::iterator* p;
	Data_Get_Struct(self, ESet::iterator, p);
	return p;
}


static VALUE eSet_begin(VALUE self) {
	ESet* p = eSet(self);
	VALUE itv = eSetIt_alloc(cESetIt);
	ESet::iterator* it = eSetIt(itv);
	*it = p->begin();
	return itv;
}


static VALUE eSet_size(VALUE self) {
	ESet* p = eSet(self);
	return INT2FIX(p->size());
}


static VALUE eSet_end(VALUE self) {
	ESet* p = eSet(self);
	VALUE itv = eSetIt_alloc(cESetIt);
	ESet::iterator* it = eSetIt(itv);
	*it = p->end();
	return itv;
}


static VALUE eSet_insert(int argc, VALUE *argv, VALUE self){
	ESet* p = eSet(self);
	VALUE a1, a2;
	rb_scan_args(argc, argv, "11", &a1, &a2);
	ESet::iterator ret;
	if(a2 == Qnil)
		ret = p->insert(a1);
	else{
		ESet::iterator* rit = eSetIt(a1);
		ret = p->insert(*rit, a2);
	}
	VALUE itv = eSetIt_alloc(cESetIt);
	ESet::iterator* it = eSetIt(itv);
	*it = ret;
	return itv;	
}


static VALUE eSetIt_inc(VALUE self) {
	ESet::iterator* it = eSetIt(self);
	++*it;
	return self;
}


static VALUE eSetIt_dec(VALUE self) {
	ESet::iterator* it = eSetIt(self);
	--*it;
	return self;
}


static VALUE eSetIt_item(VALUE self) {
	ESet::iterator* it = eSetIt(self);
	if(&*it == NULL){
		rb_raise(rb_eRangeError, "Dereferencing the end iterator");
		return Qnil;
	}else{
		return **it;
	}
}


static VALUE eSet_find(int argc, VALUE *argv, VALUE self){
	VALUE b, e, v;

	rb_scan_args(argc, argv, "21", &b, &e, &v);
	ESet::iterator* bg = eSetIt(b), * ed = eSetIt(e);

	if((ESet*)bg->_M_node->_TZ_tree != (ESet*)ed->_M_node->_TZ_tree){
		rb_raise(rb_eArgError, "First and second iterators are not from the same container");
		return Qnil;
	}

	if(v != Qnil){
		if(!rb_block_given_p()){
			for(ESet::iterator it = *bg ; it != *ed ; ++it){
				if(&*it == NULL){
					rb_raise(rb_eRangeError, "Dereferencing the end iterator");
					return Qnil;
				}
				VALUE res = rb_funcall(*it, ID_EQ, 1, v);
				if(res != Qnil && res != Qfalse)
					return *it;
			}
		}else{
			for(ESet::iterator it = *bg ; it != *ed ; ++it){
				if(&*it == NULL){
					rb_raise(rb_eRangeError, "Dereferencing the end iterator");
					return Qnil;
				}
				VALUE res = rb_funcall(*it, ID_EQ, 1, v);
				if(res != Qnil && res != Qfalse){
					res = rb_yield(*it);
					if(res != Qnil && res != Qfalse)
						return *it;
				}
			}
		}
	}else if(rb_block_given_p()){
		for(ESet::iterator it = *bg ; it != *ed ; ++it){
			if(&*it == NULL){
				rb_raise(rb_eRangeError, "Dereferencing the end iterator");
				return Qnil;
			}
			VALUE res = rb_yield(*it);
			if(res != Qnil && res != Qfalse)
				return *it;
		}
	}else{
		rb_raise(rb_eArgError, "Both argument and block are missing");
		return Qnil;
	}
	return e;
}


static VALUE eSet_for_each(VALUE self, VALUE b, VALUE e) {
	if(!rb_block_given_p()){
		rb_raise(rb_eArgError, "Block missing");
		return Qnil;
	}
	ESet::iterator* bg = eSetIt(b), * ed = eSetIt(e);
	if((ESet*)bg->_M_node->_TZ_tree != (ESet*)ed->_M_node->_TZ_tree){
		rb_raise(rb_eArgError, "First and second iterators are not from the same container");
		return Qnil;
	}
	VALUE res = Qnil;
	for(ESet::iterator it = *bg ; it != *ed ; ++it){
		if(&*it == NULL){
			rb_raise(rb_eRangeError, "Dereferencing the end iterator");
			return Qnil;
		}
		res = rb_yield(*it);
	}
	return res;
}


static VALUE eSet_erase(VALUE self, VALUE b, VALUE e) {
	ESet* p = eSet(self);
	ESet::iterator* bg = eSetIt(b), * ed = eSetIt(e);
	if((ESet*)bg->_M_node->_TZ_tree != p || (ESet*)ed->_M_node->_TZ_tree != p){
		rb_raise(rb_eArgError, "First and/or second iterators are not from the method reciever's container");
		return Qnil;
	}
	for(ESet::iterator it = *bg ; it != *ed ; ++it){
		if(&*it == NULL){
			rb_raise(rb_eRangeError, "Dereferencing the end iterator");
			return Qnil;
		}
		p->erase(it);
	}
	return Qnil;
}



void Init_ext_set(void){
	ID_CALL = rb_intern("call");
	ID_LESS = rb_intern("<");
	ID_EQ = rb_intern("==");

	cESet = rb_define_class("ESet", rb_cObject);
	cESetIt = rb_define_class_under(cESet, "Iterator", rb_cObject);

	rb_define_alloc_func(cESet, eSet_alloc);
	rb_define_alloc_func(cESetIt, eSetIt_alloc);
	rb_define_method(cESet, "initialize", RUBY_METHOD_FUNC(eSet_init), -1);
	rb_define_method(cESet, "begin", RUBY_METHOD_FUNC(eSet_begin), 0);
	rb_define_method(cESet, "end", RUBY_METHOD_FUNC(eSet_end), 0);
	rb_define_method(cESet, "size", RUBY_METHOD_FUNC(eSet_size), 0);
	rb_define_method(cESet, "insert", RUBY_METHOD_FUNC(eSet_insert), -1);

	rb_define_method(cESetIt, "inc", RUBY_METHOD_FUNC(eSetIt_inc), 0);
	rb_define_method(cESetIt, "dec", RUBY_METHOD_FUNC(eSetIt_dec), 0);
	rb_define_method(cESetIt, "item", RUBY_METHOD_FUNC(eSetIt_item), 0);

	rb_define_singleton_method(cESet, "find", RUBY_METHOD_FUNC(eSet_find), -1);
	rb_define_method(cESet, "erase", RUBY_METHOD_FUNC(eSet_erase), 2);
	rb_define_singleton_method(cESet, "for_each", RUBY_METHOD_FUNC(eSet_for_each), 2);
}



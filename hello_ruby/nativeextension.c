#include "ruby.h"
#include "extconf.h"

static VALUE rb_return_nil()
{
  return Qnil;
}

static VALUE rb_print_hello()
{
  const int result = printf("%s", "Hello From Native Extension\n");
  return INT2NUM(result);
}

void Init_nativeextension()
{
  VALUE module = rb_define_module("MyModule");

  rb_define_method(module, "return_nil", rb_return_nil, 0);
  rb_define_method(module, "print_hello", rb_print_hello, 0);
}

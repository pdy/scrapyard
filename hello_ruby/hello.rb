require "./nativeextension"
# require_relative 'nativeextension.so'

def hi
  puts "Hello World"
end



class Greeter
  def initialize(name = "World")
    @name = name
  end

  def sayHi
    puts "hi #{@name}"
  end
end

def hi_2
end

include MyModule
ret = print_hello
puts "ret #{ret}";




#include <string>
#include <iostream>

int main()
{
    std::string txt="";
    size_t c = 0;

    for (int num=0; num<2000; ++num) 
    {
        if (c != txt.capacity())
        {
            std::cout << txt.capacity() << " " << txt.size() << std::endl;
            c = txt.capacity();
        }
      txt.append(1, 'a');
    }
}

/*
*  MIT License
*  
*  Copyright (c) 2020 Pawel Drzycimski
*  
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*  
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.
*  
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
*
*/



#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>

#include <sstream>

using namespace std;

struct Attr { std::string name, value; };
//struct Tag { std::string name; std::vector<Attr> attrs;};

struct TagNode
{
    std::string name;
    std::vector<Attr> attrs;
    
    std::vector<TagNode*> children;
};

static std::vector<std::string> split(const std::string &line, char chr = ' ')
{
    std::vector<std::string> ret;
    
    size_t start = 0;
    size_t end = line.find_first_of(chr);
    while(end != std::string::npos)
    {
        ret.push_back(line.substr(start, end - start));
        
        start = end + 1;//ret.back().size();
        end = line.find_first_of(chr, start);
    }
    
    if(start < line.size())
        ret.push_back(line.substr(start));
        
    return ret;
}

static std::vector<Attr> parseAttrs(const std::string &line)
{
    // name="value"
    
    std::vector<Attr> ret;
    auto assign = line.find_first_of('=');
    while(assign != std::string::npos)
    {
        const auto nameLimit = assign - 1;
        auto startPos = nameLimit - 1;
        while(line[startPos] != ' ') --startPos;
        ++startPos;
        
        const auto valStart = assign + 3;
        auto valEnd = valStart;
        while(line[valEnd] != '"') ++valEnd;
        
        Attr attr {
            .name = line.substr(startPos, nameLimit - startPos),
            .value = line.substr(valStart, valEnd - valStart)
        };
        
        ret.push_back(std::move(attr));
        

        assign = line.find_first_of('=', valEnd + 1);
    }
    
    return ret;
    
}

static void printNode(TagNode &tag, size_t currIdx, size_t size)
{
    if(currIdx == size - 1)
        return;
        
    cout << "tag: " << tag.name << "\n";
    for(const auto &attr : tag.attrs)
        cout << "\t name [" << attr.name << "] value [" << attr.value << "]\n"; 
}

static TagNode* findRecursive(TagNode &node, std::string_view cmd)
{
    if(node.name == cmd)
        return &node;
    
    if(!node.children.empty())
    {
        for(auto it : node.children)
        {
            if(auto *found = findRecursive(*it, cmd))
                return found;
        }
    }
    
    
    return nullptr;
}

static TagNode* findFlat(TagNode &node, std::string_view cmd)
{
    if(node.name == cmd)
        return &node;
    
    if(!node.children.empty())
    {
        for(auto it : node.children)
        {
          if(it->name == cmd)
            return it;
        }
    }
    
    
    return nullptr;
}

using Query = std::vector<std::string>;


int main() {
    
    size_t n = 16;//, q = 10;


#if 0
    const std::string input = R"(<a value = "GoodVal">
<b value = "BadVal" size = "10">
</b>
<c height = "auto">
<d size = "3">
<e strength = "2">
</e>
</d>
</c>
</a>
a~value
b~value
a.b~size
a.b~value
a.b.c~height
a.c~height
a.d.e~strength
a.c.d.e~strength
d~sze
a.c.d~size)";

const std::string input = R"(<a>
<b name = "tag_one">
<c name = "tag_two" value = "val_907">
</c>
</b>
</a>
a.b~name
a.b.c~value
a.b.c~src
a.b.c.d~name)";
#endif

const std::string input = R"(<tag1 v1 = "123" v2 = "43.4" v3 = "hello">
</tag1>
<tag2 v4 = "v2" name = "Tag2">
<tag3 v1 = "Hello" v2 = "World!">
</tag3>
<tag4 v1 = "Hello" v2 = "Universe!">
</tag4>
</tag2>
<tag5>
<tag7 new_val = "New">
</tag7>
</tag5>
<tag6>
<tag8 intval = "34" floatval = "9.845">
</tag8>
</tag6>
tag1~v1
tag1~v2
tag1~v3
tag4~v2
tag2.tag4~v1
tag2.tag4~v2
tag2.tag3~v2
tag5.tag7~new_val
tag5~new_val
tag7~new_val
tag6.tag8~intval
tag6.tag8~floatval
tag6.tag8~val
tag8~intval)";

    //cin >> n >> q;
    
    std::vector<TagNode> tags; tags.reserve(n / 2);
    std::vector<Query> queries;
    
    //cin.ignore();
    
    /*
    for(size_t i = 0; i < n; ++i)
    {
        //std::string line;
        //std::cin >> line;
        //cout << line << "\n";
        
    }
    */
    
    std::istringstream iss(input);
//    iss << input;

    size_t counter = 0;
    TagNode *currNode = nullptr, *currNodeParent = nullptr;
    for(std::string line; std::getline(iss, line); ++counter)
    {
        if(counter < n)
        {
            //cout << line << "\n";
            if(line[0] == '<' && line[1] != '/')
            {
              std::cout << "line: [" << line << "]\n";
                const auto pos = line.find(' ');
                if(!currNode)
                {
                    TagNode tag; 
                    if(pos != std::string::npos)
                    {
                      tag.name = line.substr(1, pos - 1);
                    }
                    else
                    {
                      const auto closePos = line.find('>');
                      tag.name = line.substr(1, closePos - 1);

                    }
                    tags.push_back(tag);
                    currNode = &tags.back();
                }
                else{
                    TagNode *child = new TagNode;
                    child->name = line.substr(1, pos - 1);
                    
                    currNode->children.push_back(child);
                    
                    currNodeParent = currNode;
                    currNode = child;
                }
                

                currNode->attrs = parseAttrs(line);
                
            }
            
            else if(line[0] == '<' && line[1] == '/')
            {
                currNode = currNodeParent;
            }

            
            
        }
        else
        {
            auto tmp = split(line, '.');
 
            Query cmd;
            std::copy(tmp.begin(), std::prev(tmp.end()), std::back_inserter(cmd));
            
            tmp = split(tmp.back(), '~');
            std::copy(tmp.begin(), tmp.end(), std::back_inserter(cmd));
            
            queries.push_back(std::move(cmd));
            
        }
    }
  /*  
    for(const auto &tag : tags)
    {
        cout << "tag: " << tag.name << "\n";
        for(const auto &attr : tag.attrs)
            cout << "\t name [" << attr.name << "] value [" << attr.value << "]\n";
            
        for(auto *child : tag.children)
        {
            cout << "tag: " << child->name << "\n";
            for(const auto &attr : child->attrs)
                cout << "\t name [" << attr.name << "] value [" << attr.value << "]\n";
        }
}
*/
    
    for(const auto &cmds : queries)
    {
        TagNode *node{nullptr};
        for(size_t i = 0; i < cmds.size(); ++i)
        {
            const auto &cmd = cmds[i];
            if(i < cmds.size() - 1)
            {
                if(!node)
                {
                    for(auto &tag : tags)
                    {
                        if(tag.name == cmd)
                            node = &tag;
                    }
                }
                else
                {
                    node = findFlat(*node, cmd);
                }
            }
            else
            {
               if(!node)
               {  
                 cout << "Not Found!";
               }
               else
               {
                
                auto found = std::find_if(node->attrs.begin(), node->attrs.end(),
                    [&](const Attr &el) { return el.name == cmd;});
                    
                if(found != node->attrs.end())
                    cout << found->value;
                else
                    cout << "Not Found!";
                    
               }
                cout << "\n";
            }
        }
    }
    return 0;
}


#if 0

#include <cmdline.h> 
#include "simplelog/simplelog.h"

#include <iostream>

int main(int argc, char *argv[])
{
  cmdline::parser arg;
  arg.add("help", 'h', "Print help.");
//  arg.add<std::string>("file", 'f', "Example file argument.", true);
    
  if(!arg.parse(argc, const_cast<const char* const*>(argv)))
  {
    const auto fullErr = arg.error_full();
    if(!fullErr.empty())
      log << fullErr;
     
    log << arg.usage();
    return 0;
  }
  
  if(arg.exist("help"))
  {
    log << arg.usage();
    return 0;
  } 

  /* Example file arg check and get
  if(!arg.exist("file"))
  {
    log << "--file or -f argument is mandatory!\n";
    log << arg.usage();
    return 0;
  }
  
  const std::string file = arg.get<std::string>("file");
  */

  size_t n;
  std::cin >> n;

  std::cin.ignore();

  std::string nums;
  std::getline(std::cin, nums);

  std::cout << "n: " << n << " nums: " << nums << "\n";

  std::vector<std::string> arr; arr.reserve(n);
    {
        size_t first = 0;
        //nums.find()
        size_t end = nums.find(" ");
        while(end != std::string::npos)
        {
            arr.push_back(nums.substr(first, end - first));
            
            first = end + arr.back().size();
            end = nums.find(" ", first);
        }

        if(first < nums.size())
           arr.push_back(nums.substr(first)); 
    }


  for(auto it = arr.rbegin(); it != arr.rend(); ++it)
    std::cout << *it << " ";

  std::cout << "\n";

  return 0;
}

#endif

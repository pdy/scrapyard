/*
*  MIT License
*  
*  Copyright (c) 2025 Pawel Drzycimski
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

#include <cmdline.h> 
#include "simplelog/simplelog.h"
#include <stack>

template<typename T>
struct Node
{
  T data;

  Node *left{nullptr};
  Node *right{nullptr};
};

using NumNode = Node<int>;

static void preOrder_recursive(NumNode *node, std::string &answer)
{
  // pre-order == Root - Left - Right
  
  if(!node)
    return;

  answer.append(std::to_string(node->data));
  answer.push_back(' ');

  preOrder_recursive(node->left, answer);
  preOrder_recursive(node->right, answer);

}

static std::string preOrderIt(NumNode *root)
{
  // pre-order == Root - Left - Right

  std::string ret;

  std::stack<NumNode*> stack;

  auto *it = root;
  while(it)
  {
    ret.append(std::to_string(it->data));
    ret.push_back(' ');

    if(it->right)
      stack.push(it->right);

    it = it->left;
    if (!it && !stack.empty())
    {
      it = stack.top();
      stack.pop();
    }
  }

  return ret;
}

static void inOrder_recursive(NumNode *node, std::string &answer)
{
  // Left - Root - Right
  if(!node)
    return;

  inOrder_recursive(node->left, answer);

  answer.append(std::to_string(node->data));
  answer.push_back(' ');
  
  inOrder_recursive(node->right, answer);
}

static std::string inOrderIt(NumNode *root)
{
  // in-order == Left - Root - Right

  std::string ret;

  std::stack<NumNode*> stack;

  auto *it = root;
  while(it || !stack.empty())
  {
    while(it)
    {
      stack.push(it);
      it = it->left;
    }

    it = stack.top();
    stack.pop();

    ret.append(std::to_string(it->data));
    ret.push_back(' ');

    it = it->right;
  }

  return ret;
}

static void postOrder_recursive(NumNode *node, std::string &answer)
{
  // Left - Right - Root

  if(!node)
    return;

  postOrder_recursive(node->left, answer);
  postOrder_recursive(node->right, answer);

  answer.append(std::to_string(node->data));
  answer.push_back(' ');
}

static std::string postOrderIt(NumNode *node)
{
  // Left - Right - Root

  std::string answer;

  std::stack<NumNode*> stack_1, stack_2;
  NumNode *it = nullptr;
  stack_1.push(node);
  while(!stack_1.empty())
  {
    it = stack_1.top();
    stack_1.pop();

    stack_2.push(it);

    if(it->left)
      stack_1.push(it->left);
    if(it->right)
      stack_1.push(it->right);
  }

  while(!stack_2.empty())
  {
    it = stack_2.top();

    answer.append(std::to_string(it->data));
    answer.push_back(' ');

    stack_2.pop();
  }

  return answer;
}

static std::string preOrder(NumNode *node)
{
  // pre-order == Root - Left - Right
  
  std::string ret;
  preOrder_recursive(node, ret);
  return ret;
}

static std::string inOrder(NumNode *root)
{
  // Left - Root - Right
  
  std::string ret;
  inOrder_recursive(root, ret);
  return ret;
}

static std::string postOrder(NumNode *root)
{
  // Left - Right - Root
  
  std::string ret;
  postOrder_recursive(root, ret);
  return ret;
}

static size_t height(NumNode *node)
{
  if(!node)
    return 0;

  auto lh = height(node->left) + 1;
  auto rh = height(node->right) + 1;

  return std::max(lh, rh);
}

static void preOrderTest()
{

  // pre-order == Root - Left - Right

  /*
   *                  1
   *                 2  3
   *               4 5   6
   *                      7
   *
   */

  NumNode root{.data = 1};
  NumNode left_2 {.data = 2};
  NumNode left_4 {.data = 4};
  NumNode right_5 {.data = 5};

  root.left = &left_2;
  left_2.left = &left_4;
  left_2.right = &right_5;

  NumNode right_3{.data = 3};
  NumNode right_6{.data = 6};
  NumNode right_7{.data = 7};

  root.right = &right_3;
  right_3.right = &right_6;
  right_6.right = &right_7;

  const std::string_view EXPECTED = "1 2 4 5 3 6 7 ";

  LOG << "preOrder rec " << (EXPECTED == preOrder(&root) ? "Ok" : "Fail!");
  LOG << "preOrder  it " << (EXPECTED == preOrderIt(&root) ? "Ok" : "Fail!");
}

static void inOrderTest()
{

  // in-order == Left - Root - Right

  /*
   *                  1
   *                 2  3
   *               4 5   6
   *                      7
   *
   */

  NumNode root{.data = 1};
  NumNode left_2 {.data = 2};
  NumNode left_4 {.data = 4};
  NumNode right_5 {.data = 5};

  root.left = &left_2;
  left_2.left = &left_4;
  left_2.right = &right_5;

  NumNode right_3{.data = 3};
  NumNode right_6{.data = 6};
  NumNode right_7{.data = 7};

  root.right = &right_3;
  right_3.right = &right_6;
  right_6.right = &right_7;

  const std::string_view EXPECTED = "4 2 5 1 3 6 7 ";

  LOG << "inOrder rec " << (EXPECTED == inOrder(&root) ? "Ok" : "Fail!");
  LOG << "inOrder it  " << (EXPECTED == inOrderIt(&root) ? "Ok" : "Fail!");
}

static void postOrderTest()
{

  // post-order == Left - Right - Root

  /*
   *                  1
   *                 2  3
   *               4 5   6
   *                      7
   *
   */

  NumNode root{.data = 1};
  NumNode left_2 {.data = 2};
  NumNode left_4 {.data = 4};
  NumNode right_5 {.data = 5};

  root.left = &left_2;
  left_2.left = &left_4;
  left_2.right = &right_5;

  NumNode right_3{.data = 3};
  NumNode right_6{.data = 6};
  NumNode right_7{.data = 7};

  root.right = &right_3;
  right_3.right = &right_6;
  right_6.right = &right_7;

  const std::string_view EXPECTED ="4 5 2 7 6 3 1 ";

  LOG << "postOrder " << (EXPECTED == postOrder(&root) ? "Ok" : "Fail!");
  LOG << "postOrderIt " << (EXPECTED == postOrderIt(&root) ? "Ok" : "Fail!");
}

static void heightTest()
{

  /*
   *                  1
   *                 2  3
   *               4 5   6
   *                      7
   *
   */

  NumNode root{.data = 1};
  NumNode left_2 {.data = 2};
  NumNode left_4 {.data = 4};
  NumNode right_5 {.data = 5};

  root.left = &left_2;
  left_2.left = &left_4;
  left_2.right = &right_5;

  NumNode right_3{.data = 3};
  NumNode right_6{.data = 6};
  NumNode right_7{.data = 7};

  root.right = &right_3;
  right_3.right = &right_6;
  right_6.right = &right_7;

  const size_t EXPECTED = 4;

  LOG << "height " << (EXPECTED == height(&root) ? "Ok" : "Fail!");
}

static void heightTest_2()
{

  /*
   *                  1
   *                 2  3
   *               4 5    6
   *                 12 3  7
   *                11   2
   *                 13
   */

  NumNode root{.data = 1};
  NumNode left_2 {.data = 2};
  NumNode left_4 {.data = 4};
  NumNode right_5 {.data = 5};
  NumNode right_12 {.data = 12};
  NumNode left_11 {.data = 11};
  NumNode right_13 {.data = 13};

  { // left
    root.left = &left_2;
    left_2.left = &left_4;
    left_2.right = &right_5;

    right_5.right = &right_12;
    right_12.left = &left_11;
    left_11.right = &right_13;
  }

  NumNode right_3{.data = 3};
  NumNode right_6{.data = 6};
  NumNode right_7{.data = 7};
  NumNode left_3{.data = 3};
  NumNode right_2{.data = 2};

  { // right
    root.right = &right_3;
    right_3.right = &right_6;
    right_6.right = &right_7;
    right_6.left = &left_3;

    left_3.right = &right_2;

  }

  const size_t EXPECTED = 6;

  LOG << "height_2 " << (EXPECTED == height(&root) ? "Ok" : "Fail!");
}


static void bstInOrderTests()
{
  /*
   *                  7
   *                 5  8   
   *               4 6   12
   *                      13
   *
   */

  NumNode root{.data = 7};
  NumNode left_5 {.data = 5};
  NumNode left_6 {.data = 6};
  NumNode left_4 {.data = 4};

  root.left = &left_5;
  left_5.left = &left_4;
  left_5.right = &left_6;

  NumNode right_8{.data = 8};
  NumNode right_12{.data = 12};
  NumNode right_13{.data = 13};

  root.right = &right_8;
  right_8.right = &right_12;
  right_12.right = &right_13;

  const std::string EXPECTED = "4 5 6 7 8 12 13 ";

  LOG << "bstInOrderTests  it " << (EXPECTED == inOrderIt(&root) ? "Ok" : "Fail!");
  LOG << "bstInOrderTests rec " << (EXPECTED == inOrder(&root) ? "Ok" : "Fail!");
}

int main()
{

  preOrderTest(); 
  inOrderTest();
  bstInOrderTests();
  postOrderTest();
  heightTest();
  heightTest_2();
  return 0;
}

#if 0
int main(int argc, char *argv[])
{
  cmdline::parser arg;
  arg.add("help", 'h', "Print help.");
//  arg.add<std::string>("file", 'f', "Example file argument.", true);
    
  if(!arg.parse(argc, const_cast<const char* const*>(argv)))
  {
    const auto fullErr = arg.error_full();
    if(!fullErr.empty())
      LOG << fullErr;
     
    LOG << arg.usage();
    return 0;
  }
  
  if(arg.exist("help"))
  {
    LOG << arg.usage();
    return 0;
  } 

  /* Example file arg check and get
  if(!arg.exist("file"))
  {
    LOG << "--file or -f argument is mandatory!\n";
    LOG << arg.usage();
    return 0;
  }
  
  const std::string file = arg.get<std::string>("file");
  */
       
  return 0;
}
#endif

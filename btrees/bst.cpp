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
#include <limits>
#include <stack>

template<typename T>
struct Node
{
  T data;

  Node *left{nullptr};
  Node *right{nullptr};
};

using NumNode = Node<int>;



static bool isBST_inorder_rec_impl(NumNode *root, int &prev)
{
  if (!root)
    return true;

  if (!isBST_inorder_rec_impl(root->left, prev))
    return false;

  if (prev >= root->data)
    return false;
  
  prev = root->data;

  return isBST_inorder_rec_impl(root->right, prev);
}

static bool isBST_inorder_rec(NumNode *root)
{
  int prev = std::numeric_limits<int>::min();
  return isBST_inorder_rec_impl(root, prev);
}

static bool isBST_stack(NumNode *root)
{
  std::stack<NumNode*> st;
  auto *it = root;
  int prev = std::numeric_limits<int>::min();
  while(it || !st.empty())
  {
    while(it)
    {
      st.push(it);
      it = it->left;
    }

    it = st.top();
    st.pop();

    if(prev >= it->data)
      return false;

    prev = it->data;
    it = it->right;
  }

  return true;
}

static void isBstTest()
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

  constexpr bool EXPECTED = true;

  LOG << "isBST_stack       " << (EXPECTED == isBST_stack(&root) ? "Ok" : "Fail!");
  LOG << "isBST_inorder_rec " << (EXPECTED == isBST_inorder_rec(&root) ? "Ok" : "Fail!");
}

static void isBstTest_2()
{
  /*
   *                  7
   *                 5  8   
   *               4 9   12
   *                      13
   *
   */

  NumNode root{.data = 7};
  NumNode left_5 {.data = 5};
  NumNode left_9 {.data = 9};
  NumNode left_4 {.data = 4};

  root.left = &left_5;
  left_5.left = &left_4;
  left_5.right = &left_9;

  NumNode right_8{.data = 8};
  NumNode right_12{.data = 12};
  NumNode right_13{.data = 13};

  root.right = &right_8;
  right_8.right = &right_12;
  right_12.right = &right_13;

  constexpr bool EXPECTED = false;

  LOG << "isBST_stack       " << (EXPECTED == isBST_stack(&root) ? "Ok" : "Fail!");
  LOG << "isBST_inorder_rec " << (EXPECTED == isBST_inorder_rec(&root) ? "Ok" : "Fail!");
}

int main()
{
  isBstTest();
  isBstTest_2();
  return 0;
}

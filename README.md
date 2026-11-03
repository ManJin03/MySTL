# MySTL

这是一个std标准库里的STL容器的个人实现。

在本项目中，我不仅会深入每个容器的底层原理，实现和其相同的功能，同时也不会仅仅局限于STL容器，会加入更多其他的数据结构或功能容器。

本项目的所有容器都会在mtd命名空间内，详细的解释文章在我的个人技术博客[tech-blog](https://tech-manjin.pages.dev)
，参考教程有彭于斌老师的现代C++编程实战。

## 目前实现

| 文件名                                           | 内容                              |
|--------------------------------------------------|-----------------------------------|
| [Function](mtd/Function.hpp)                     | std::function的模拟实现           |
| [move_only_Function](mtd/move_only_Function.hpp) | std::move_only_function的模拟实现 |

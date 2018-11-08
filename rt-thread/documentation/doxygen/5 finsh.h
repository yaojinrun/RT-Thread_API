/*
 * This file is only used for doxygen document generation.
 */

/**
 * @defgroup finsh FinSH控制台
 *
 * @brief FinSH控制台接口
 * 
 * FinSH是RT-Thread的命令行外壳（shell），提供一套供用户在命令行的操作接口，主要用于调试或查看系统信息。FinSH支持两种模式：
 * 
 * 1. C语言解释器模式，为行文方便称之为c-style；
 * 2. 传统命令行模式，此模式又称为msh(module shell)。
 * 
 * C语言表达式解释模式下，FinSH能够解析执行大部分C语言的表达式，并使用类似C语言的函数调用方式访问系统中的函数及全局变量，此外它也能够通过命令行方式创建变量。
 * 
 * 在msh模式下，FinSH运行方式类似于dos/bash等传统shell。
 * 
 * 在本章的最后一节宏选项中介绍如何配置FinSH，读者可以根据自己的喜好配置FinSH。
 */
